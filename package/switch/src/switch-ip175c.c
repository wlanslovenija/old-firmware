/*
 * switch-ip175c.c: Switch configuration for IC+ IP175C switch
 * Version 0.3
 *
 * Copyright (C) 2008 Patrick Horn <patrick.horn@gmail.com>
 * Copyright (C) 2008 Martin Mares <mj@ucw.cz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/mii.h>
#include <linux/phy.h>
#include <linux/delay.h>

#include "switch-core.h"

#define MAX_VLANS 16
#define MAX_PORTS 9

#define DRIVER_NAME "ip175c"
#define DRIVER_VERSION "0.3"

typedef struct ip175c_reg {
	u16 p; 			// phy
	u16 m; 			// mii
} reg;
typedef char bitnum;

#define NOTSUPPORTED {-1,-1}

#define REG_SUPP(x) ((x).m != ((u16)-1))

/*********** CONSTANTS ***********/
struct register_mappings {
	char *NAME;
	u16 MODEL_NO;			// compare to bits 4-9 of MII register 0,3.
	bitnum NUM_PORTS;
	bitnum CPU_PORT;

/* The default VLAN for each port.
	 Default: 0x0001 for Ports 0,1,2,3
		  0x0002 for Ports 4,5 */
	reg VLAN_DEFAULT_TAG_REG[MAX_PORTS];

/* These ports are tagged.
	 Default: 0x00 */
	reg ADD_TAG_REG;
	reg REMOVE_TAG_REG;
	bitnum ADD_TAG_BIT[MAX_PORTS];
/* These ports are untagged.
	 Default: 0x00 (i.e. do not alter any VLAN tags...)
	 Maybe set to 0 if user disables VLANs. */
	bitnum REMOVE_TAG_BIT[MAX_PORTS];

/* Port M and Port N are on the same VLAN.
	 Default: All ports on all VLANs. */
// Use register {29, 19+N/2}
	reg VLAN_LOOKUP_REG;
// Port 5 uses register {30, 18} but same as odd bits.
	reg VLAN_LOOKUP_REG_5;		// in a different register on IP175C.
	bitnum VLAN_LOOKUP_EVEN_BIT[MAX_PORTS];
	bitnum VLAN_LOOKUP_ODD_BIT[MAX_PORTS];

/* This VLAN corresponds to which ports.
	 Default: 0x2f,0x30,0x3f,0x3f... */
	reg TAG_VLAN_MASK_REG;
	bitnum TAG_VLAN_MASK_EVEN_BIT[MAX_PORTS];
	bitnum TAG_VLAN_MASK_ODD_BIT[MAX_PORTS];

	int RESET_VAL;
	reg RESET_REG;

/* General flags */
	reg ROUTER_CONTROL_REG;
	reg VLAN_CONTROL_REG;
	bitnum TAG_VLAN_BIT;
	bitnum ROUTER_EN_BIT;
	bitnum NUMLAN_GROUPS_MAX;
	bitnum NUMLAN_GROUPS_BIT;

	reg MII_REGISTER_EN;
	bitnum MII_REGISTER_EN_BIT;

	// set to 1 for 178C, 0 for 175C.
	bitnum SIMPLE_VLAN_REGISTERS;	// 175C has two vlans per register but 178C has only one.
};

static const struct register_mappings IP178C = {
	.NAME = "IP178C",
	.MODEL_NO = 0x18,
	.VLAN_DEFAULT_TAG_REG = {
		{30,3},{30,4},{30,5},{30,6},{30,7},{30,8},
		{30,9},{30,10},{30,11},
	},

	.ADD_TAG_REG = {30,12},
	.ADD_TAG_BIT = {0,1,2,3,4,5,6,7,8},
	.REMOVE_TAG_REG = {30,13},
	.REMOVE_TAG_BIT = {4,5,6,7,8,9,10,11,12},

	.SIMPLE_VLAN_REGISTERS = 1,

	.VLAN_LOOKUP_REG = {31,0},// +N
	.VLAN_LOOKUP_REG_5 = NOTSUPPORTED, // not used with SIMPLE_VLAN_REGISTERS
	.VLAN_LOOKUP_EVEN_BIT = {0,1,2,3,4,5,6,7,8},
	.VLAN_LOOKUP_ODD_BIT = {0,1,2,3,4,5,6,7,8},

	.TAG_VLAN_MASK_REG = {30,14}, // +N
	.TAG_VLAN_MASK_EVEN_BIT = {0,1,2,3,4,5,6,7,8},
	.TAG_VLAN_MASK_ODD_BIT = {0,1,2,3,4,5,6,7,8},

	.RESET_VAL = 0x55AA,
	.RESET_REG = {30,0},

	.ROUTER_CONTROL_REG = {30,30},
	.ROUTER_EN_BIT = 11,
	.NUMLAN_GROUPS_MAX = 8,
	.NUMLAN_GROUPS_BIT = 8, // {0-2}

	.VLAN_CONTROL_REG = {30,13},
	.TAG_VLAN_BIT = 3,

	.CPU_PORT = 8,
	.NUM_PORTS = 9,

	.MII_REGISTER_EN = NOTSUPPORTED,

};

