/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Copyright (C) 2008 John Crispin <blogic@openwrt.org>
 * Based on EP93xx and ifxmips wdt driver
 */

#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/addrspace.h>
#include <ar531x.h>

#define CLOCK_RATE 40000000
#define HEARTBEAT(x) (x < 1 || x > 90)?(20):(x)

static int wdt_timeout = 20;
static int started = 0;
static int in_use = 0;
static int got_magic = 0;

static void
ar2315_wdt_enable(void)
{
	sysRegWrite(AR5315_WD, wdt_timeout * CLOCK_RATE);
	sysRegWrite(AR5315_ISR, 0x80);
}

static ssize_t
ar2315_wdt_write(struct file *file, const char __user *data, size_t len, loff_t *ppos)
{
	if(len)
	{
		ar2315_wdt_enable();
		got_magic = 0;

		size_t i;
		for(i = 0; i != len; i++)
		{
			char c;

			if(get_user(c, data + i))
				return -EFAULT;

			if(c == 'V')
				got_magic = 1;
		}
	}
	return len;
}

static int
ar2315_wdt_open(struct inode *inode, struct file *file)
{
	if(in_use)
		return -EBUSY;
	ar2315_wdt_enable();
	in_use = started = 1;
	got_magic = 0;
	printk(KERN_INFO "ar2315_wdt: enabling watchdog\n");
	return nonseekable_open(inode, file);
}

static int
ar2315_wdt_release(struct inode *inode, struct file *file)
{
	in_use = 0;
	if(got_magic)
	{
		printk(KERN_INFO "ar2315_wdt: disabling watchdog with magic\n");
		started = 0;
		got_magic = 0;
		sysRegWrite(AR5315_WDC, 0);
		sysRegWrite(AR5315_WD, 0);
		sysRegWrite(AR5315_ISR, 0x80);
	}
	return 0;
}

static irqreturn_t
ar2315_wdt_interrupt(int irq, void *dev_id)
{
	if(started)
	{
		printk(KERN_CRIT "ar2315_wdt: watchdog expired, rebooting system\n");
		emergency_restart();
	} else {
		sysRegWrite(AR5315_WDC, 0);
		sysRegWrite(AR5315_WD, 0);
		sysRegWrite(AR5315_ISR, 0x80);
	}
	return IRQ_HANDLED;
}

static struct watchdog_info ident = {
	.options = WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING,
	.identity = "ar2315 Watchdog",
};

static int
ar2315_wdt_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int new_wdt_timeout;
	int ret = -ENOIOCTLCMD;

	switch(cmd)
	{
		case WDIOC_GETSUPPORT:
			ret = copy_to_user((struct watchdog_info __user *)arg, &ident, sizeof(ident)) ? -EFAULT : 0;
			break;

		case WDIOC_KEEPALIVE:
			ar2315_wdt_enable();
			ret = 0;
			break;

		case WDIOC_SETTIMEOUT:
			if((ret = get_user(new_wdt_timeout, (int __user *)arg)))
				break;
			wdt_timeout = HEARTBEAT(new_wdt_timeout);
			ar2315_wdt_enable();
			break;

		case WDIOC_GETTIMEOUT:
			ret = put_user(wdt_timeout, (int __user *)arg);
			break;
	}
	return ret;
}

static struct file_operations ar2315_wdt_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.write		= ar2315_wdt_write,
	.ioctl		= ar2315_wdt_ioctl,
	.open		= ar2315_wdt_open,
	.release	= ar2315_wdt_release,
};

static struct miscdevice ar2315_wdt_miscdev = {
	.minor	= WATCHDOG_MINOR,
	.name	= "watchdog",
	.fops	= &ar2315_wdt_fops,
};

static int
ar2315_wdt_probe(struct platform_device *dev)
{
	int ret = 0;

	ar2315_wdt_enable();
	ret = request_irq(AR531X_MISC_IRQ_WATCHDOG, ar2315_wdt_interrupt, IRQF_DISABLED, "ar2315_wdt", NULL);
	if(ret)
	{
		printk(KERN_ERR "ar2315wdt: failed to register inetrrupt\n");
		goto out;
	}

	ret = misc_register(&ar2315_wdt_miscdev);
	if(ret)
		printk(KERN_ERR "ar2315wdt: failed to register miscdev\n");

out:
	return ret;
}

static int
ar2315_wdt_remove(struct platform_device *dev)
{
	misc_deregister(&ar2315_wdt_miscdev);
	free_irq(AR531X_MISC_IRQ_WATCHDOG, NULL);
	return 0;
}

static struct platform_driver ar2315_wdt_driver = {
	.probe = ar2315_wdt_probe,
	.remove = ar2315_wdt_remove,
	.driver = {
		.name = "ar2315_wdt",
		.owner = THIS_MODULE,
	},
};

static int __init
init_ar2315_wdt(void)
{
	int ret = platform_driver_register(&ar2315_wdt_driver);
	if(ret)
		printk(KERN_ERR "ar2315_wdt: error registering platfom driver!\n");
	return ret;
}

static void __exit
exit_ar2315_wdt(void)
{
	platform_driver_unregister(&ar2315_wdt_driver);
}

module_init(init_ar2315_wdt);
module_exit(exit_ar2315_wdt);
