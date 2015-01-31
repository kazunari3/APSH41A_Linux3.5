/*
 * .../mach-ap4a2a/irq.c
 *
 * ALPHAPROJECT AP-SH4A-2A Support.
 *
 * Copyright (C) 2009 ALPHAPROJECT.
 * Copyright (C) 2008 Renesas Solutions Corp.
 * Copyright (C) 2008  Nobuhiro Iwamatsu <iwamatsu.nobuhiro@renesas.com>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/init.h>
#include <linux/irq.h>
#include <asm/io.h>
#include <asm/irq.h>
//#include <mach/sh7763rdp.h>

#define INTC_BASE		(0xFFD00000)
#define INTC_INT2PRI7   (INTC_BASE+0x4001C)
#define INTC_INT2MSKCR	(INTC_BASE+0x4003C)
#define INTC_INT2MSKCR1	(INTC_BASE+0x400D4)

#define HL_FPGA_IRQ_BASE	0 //200
#define HL_NR_IRL		15

#define IRQ_CF		(HL_FPGA_IRQ_BASE + evt2irq(0x360))
#define IRQ_AX88796		(HL_FPGA_IRQ_BASE + evt2irq(0x2A0))

enum {
	UNUSED = 0,

	/* board specific interrupt sources */

	AX88796,          /* Ethernet controller */
	CF,               /* Compact Flash */

};

static struct intc_vect vectors[] __initdata = {
	INTC_IRQ(AX88796, IRQ_AX88796),
	INTC_IRQ(CF, IRQ_CF),

};

static struct intc_mask_reg mask_registers[] __initdata = {
	{ 0xa5000000, 0, 16, /* IRLMSK */
	  {0, 0, 0, 0, 0, 0, 0, 0,
	    0, 0, 0, 0, 0, 0, CF, AX88796 } },
};

#if 0
static unsigned char irl2irq[HL_NR_IRL] __initdata = {
	65, 66, 67, 68,
	IRQ_CF, 0, 0, 0,
	0, 0, 0, 0,
	IRQ_AX88796, IRQ_PSW
};
#endif

static DECLARE_INTC_DESC(intc_desc, "r7780rp", vectors,
			 NULL, mask_registers, NULL, NULL);

/*
 * Initialize IRQ setting
 */
void __init init_sh7763rdp_IRQ(void)
{

#if 0
	/* GPIO enabled */
	ctrl_outl(1 << 25, INTC_INT2MSKCR);

	/* enable GPIO interrupts */
	ctrl_outl((ctrl_inl(INTC_INT2PRI7) & 0xFF00FFFF) | 0x000F0000,
		  INTC_INT2PRI7);

	/* USBH enabled */
	ctrl_outl(1 << 17, INTC_INT2MSKCR1);

	/* GETHER enabled */
	ctrl_outl(1 << 16, INTC_INT2MSKCR1);

	/* DMAC enabled */
	ctrl_outl(1 << 8, INTC_INT2MSKCR);
#endif
	register_intc_controller(&intc_desc);
plat_irq_setup_pins(IRQ_MODE_IRQ);
printk("%s... doen\n", __FUNCTION__); 
}