static const struct register_mappings IP175C = {
	.NAME = "IP175C",
	.MODEL_NO = 0x18,
	.VLAN_DEFAULT_TAG_REG = {
		{29,24},{29,25},{29,26},{29,27},{29,28},{29,30},
		NOTSUPPORTED,NOTSUPPORTED,NOTSUPPORTED
	},

	.ADD_TAG_REG = {29,23},
	.REMOVE_TAG_REG = {29,23},
	.ADD_TAG_BIT = {11,12,13,14,15,1,-1,-1,-1},
	.REMOVE_TAG_BIT = {6,7,8,9,10,0,-1,-1,-1},

	.SIMPLE_VLAN_REGISTERS = 0,

	.VLAN_LOOKUP_REG = {29,19},// +N/2
	.VLAN_LOOKUP_REG_5 = {30,18},
	.VLAN_LOOKUP_EVEN_BIT = {8,9,10,11,12,15,-1,-1,-1},
	.VLAN_LOOKUP_ODD_BIT = {0,1,2,3,4,7,-1,-1,-1},

	.TAG_VLAN_MASK_REG = {30,1}, // +N/2
	.TAG_VLAN_MASK_EVEN_BIT = {0,1,2,3,4,5,-1,-1,-1},
	.TAG_VLAN_MASK_ODD_BIT = {8,9,10,11,12,13,-1,-1,-1},

	.RESET_VAL = 0x175C,
	.RESET_REG = {30,0},

	.ROUTER_CONTROL_REG = {30,9},
	.ROUTER_EN_BIT = 3,
	.NUMLAN_GROUPS_MAX = 8,
	.NUMLAN_GROUPS_BIT = 0, // {0-2}

	.VLAN_CONTROL_REG = {30,9},
	.TAG_VLAN_BIT = 7,

	.NUM_PORTS = 6,
	.CPU_PORT = 5,

	.MII_REGISTER_EN = NOTSUPPORTED,

};

static const struct register_mappings IP175A = {
	.NAME = "IP175A",
	.MODEL_NO = 0x05,
	.VLAN_DEFAULT_TAG_REG = {
		{0,24},{0,25},{0,26},{0,27},{0,28},NOTSUPPORTED,
		NOTSUPPORTED,NOTSUPPORTED,NOTSUPPORTED
	},

	.ADD_TAG_REG = {0,23},
	.REMOVE_TAG_REG = {0,23},
	.ADD_TAG_BIT = {11,12,13,14,15,1,-1,-1,-1},
	.REMOVE_TAG_BIT = {6,7,8,9,10,0,-1,-1,-1},

	.SIMPLE_VLAN_REGISTERS = 1,

	// Only programmable via. EEPROM
	.VLAN_LOOKUP_REG = NOTSUPPORTED,// +N/2
	.VLAN_LOOKUP_REG_5 = NOTSUPPORTED,
	.VLAN_LOOKUP_EVEN_BIT = {8,9,10,11,12,15,-1,-1,-1},
	.VLAN_LOOKUP_ODD_BIT = {0,1,2,3,4,7,-1,-1,-1},

	.TAG_VLAN_MASK_REG = NOTSUPPORTED, // +N/2
	.TAG_VLAN_MASK_EVEN_BIT = {0,1,2,3,4,5,-1,-1,-1},
	.TAG_VLAN_MASK_ODD_BIT = {8,9,10,11,12,13,-1,-1,-1},

	.RESET_VAL = -1,
	.RESET_REG = NOTSUPPORTED,

	.ROUTER_CONTROL_REG = NOTSUPPORTED,
	.VLAN_CONTROL_REG = NOTSUPPORTED,
	.TAG_VLAN_BIT = -1,
	.ROUTER_EN_BIT = -1,
	.NUMLAN_GROUPS_MAX = -1,
	.NUMLAN_GROUPS_BIT = -1, // {0-2}

	.NUM_PORTS = 6,
	.CPU_PORT = 5,

	.MII_REGISTER_EN = {0, 12},
	.MII_REGISTER_EN_BIT = 7,
};

struct ip175c_state {
	struct list_head list;
	struct mii_bus *mii_bus;
	switch_driver *dev;

	int router_mode;		// ROUTER_EN
	int vlan_enabled;		// TAG_VLAN_EN
	struct port_state {
		struct phy_device *phy;
		unsigned int shareports;
		u16 vlan_tag;
	} ports[MAX_PORTS];
	unsigned int add_tag;
	unsigned int remove_tag;
	int num_vlans;
	unsigned int vlan_ports[MAX_VLANS];
	const struct register_mappings *regs;
	reg proc_mii; /*!< phy/reg for the low level register access via /proc */
	int proc_errno; /*!< error code of the last read/write to "val" */
};

static struct list_head state_list;

static int getPhy (struct ip175c_state *state, reg mii)
{
	struct mii_bus *bus = state->mii_bus;
	int err;

	if (!REG_SUPP(mii))
		return -EFAULT;
	mutex_lock(&bus->mdio_lock);
	err = bus->read(bus, mii.p, mii.m);
	mutex_unlock(&bus->mdio_lock);
	if (err < 0) {
		pr_warning("IP175C: Unable to get MII register %d,%d: error %d\n", mii.p,mii.m,-err);
		return err;
	}

	pr_debug("IP175C: Read MII register %d,%d -> %04x\n", mii.p, mii.m, err);
	return err;
}

