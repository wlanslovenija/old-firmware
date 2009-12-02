/*
 * Copyright (c) 2008, Jernej Kos <kostko@unimatrix-one.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * * Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 * * Neither the name of the UniK olsr daemon nor the names of its contributors 
 *   may be used to endorse or promote products derived from this software 
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>

#include "olsrd_plugin.h"
#include "plugin_util.h"
#include "olsr.h"
#include "log.h"
#include "process_routes.h"
#include "kernel_routes.h"
#include "scheduler.h"

#define PLUGIN_INTERFACE_VERSION 5
#define ROUTE_FLAP_TIMER_MSEC 20000

/* !!! Redefine olsrd's macro as it is totally wrong !!! */
#define TIME_DUE(s1) ((int)((s1 - now_times) * olsr_cnf->system_tick_divider))

/* Pointers to original OLSR functions, so our hooks can call
 * them after they are done with their thing. */
static export_route_function orig_addroute_function;
static export_route_function orig_delroute_function;

/* NOTE: Code depends on there only being two operations that
 * are opposite each other (so if route flapping occurs within
 * the designated interval, actions will cancel out and no mass
 * executions will occur).
 *
 * So... don't change this ;)
 */
enum action_type {
  RT_ADD = 0,
  RT_DEL,

  /* This is ok as it is only used on init/finish */
  RT_INIT,
  RT_FINISH
};

struct action_queue {
  struct trigger_list *trigger;
  int type;
  struct action_queue *next;
};

struct trigger_list {
  struct in_addr trigger_addr;
  char *script;
  struct timer_entry *timer;
  clock_t next_update;
  struct trigger_list *next;
};

static struct {
  struct trigger_list *triggers;
  struct action_queue *aq;
} actions_conf;

/* Forward declarations */
void add_trigger(const char *addr, const char *script);

int olsrd_plugin_interface_version(void)
{
    return PLUGIN_INTERFACE_VERSION;
}

static int set_trigger(const char *value, void *data __attribute__((unused)), set_plugin_parameter_addon addon __attribute__((unused)))
{
    char addr[20] = {0,};
    char *script = strchr(value, '>');
    if (script == NULL || script - value > 16 || script - value < 7)
      return 1;
    
    strncpy(addr, value, script - value);
    /* Do not forget to free this script string later when freeing
     * triggers! */
    add_trigger(addr, strdup(script + 1));
    return 0;
}

static const struct olsrd_plugin_parameters plugin_parameters[] = {
  { .name = "trigger",   .set_plugin_parameter = &set_trigger,      .data = NULL },
};

void olsrd_get_plugin_parameters(const struct olsrd_plugin_parameters **params, int *size)
{
    *params = plugin_parameters;
    *size = sizeof(plugin_parameters)/sizeof(*plugin_parameters);
}

/**
 * Adds a new trigger to the trigger list.
 *
 * @param addr Network address
 * @param script Script to execute
 */
void add_trigger(const char *addr, const char *script)
{
  struct trigger_list *entry;
  
  entry = (struct trigger_list*) malloc(sizeof(struct trigger_list));
  entry->script = (char*) script;
  inet_aton(addr, &entry->trigger_addr);
  entry->timer = 0;
  entry->next_update = GET_TIMESTAMP(ROUTE_FLAP_TIMER_MSEC);
  entry->next = actions_conf.triggers;
  actions_conf.triggers = entry;
}

/**
 * Executes the given script.
 *
 * @param trigger Trigger data struct
 * @param type Action type
 */
void execute_script(const struct trigger_list *trigger, const int type)
{
  if (fork() == 0) {
    /* Scripts get passed the following arguments:
     *  $0 - Script path
     *  $1 - Operation ('add' or 'del') that ocurred
     *  $2 - Routing entry which has been added or deleted
     */
    if (type == RT_INIT) {
      execl(trigger->script, trigger->script, "init", (char*) NULL);
    } else if (type == RT_FINISH) {
      execl(trigger->script, trigger->script, "finish", (char*) NULL);
    } else {
      execl(trigger->script, trigger->script, type == RT_ADD ? "add" : "del", inet_ntoa(trigger->trigger_addr), (char*) NULL);
    }
  }
}

/**
 * Executes any pending scripts for the given trigger.
 *
 * @param context The trigger to process actions for
 */
void actions_execute_queued(void *context)
{
  struct action_queue *entry = actions_conf.aq;
  struct action_queue *tmp;
  struct action_queue *prev = 0;
  struct trigger_list *trigger = (struct trigger_list*) context;

  while (entry) {
    if (entry->trigger == trigger) {
      execute_script(entry->trigger, entry->type);
      
      /* Update next pointer, so we can remove ourselves */
      if (prev)
        prev->next = entry->next;
      else
        actions_conf.aq = entry->next;
      
      tmp = entry->next;
      free(entry);
      entry = tmp;
    } else {
      entry = entry->next;
    }
  }

  trigger->next_update = GET_TIMESTAMP(ROUTE_FLAP_TIMER_MSEC);
}


