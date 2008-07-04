/*
 * messyd - Mesh management deamon
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation; either version
 *   2 of the License, or (at your option) any later version.
 *
 * Copyright (C) 2008 Jernej Kos <kostko@unimatrix-one.org>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <net/if_arp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/uio.h>

#include "libnetlink.h"
#include "ll_map.h"
#include "utils.h"

static struct rtnl_handle nl_hdl;
static int wl_ifidx = -1;
static char wl_ifname[10];

int process_neighbours(const struct sockaddr_nl *s, struct nlmsghdr *n, void *p)
{
  struct ndmsg *r = NLMSG_DATA(n);
  struct rtattr *tb[NDA_MAX + 1];
  char abuf[256];
  
  if (n->nlmsg_type != RTM_NEWNEIGH)
    return 0;
  
  if (r->ndm_ifindex != wl_ifidx)
    return 0;
  
  parse_rtattr(tb, NDA_MAX, NDA_RTA(r), n->nlmsg_len - NLMSG_LENGTH(sizeof(*r)));

  if (tb[NDA_DST] && tb[NDA_LLADDR]) {
    SPRINT_BUF(b1);
    ll_addr_n2a(RTA_DATA(tb[NDA_LLADDR]), RTA_PAYLOAD(tb[NDA_LLADDR]), ll_index_to_type(r->ndm_ifindex), b1, sizeof(b1));

    // Skip broadcast addresses (they show up sometimes)
    if (strcmp(b1, "ff:ff:ff:ff:ff:ff") == 0)
      return 0;

    printf("INFO: Discovered neighbour - IP: %s MAC: %s iface: %s\n",
           format_host(r->ndm_family, RTA_PAYLOAD(tb[NDA_DST]), RTA_DATA(tb[NDA_DST]), abuf, sizeof(abuf)),
           b1,
           ll_index_to_name(r->ndm_ifindex));
  }

  return 0;
}

int request_neighbours_table()
{
  if (rtnl_wilddump_request(&nl_hdl, AF_INET, RTM_GETNEIGH) < 0)
    return -1;

  return 0;
}

int main(int argc, char **argv)
{
  if (argc < 4) {
    fprintf(stderr, "Usage: %s [iface] [controlIp] [controlPath]\n", argv[0]);
    fprintf(stderr, "ERROR: Missing arguments!\n");
    return 1;
  } else {
    strncpy(wl_ifname, argv[1], sizeof(wl_ifname));
  }

  // Initialize netlink and subscribe to neighbour notifications
  if (rtnl_open(&nl_hdl, RTMGRP_NEIGH) < 0) {
    fprintf(stderr, "ERROR: Unable to establish NETLINK connection!\n");
    return 1;
  }

  // Initialize link name map
  ll_init_map(&nl_hdl);

  if ((wl_ifidx = ll_name_to_index(wl_ifname)) == 0) {
    fprintf(stderr, "ERROR: Invalid interface name specified (%s)!\n", wl_ifname);
    return 1;
  }

  srand(time(0));

  for (;;) {
    // Check for new neighbours
    request_neighbours_table();
    rtnl_dump_filter(&nl_hdl, process_neighbours, NULL, NULL, NULL);

    // TODO Get current bandwidth usage data
    //process_bandwidth_usage();
    
    // TODO Notify the control server
    //push_data_to_control();
    
    // Sleep ~ 10 minutes
    sleep(600 + (rand() % 30));
  }

  return 0;
}