static int setPhy (struct ip175c_state *state, reg mii, u16 value)
{
	struct mii_bus *bus = state->mii_bus;
	int err;

	if (!REG_SUPP(mii))
		return -EFAULT;
	mutex_lock(&bus->mdio_lock);
	err = bus->write(bus, mii.p, mii.m, value);
	mutex_unlock(&bus->mdio_lock);
	if (err < 0) {
		pr_warning("IP175C: Unable to set MII register %d,%d to %d: error %d\n", mii.p,mii.m,value,-err);
		return err;
	}
	mdelay(2);
	getPhy(state, mii);
	pr_debug("IP175C: Set MII register %d,%d to %04x\n", mii.p, mii.m, value);
	return 0;
}

#define GET_PORT_BITS(state, bits, addr, bit_lookup)		\
	do {							\
		int i, val = getPhy((state), (addr));		\
		if (val < 0)					\
			return val;				\
		(bits) = 0;					\
		for (i = 0; i < MAX_PORTS; i++) {		\
			if ((bit_lookup)[i] == -1) continue;	\
			if (val & (1<<(bit_lookup)[i]))		\
				(bits) |= (1<<i);		\
		}						\
	} while (0)

#define SET_PORT_BITS(state, bits, addr, bit_lookup)		\
	do {							\
		int i, val = getPhy((state), (addr));		\
		if (val < 0)					\
			return val;				\
		for (i = 0; i < MAX_PORTS; i++) {		\
			unsigned int newmask = ((bits)&(1<<i));	\
			if ((bit_lookup)[i] == -1) continue;	\
			val &= ~(1<<(bit_lookup)[i]);		\
			val |= ((newmask>>i)<<(bit_lookup)[i]);	\
		}						\
		val = setPhy((state), (addr), val);		\
		if (val < 0)					\
			return val;				\
	} while (0)

static int get_model(struct ip175c_state *state)
{
	reg oui_id_reg = {0, 2};
	int oui_id;
	reg model_no_reg = {0, 3};
	int model_no, model_no_orig;

	// 175 and 178 have the same oui ID.
	reg oui_id_reg_178c = {5, 2}; // returns error on IP175C.
	int is_178c = 0;

	oui_id = getPhy(state, oui_id_reg);
	if (oui_id != 0x0243) {
		// non
		return -ENODEV; // Not a IC+ chip.
	}
	oui_id = getPhy(state, oui_id_reg_178c);
	if (oui_id == 0x0243) {
		is_178c = 1;
	}

	model_no_orig = getPhy(state, model_no_reg);
	if (model_no_orig < 0) {
		return -ENODEV;
	}
	model_no = model_no_orig >> 4; // shift out revision number.
	model_no &= 0x3f; // only take the model number (low 6 bits).
	if (model_no == IP175A.MODEL_NO) {
		state->regs = &IP175A;
	} else if (model_no == IP175C.MODEL_NO) {
		if (is_178c) {
			state->regs = &IP178C;
		} else {
			state->regs = &IP175C;
		}
	} else {
		printk(KERN_WARNING "ip175c: Found an unknown IC+ switch with model number %02Xh.\n", model_no_orig);
		return -EPERM;
	}
	return 0;
}