/**
 * Queues script execution for a later interval if the same script
 * is not already scheduled to be executed.
 *
 * @param trigger Trigger data struct
 * @param type Action type
 */
void queue_execute_script(struct trigger_list *trigger, const int type)
{
  /* See if this script has already been scheduled for execution */
  struct action_queue *entry = actions_conf.aq;
  struct action_queue *prev = 0;
  while (entry) {
    if (entry->trigger == trigger && type != entry->type) {
      /* Reverse operation already scheduled, so we should actually
       * do nothing at all - remove the op. */
      if (prev)
        prev->next = entry->next;
      else
        actions_conf.aq = entry->next;
      
      free(entry);
      return;
    } else if (entry->trigger == trigger && type == entry->type) {
      /* How can this be ? */
      return;
    }
    
    prev = entry;
    entry = entry->next;
  }
  
  /* Create new entry in the queue */
  entry = (struct action_queue*) malloc(sizeof(struct action_queue));
  entry->trigger = trigger;
  entry->type = type;
  entry->next = actions_conf.aq;
  actions_conf.aq = entry;

  /* Check if we can run queue processing */
  if (TIMED_OUT(trigger->next_update)) {
    /* Run now and set next update timestamp */
    actions_execute_queued((void*) trigger);

    /* If a timer has been set, stop it */
    olsr_stop_timer(trigger->timer);
    trigger->timer = 0;
  } else {
    /* We can't run now, schedule run for a later time */
    olsr_set_timer(&trigger->timer, TIME_DUE(trigger->next_update), 5, OLSR_TIMER_ONESHOT, &actions_execute_queued, (void*) trigger, 0);
  }
}

/**
 * A helper function that finds the trigger matching this
 * route entry and queues it for execution.
 *
 * @param type Action type
 * @param r Routing entry
 */
void find_and_exec_trigger(int type, const struct rt_entry *r)
{
  struct trigger_list *entry = actions_conf.triggers;

  while (entry) {
    /* If any actions are configured for this route, execute the script */
    if (r->rt_dst.prefix.v4.s_addr == entry->trigger_addr.s_addr) {
      queue_execute_script(entry, type);
      break;
    }

    entry = entry->next;
  }
}

/**
 * Overriden OLSR add route handler.
 */
int actions_add_olsr_v4_route(const struct rt_entry *r)
{
  find_and_exec_trigger(RT_ADD, r);
  return orig_addroute_function(r);
}

/**
 * Overriden OLSR del route handler.
 */
int actions_del_olsr_v4_route(const struct rt_entry *r)
{
  find_and_exec_trigger(RT_DEL, r);
  return orig_delroute_function(r);
}

/**
 * Just executes waitpid on the child process so it doesn't stay a
 * zombie.
 */
void actions_reap_zombies(int signal)
{
  int status;
  waitpid(-1, &status, WNOHANG);
}

int olsrd_plugin_init(void)
{
  /* Ensure that no zombies will be created by our forks */
  struct sigaction sa;
  sa.sa_handler = &actions_reap_zombies;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_NOCLDWAIT;
  sigaction(SIGCHLD, &sa, NULL);

  /* Register routing hooks */
  orig_addroute_function = olsr_addroute_function;
  orig_delroute_function = olsr_delroute_function;
  olsr_addroute_function = actions_add_olsr_v4_route;
  olsr_delroute_function = actions_del_olsr_v4_route;

  /* Trigger all registered actions with "init" parameter */
  struct trigger_list *entry = actions_conf.triggers;

  while (entry) {
    execute_script(entry, RT_INIT);
    entry = entry->next;
  }

  return 1;
}

static void my_init(void) __attribute__ ((constructor));
static void my_fini(void) __attribute__ ((destructor));

/**
 * Plugin constructor.
 */
static void my_init(void)
{
  actions_conf.aq = 0;
  actions_conf.triggers = 0;
}

/**
 * Plugin destructor.
 */
static void my_fini(void)
{
  struct action_queue *a_entry = actions_conf.aq;
  struct action_queue *a_tmp;
  struct trigger_list *t_entry = actions_conf.triggers;
  struct trigger_list *t_tmp;

  /* Free all triggers */
  while (t_entry) {
    execute_script(t_entry, RT_FINISH);
    t_tmp = t_entry->next;
    free(t_entry->script);
    free(t_entry);
    t_entry = t_tmp;
  }

  /* Free all actions that might have been queued */
  while (a_entry) {
    a_tmp = a_entry->next;
    free(a_entry);
    a_entry = a_tmp;
  }

  actions_conf.aq = 0;
  actions_conf.triggers = 0;
}