/** Get only the vlan and router flags on the router **/
static int get_flags(struct ip175c_state *state)
{
	int val;

	state->router_mode = 0;
	state->vlan_enabled = -1; // hack
	state->num_vlans = 0;

	if (!REG_SUPP(state->regs->ROUTER_CONTROL_REG)) {
		return 0; // not an error if it doesn't support enable vlan.
	}

	val = getPhy(state, state->regs->ROUTER_CONTROL_REG);
	if (val < 0) {
		return val;
	}
	if (state->regs->ROUTER_EN_BIT >= 0)
		state->router_mode = ((val>>state->regs->ROUTER_EN_BIT) & 1);
	
	if (state->regs->NUMLAN_GROUPS_BIT >= 0) {
		state->num_vlans = (val >> state->regs->NUMLAN_GROUPS_BIT);
		state->num_vlans &= (state->regs->NUMLAN_GROUPS_MAX-1);
		state->num_vlans+=1; // does not include WAN.
	}
	

	val = getPhy(state, state->regs->VLAN_CONTROL_REG);
	if (val < 0) {
		return 0;
	}
	if (state->regs->TAG_VLAN_BIT >= 0)
		state->vlan_enabled = ((val>>state->regs->TAG_VLAN_BIT) & 1);

	return  0;
}
/** Get all state variables for VLAN mappings and port-based tagging. **/
static int get_state(struct ip175c_state *state)
{
	int i, j;
	int ret;
	ret = get_flags(state);
	if (ret < 0) {
		return ret;
	}
	GET_PORT_BITS(state, state->remove_tag,
				  state->regs->REMOVE_TAG_REG, state->regs->REMOVE_TAG_BIT);
	GET_PORT_BITS(state, state->add_tag,
				  state->regs->ADD_TAG_REG, state->regs->ADD_TAG_BIT);

	if (state->vlan_enabled == -1) {
		// not sure how to get this...
		state->vlan_enabled = (!state->remove_tag && !state->add_tag);
	}

	if (REG_SUPP(state->regs->VLAN_LOOKUP_REG)) {
	for (j=0; j<MAX_PORTS; j++) {
		state->ports[j].shareports = 0; // initialize them in case.
	}
	for (j=0; j<state->regs->NUM_PORTS; j++) {
		reg addr;
		const bitnum *bit_lookup = (j%2==0)?
			state->regs->VLAN_LOOKUP_EVEN_BIT:
			state->regs->VLAN_LOOKUP_ODD_BIT;
		addr = state->regs->VLAN_LOOKUP_REG;
		if (state->regs->SIMPLE_VLAN_REGISTERS) {
			addr.m += j;
		} else {
			switch (j) {
			case 0:
			case 1:
				break;
			case 2:
			case 3:
				addr.m+=1;
				break;
			case 4:
				addr.m+=2;
				break;
			case 5:
				addr = state->regs->VLAN_LOOKUP_REG_5;
				break;
			}
		}

		if (REG_SUPP(addr)) {
			GET_PORT_BITS(state, state->ports[j].shareports, addr, bit_lookup);
		}
	}
	} else {
		for (j=0; j<MAX_PORTS; j++) {
			state->ports[j].shareports = 0xff;
		}
	}

	for (i=0; i<MAX_PORTS; i++) {
		if (REG_SUPP(state->regs->VLAN_DEFAULT_TAG_REG[i])) {
			int val = getPhy(state, state->regs->VLAN_DEFAULT_TAG_REG[i]);
			if (val < 0) {
				return val;
			}
			state->ports[i].vlan_tag = val;
		} else {
			state->ports[i].vlan_tag = 0;
		}
	}

	if (REG_SUPP(state->regs->TAG_VLAN_MASK_REG)) {
		for (j=0; j<MAX_VLANS; j++) {
			reg addr = state->regs->TAG_VLAN_MASK_REG;
			const bitnum *bit_lookup = (j%2==0)?
				state->regs->TAG_VLAN_MASK_EVEN_BIT:
				state->regs->TAG_VLAN_MASK_ODD_BIT;
			if (state->regs->SIMPLE_VLAN_REGISTERS) {
				addr.m += j;
			} else {
				addr.m += j/2;
			}
			GET_PORT_BITS(state, state->vlan_ports[j], addr, bit_lookup);
		}
	} else {
		for (j=0; j<MAX_VLANS; j++) {
			state->vlan_ports[j] = 0;
			for (i=0; i<state->regs->NUM_PORTS; i++) {
				if ((state->ports[i].vlan_tag == j) ||
						(state->ports[i].vlan_tag == 0)) {
					state->vlan_ports[j] |= (1<<i);
				}
			}
		}
	}

	return 0;
}


/** Only update vlan and router flags in the switch **/
static int update_flags(struct ip175c_state *state)
{
	int val;

	if (!REG_SUPP(state->regs->ROUTER_CONTROL_REG)) {
		return 0;
	}

	val = getPhy(state, state->regs->ROUTER_CONTROL_REG);
	if (val < 0) {
		return val;
	}
	if (state->regs->ROUTER_EN_BIT >= 0) {
		if (state->router_mode) {
			val |= (1<<state->regs->ROUTER_EN_BIT);
		} else {
			val &= (~(1<<state->regs->ROUTER_EN_BIT));
		}
	}
	if (state->regs->TAG_VLAN_BIT >= 0) {
		if (state->vlan_enabled) {
			val |= (1<<state->regs->TAG_VLAN_BIT);
		} else {
			val &= (~(1<<state->regs->TAG_VLAN_BIT));
		}
	}
	if (state->regs->NUMLAN_GROUPS_BIT >= 0) {
		val &= (~((state->regs->NUMLAN_GROUPS_MAX-1)<<state->regs->NUMLAN_GROUPS_BIT));
		if (state->num_vlans > state->regs->NUMLAN_GROUPS_MAX) {
			val |= state->regs->NUMLAN_GROUPS_MAX << state->regs->NUMLAN_GROUPS_BIT;
		} else if (state->num_vlans >= 1) {
			val |= (state->num_vlans-1) << state->regs->NUMLAN_GROUPS_BIT;
		}
	}
	return setPhy(state, state->regs->ROUTER_CONTROL_REG, val);
}

/** Update all VLAN and port state.  Usually you should call "correct_vlan_state" first. **/
static int update_state(struct ip175c_state *state)
{
	int j;
	int i;
	SET_PORT_BITS(state, state->add_tag,
				  state->regs->ADD_TAG_REG, state->regs->ADD_TAG_BIT);
	SET_PORT_BITS(state, state->remove_tag,
				  state->regs->REMOVE_TAG_REG, state->regs->REMOVE_TAG_BIT);

	if (REG_SUPP(state->regs->VLAN_LOOKUP_REG)) {
	for (j=0; j<state->regs->NUM_PORTS; j++) {
		reg addr;
		const bitnum *bit_lookup = (j%2==0)?
			state->regs->VLAN_LOOKUP_EVEN_BIT:
			state->regs->VLAN_LOOKUP_ODD_BIT;

		// duplicate code -- sorry
		addr = state->regs->VLAN_LOOKUP_REG;
		if (state->regs->SIMPLE_VLAN_REGISTERS) {
			addr.m += j;
		} else {
			switch (j) {
			case 0:
			case 1:
				break;
			case 2:
			case 3:
				addr.m+=1;
				break;
			case 4:
				addr.m+=2;
				break;
			case 5:
				addr = state->regs->VLAN_LOOKUP_REG_5;
				break;
			default:
				addr.m = -1; // shouldn't get here, but...
				break;
			}
		}
		//printf("shareports for %d is %02X\n",j,state->ports[j].shareports);
		if (REG_SUPP(addr)) {
			SET_PORT_BITS(state, state->ports[j].shareports, addr, bit_lookup);
		}
	}
	}
	if (REG_SUPP(state->regs->TAG_VLAN_MASK_REG)) {
	for (j=0; j<MAX_VLANS; j++) {
		reg addr = state->regs->TAG_VLAN_MASK_REG;
		const bitnum *bit_lookup = (j%2==0)?
			state->regs->TAG_VLAN_MASK_EVEN_BIT:
			state->regs->TAG_VLAN_MASK_ODD_BIT;
		unsigned int vlan_mask;
		if (state->regs->SIMPLE_VLAN_REGISTERS) {
			addr.m += j;
		} else {
			addr.m += j/2;
		}
		vlan_mask = state->vlan_ports[j];
		SET_PORT_BITS(state, vlan_mask, addr, bit_lookup);
	}
	}

	for (i=0; i<MAX_PORTS; i++) {
		if (REG_SUPP(state->regs->VLAN_DEFAULT_TAG_REG[i])) {
			int err = setPhy(state, state->regs->VLAN_DEFAULT_TAG_REG[i],
				 	state->ports[i].vlan_tag);
			if (err < 0) {
				return err;
			}
		}
	}

	return update_flags(state);

	// software reset: 30.0 = 0x175C
	// wait 2ms
	// reset ports 0,1,2,3,4
}

/*
  Uses only the VLAN port mask and the add tag mask to generate the other fields:
  which ports are part of the same VLAN, removing vlan tags, and VLAN tag ids.
 */
static void correct_vlan_state(struct ip175c_state *state)
{
	int i, j;
	state->num_vlans = 0;
	for (i=0; i<MAX_VLANS; i++) {
		if (state->vlan_ports[i] != 0) {
			state->num_vlans = i+1; //hack -- we need to store the "set" vlans somewhere...
		}
	}

	for (i=0; i<state->regs->NUM_PORTS; i++) {
		int oldtag = state->ports[i].vlan_tag;
		if (oldtag >= 0 && oldtag < MAX_VLANS) {
			if (state->vlan_ports[oldtag] & (1<<i)) {
				continue; // primary vlan is valid.
			}
		}
		state->ports[i].vlan_tag = 0;
	}

	for (i=0; i<state->regs->NUM_PORTS; i++) {
		unsigned int portmask = (1<<i);
		state->ports[i].shareports = portmask;
		for (j=0; j<MAX_VLANS; j++) {
			if (state->vlan_ports[j] & portmask) {
				state->ports[i].shareports |= state->vlan_ports[j];
				if (state->ports[i].vlan_tag == 0) {
					state->ports[i].vlan_tag = j;
				}
			}
		}
		if (!state->vlan_enabled) {
			// share with everybody!
			state->ports[i].shareports = (1<<state->regs->NUM_PORTS)-1;
		}
	}
	state->remove_tag = ((~state->add_tag) & ((1<<state->regs->NUM_PORTS)-1));
}

enum {
	IP175C_ENABLE_VLAN,
	IP175C_ENABLE_ROUTER,
	IP175C_VLAN_PORTS
};

static int ip175c_get_router_mode(void *driver, char *buf, int nr)
{
	switch_driver *dev = driver;
	struct ip175c_state *state = dev->priv;
	int err;

	err = get_flags(state);
	if (err < 0)
		return err;
	return sprintf(buf, "%d\n", state->router_mode);
}

static int ip175c_set_router_mode(void *driver, char *buf, int nr)
{
	switch_driver *dev = driver;
	struct ip175c_state *state = dev->priv;
	int err;
	int enable;

	err = get_flags(state);
	if (err < 0)
		return err;
	enable = ((buf[0] != '1') ? 0 : 1);
	state->router_mode = enable;
	return update_flags(state);
}

static int ip175c_get_enable_vlan(void *driver, char *buf, int nr)
{
	switch_driver *dev = driver;
	struct ip175c_state *state = dev->priv;
	int err;

	err = get_state(state); // may be set in get_state.
	if (err < 0)
		return err;
	return sprintf(buf, "%d\n", state->vlan_enabled);
}

static int ip175c_set_enable_vlan(void *driver, char *buf, int nr)
{
	switch_driver *dev = driver;
	struct ip175c_state *state = dev->priv;
	int err;
	int enable;
	int i;

	err = get_state(state);
	if (err < 0)
		return err;
	enable = ((buf[0] != '1') ? 0 : 1);
	//printk(KERN_WARNING "set enable vlan to %d\n",enable);

	if (state->vlan_enabled == enable) {
		// do not change any state.
		return 0;
	}
	state->vlan_enabled = enable;

	// Otherwise, if we are switching state, set fields to a known default.
	state->remove_tag = 0x0000;
	state->add_tag = 0x0000;
	for (i = 0; i < MAX_PORTS; i++) {
		state->ports[i].vlan_tag = 0;
		state->ports[i].shareports = 0xffff;
	}
	for (i = 0; i < MAX_VLANS; i++) {
		state->vlan_ports[i] = 0x0;
	}

	if (state->vlan_enabled) {
		// updates other fields only based off vlan_ports and add_tag fields.
		// Note that by default, no ports are in any vlans.
		correct_vlan_state(state);
	}
	// ensure sane defaults?
	return update_state(state);
}

static int ip175c_get_ports(void *driver, char *buf, int nr)
{
	switch_driver *dev = driver;
	struct ip175c_state *state = dev->priv;
	int err;
	int b;
	int buflen;
	unsigned int ports;

	if (nr >= dev->vlans || nr < 0)
		return -EINVAL;

	err = get_state(state);
	if (err<0)
		return err;

	ports = state->vlan_ports[nr];
	b = 0;
	buflen = 0;
	while (b < state->regs->NUM_PORTS) {
		if (ports&1) {
			int istagged = ((state->add_tag >> b) & 1);
			buflen += sprintf(buf+buflen, "%d", b);
			if (!istagged) {
				if (b == state->regs->CPU_PORT)
					buf[buflen++] = 'u';
			} else {
				buf[buflen++] = 't';
				if (state->ports[b].vlan_tag == nr) {
					buf[buflen++] = '*';
				}
			}
			buf[buflen++] = '\t';
		}
		b++;
		ports >>= 1;
	}
	buf[buflen++] = '\n';
	buf[buflen] = '\0';

	return buflen;
}

static int ip175c_set_ports(void *driver, char *buf, int nr)
{
	switch_driver *dev = driver;
	struct ip175c_state *state = dev->priv;
	int i;
	int err;
  	switch_vlan_config *vlan_config;

	if (nr >= dev->vlans || nr < 0)
		return -EINVAL;
	err = get_state(state);
	if (err < 0)
		return err;

	vlan_config = switch_parse_vlan(dev, buf);
	
	state->vlan_ports[nr] = vlan_config->port;
	for (i = 0; i< state->regs->NUM_PORTS; i++) {
		int bitmask = (1<<i);
		if (state->vlan_ports[nr] & bitmask) {
			// don't update tagged flag for unspecified ports.
			if (vlan_config->untag & bitmask) {
				state->add_tag &= (~bitmask);
			} else {
				state->add_tag |= bitmask;
			}
		}
	}
	for (i = 0; i< state->regs->NUM_PORTS; i++) {
		if (vlan_config->pvid & (1<<i)) {
			state->ports[i].vlan_tag = nr;
		}
	}

	// pvid?  Right now correct_vlan_state uses the first vlan.

	//printk(KERN_WARNING "set vlan %d ports to %04X\n",nr, state->vlan_ports[nr]);

	correct_vlan_state(state);
	//printk(KERN_WARNING "really set vlan %d ports to %04X\n",nr, state->vlan_ports[nr]);

	err = update_state(state);

	kfree(vlan_config); // Why does switch-robo leak this?

	return err;
}

static int ip175c_apply(void *driver, char *buf, int nr)
{
	switch_driver *dev = driver;
	struct ip175c_state *state = dev->priv;
	int disable = ((buf[0] != '1') ? 1 : 0);
	int err;

	err = get_flags(state);
	if (err < 0)
		return err;

	if (REG_SUPP(state->regs->MII_REGISTER_EN)){
		int val = getPhy(state, state->regs->MII_REGISTER_EN);
		if (val < 0) {
			return val;
		}
		if (disable) {
			val &= ~(1<<state->regs->MII_REGISTER_EN_BIT);
		} else {
			val |= (1<<state->regs->MII_REGISTER_EN_BIT);
		}
		return setPhy(state, state->regs->MII_REGISTER_EN, val);
	}
	return 0;
}

static int ip175c_reset(void *driver, char *buf, int nr)
{
	switch_driver *dev = driver;
	struct ip175c_state *state = dev->priv;
	int err;

	err = get_flags(state);
	if (err < 0)
		return err;

	if (REG_SUPP(state->regs->RESET_REG)) {
		err = setPhy(state, state->regs->RESET_REG, state->regs->RESET_VAL);
		if (err < 0)
			return err;
		err = getPhy(state, state->regs->RESET_REG);

		/* data sheet specifies reset period is 2 msec
		   (don't see any mention of the 2ms delay in the IP178C spec, only
		    in IP175C, but it can't hurt.) */
		mdelay(2);
	}
	return 0;
}


/* low level /proc procedures */

/*! get the current phy address */
static int ip175c_get_phy(void *driver, char *buf, int nr)
{
	switch_driver *dev = driver;
	struct ip175c_state *state = dev->priv;

	return sprintf(buf, "%u\n", state->proc_mii.p);
}

/*! set a new phy address for low level access to registers */
static int ip175c_set_phy(void *driver, char *buf, int nr)
{
	switch_driver *dev = driver;
	struct ip175c_state *state = dev->priv;
	char *end;
	unsigned long new_phy;

	new_phy = simple_strtoul(buf, &end, 0);
	if (buf != end) {
		state->proc_mii.p = (u16)new_phy;
		return 0;
	} else
		return -EINVAL;
}

/*! get the current register number */
static int ip175c_get_reg(void *driver, char *buf, int nr)
{
	switch_driver *dev = driver;
	struct ip175c_state *state = dev->priv;

	return sprintf(buf, "%u\n", state->proc_mii.m);
}

/*! set a new register address for low level access to registers */
static int ip175c_set_reg(void *driver, char *buf, int nr)
{
	switch_driver *dev = driver;
	struct ip175c_state *state = dev->priv;
	char *end;
	unsigned long new_reg;

	new_reg = simple_strtoul(buf, &end, 0);
	if (buf != end) {
		state->proc_mii.m = (u16)new_reg;
		return 0;
	} else
		return -EINVAL;
}

/*! get the register content of state->proc_mii */
static int ip175c_get_val(void *driver, char *buf, int nr)
{
	switch_driver *dev = driver;
	struct ip175c_state *state = dev->priv;
	int val;

	val = getPhy(state, state->proc_mii);

	if (val < 0) {
		state->proc_errno = val;
		return sprintf(buf, "error\n");
	} else {
		state->proc_errno = 0;
		return sprintf(buf, "0x%04x\n", val);
	}
}

/*! write a value to the register defined by phy/reg above */
static int ip175c_set_val(void *driver, char *buf, int nr)
{
	switch_driver *dev = driver;
	struct ip175c_state *state = dev->priv;
	char *end;
	unsigned long val;

	if (buf[0] == '0' && buf[1] == 'x') {
		unsigned myval = (unsigned)-1; // should fail check below.
		sscanf(buf+2, "%x", &myval);
		val = myval;
	} else {
		val = simple_strtoul(buf, &end, 0);
	}
	if (buf != end && val <= 0xffff) {
		state->proc_errno = setPhy(state, state->proc_mii, (u16)val);
	} else {
		state->proc_errno = -EINVAL;
	}
	return state->proc_errno;
}

/*! get the errno of the last read/write of "val" */
static int ip175c_get_errno(void *driver, char *buf, int nr)
{
	switch_driver *dev = driver;
	struct ip175c_state *state = dev->priv;

	return sprintf(buf, "%d\n", state->proc_errno);
}

static int ip175c_read_name(void *driver, char *buf, int nr)
{
	switch_driver *dev = driver;
	struct ip175c_state *state = dev->priv;

	return sprintf(buf, "%s\n", state->regs->NAME);
}


static int ip175c_get_enabled(void *driver, char *buf, int nr)
{
	switch_driver *dev = driver;
	struct ip175c_state *state = dev->priv;
	int err;
	int enabled = 1;

	err = get_flags(state);
	if (err < 0)
		return err;

	if (REG_SUPP(state->regs->MII_REGISTER_EN)){
		int val = getPhy(state, state->regs->MII_REGISTER_EN);
		if (val < 0) {
			return val;
		}
		enabled = ((val>>state->regs->MII_REGISTER_EN_BIT) & 1);
	}
	return sprintf(buf, "%d\n", enabled);
}

static int ip175c_get_port_status(void *driver, char *buf, int nr)
{
	switch_driver *dev = driver;
	struct ip175c_state *state = dev->priv;
	struct phy_device *phy;
	int ctrl, speed, status;
	int len;

	if (nr == state->regs->CPU_PORT) {
		return sprintf(buf, "up, 100 Mbps, full duplex, cpu port\n");
	}

	if (nr >= dev->ports || nr < 0)
		return -EINVAL;
	phy = state->ports[nr].phy;
	if (!phy)
		return -EINVAL;

	ctrl = phy_read(phy, 0);
	status = phy_read(phy, 1);
	speed = phy_read(phy, 18);
	if (ctrl < 0 || status < 0 || speed < 0)
		return -EIO;
	pr_debug("IP175C: Port %d: ctrl=%04x status=%04x speed=%04x\n", nr, ctrl, status, speed);

	if (status & 4)
		len = sprintf(buf, "up, %d Mbps, %s duplex",
			((speed & (1<<11)) ? 100 : 10),
			((speed & (1<<10)) ? "full" : "half"));
	else
		len = sprintf(buf, "down");

	if (ctrl & (1<<12)) {
		len += sprintf(buf+len, ", auto-negotiate");
		if (!(status & (1<<5)))
			len += sprintf(buf+len, " (in progress)");
	} else {
		len += sprintf(buf+len, ", fixed speed");
	}

	buf[len++] = '\n';
	buf[len] = '\0';
	return len;
}

static const switch_config ip175c_config[] = {
	{
		.name  = "reset",
		.read  = ip175c_get_enabled,
		.write = ip175c_reset,
	},
	{
		.name  = "enable",
		.read  = ip175c_get_enabled,
		.write = ip175c_apply,
	},
	{
		.name  = "enable_vlan",
		.read  = ip175c_get_enable_vlan,
		.write = ip175c_set_enable_vlan,
	},
	{
		.name  = "enable_router",
		.read  = ip175c_get_router_mode,
		.write = ip175c_set_router_mode,
	},
	{
		.name  = "name",
		.read  = ip175c_read_name,
		.write = NULL,
	},
	/* jal: added for low level debugging etc. */
	{
		.name  = "phy",
		.read  = ip175c_get_phy,
		.write = ip175c_set_phy,
	},
	{
		.name  = "reg",
		.read  = ip175c_get_reg,
		.write = ip175c_set_reg,
	},
	{
		.name  = "val",
		.read  = ip175c_get_val,
		.write = ip175c_set_val,
	},
	{
		.name  = "errno",
		.read  = ip175c_get_errno,
		.write = NULL,
	},

	{ NULL, }
};

static const switch_config ip175c_vlan[] = {

	{
		.name  = "ports",
		.read  = ip175c_get_ports,
		.write = ip175c_set_ports,
	},
	{ NULL, }
};

static const switch_config ip175c_port[] = {

	{
		.name  = "status",
		.read  = ip175c_get_port_status,
		.write = NULL,
	},
	{ NULL, }
};

static void ip175c_cleanup(void *driver)
{
	switch_driver *dev = driver;
	struct ip175c_state *state = dev->priv;
	int i;

	pr_debug("IP175C: Cleaning up %s\n", dev->interface);
	for (i=0; i<MAX_PORTS; i++)
		if (state->ports[i].phy)
			put_device(&state->ports[i].phy->dev);
	list_del(&state->list);
	kfree(state);
}

static struct ip175c_state *lookup_state(struct mii_bus *bus)
{
	struct list_head *pos;
	struct ip175c_state *state;
	switch_driver *dev;
	int err;

	list_for_each(pos, &state_list) {
		state = list_entry(pos, struct ip175c_state, list);
		if (state->mii_bus == bus)
			return state;
	}

	pr_debug("IP175C: New state for busname=%s busid=%s busdev=%s\n", bus->name, bus->id, bus->dev->bus_id);

	dev = kcalloc(1, sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return NULL;
	dev->name = DRIVER_NAME;
	dev->version = DRIVER_VERSION;
	dev->vlans = MAX_VLANS,
	dev->driver_handlers = ip175c_config;
	dev->vlan_handlers = ip175c_vlan;
	dev->port_handlers = ip175c_port;
	dev->cleanup = ip175c_cleanup;

	state = kcalloc(1, sizeof(*state), GFP_KERNEL);
	if (!state)
		goto bad2;
	list_add_tail(&state->list, &state_list);
	state->mii_bus = bus;
	state->dev = dev;
	dev->priv = state;
	dev->interface = bus->id;

	if ((err = get_model(state)))
		goto bad;

	dev->cpuport = state->regs->CPU_PORT;
	dev->ports = state->regs->NUM_PORTS;
	printk(KERN_INFO "switch-ip175c: Found a %s chip at %s, id %s\n", state->regs->NAME, bus->name, bus->id);

	if (switch_register_driver(dev) < 0)
		goto bad;

	return state;

bad:
	ip175c_cleanup(dev);
bad2:
	kfree(dev);
	return NULL;
}

#define PRINT_FOR_EACH_PHY // slightly increases loading times but could be useful.
static int __init ip175c_init_dev(struct device *kdev, void *data)
{
	struct ip175c_state *state;
	struct phy_device *phy;

	pr_debug("IP175C: Found PHY: bus_id=%s\n", kdev->bus_id);
	phy = to_phy_device(kdev);
	state = lookup_state(phy->bus);
	if (state) {
#ifdef PRINT_FOR_EACH_PHY
		printk(KERN_DEBUG "switch-ip175c: Found %s switch at bus id %s (PHY %d)\n", state->regs->NAME, kdev->bus_id, phy->addr);
#endif
		if (phy->addr < state->regs->NUM_PORTS) {
			state->ports[phy->addr].phy = phy;
			get_device(kdev);
		}
	}
	return 0;
}

int __init ip175c_init(void)
{
	struct device_driver *drv = driver_find("ICPlus", &mdio_bus_type);
	if (!drv) {
		printk(KERN_INFO "switch-ip175c: No ICPlus driver found.\n");
		pr_info("IP175C: No ICPlus driver found.\n");
		return -ENOENT;
	}
	INIT_LIST_HEAD(&state_list);
	driver_for_each_device(drv, NULL, NULL, ip175c_init_dev);
	put_driver(drv);
	return 0;
}

void __exit ip175c_exit(void)
{
	switch_unregister_driver(DRIVER_NAME);
}

MODULE_AUTHOR("Patrick Horn <patrick.horn@gmail.com>");
MODULE_LICENSE("GPL");

module_init(ip175c_init);
module_exit(ip175c_exit);

