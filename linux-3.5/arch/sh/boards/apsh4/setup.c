#include <linux/platform_device.h>
//#include <asm/sh_eth.h>
#include <linux/ata_platform.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <asm/io.h>
#include <asm/machvec.h>

#include <linux/mtd/partitions.h>
#include <linux/mtd/physmap.h>

#undef ax88

#ifdef ax88
#include <net/ax88796.h>
#endif

#include <linux/irq.h>
#include <linux/interrupt.h>

//#define CF_DEBUG 1

#define CF_BASE_PHY	0x14000000
#define CF_IO_SIZE	0x1000
//#define CF_OFFSET	0x0
#define CF_OFFSET	0x1f0
#define IRQ_AX	(evt2irq(0x2A0))	
#define IRQ_CFCARD	(evt2irq(0x360))

#define CCN_CVR		0xff000040

#define AX_BASE_PHY	0x10000000

extern void init_sh7763rdp_IRQ(void);

static unsigned long cf_io_base = 0;

static void release_platform_dev(struct device * dev)
{
	dev->parent = NULL;
}

static struct resource cf_resources[] = {
	[0] = {
		.start	= CF_OFFSET,
		.end	= CF_OFFSET + 0x0f,
		.flags	= IORESOURCE_IO,
	},
	[1] = {
		.start	= CF_OFFSET + 0x206,
		.end	= CF_OFFSET + 0x20f,
		.flags	= IORESOURCE_IO,
	},
	[2] = {
		.start	= IRQ_CFCARD,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct pata_platform_info cf_info = {
	.ioport_shift	= 0,
};

static struct platform_device cf_device  = {
	.name		= "pata_platform",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(cf_resources),
	.resource	= cf_resources,
	.dev	= {
		.platform_data	= &cf_info,
	},
};




static struct mtd_partition nor_flash_partitions[] =
{
	{
		.name = "ipl",
		.offset = 0,
		.size = 0x00010000,
	},
	{
		.name = "sh-ipl+g",
		.offset = MTDPART_OFS_APPEND,
		.size = 0x00080000,
	},
/*
	{
		.name = "kernel",
		.offset = MTDPART_OFS_APPEND,
		.size = 0x00400000,
	},
*/
	{
		.name = "other",
		.offset = MTDPART_OFS_APPEND,
		.size = MTDPART_SIZ_FULL,
	},
};

static struct physmap_flash_data nor_flash_data = {
	.width		= 2,
	.parts		= nor_flash_partitions,
	.nr_parts	= ARRAY_SIZE(nor_flash_partitions),
};

static struct resource nor_flash_resources[] = {
	[0] = {
		.name	= "NOR Flash",
		.start	= 0x00000000,
		.end	= 0x003FFFFF,
		.flags	= IORESOURCE_MEM,
	}
};

static struct platform_device nor_flash_device = {
	.name		= "physmap-flash",
	.resource	= nor_flash_resources,
	.num_resources	= ARRAY_SIZE(nor_flash_resources),
	.dev		= {
		.platform_data = &nor_flash_data,
	},
};


/*
 * ax8769
 */
#ifdef ax88
static struct ax_plat_data ax88796_platdata = {
	.flags          = AXFLG_HAS_93CX6,
	.wordlength     = 1,
	.dcr_val        = 0x1,
	.rcr_val        = 0x40,
};
	
static struct resource ax88796_resources[] = {
	[0] = {
		.start  = AX_BASE_PHY,
		.end    = AX_BASE_PHY +  0xFFFFF, //(0x20 * 0x2) - 1,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_AX,
		.end    = IRQ_AX,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device ax88796_device = {
	.name           = "ax88796",
	.id             = 0,

	.dev    = {
		.platform_data = &ax88796_platdata,
	},

	.num_resources  = ARRAY_SIZE(ax88796_resources),
	.resource       = ax88796_resources,
};
	
#else
static struct resource ax88796_resources[] = {
	[0] = {
		.start  = AX_BASE_PHY,
		.end    = AX_BASE_PHY +  0xFFFFF, //(0x20 * 0x2) - 1,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_AX,
		.end    = IRQ_AX,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device ax88796_device = {
	.name           = "ax88796b",
	.id             = 0,

	.num_resources  = ARRAY_SIZE(ax88796_resources),
	.resource       = ax88796_resources,
};
#endif

static struct platform_device *apsh4a2a_devices[] __initdata = {
	//&apsh4a2a_eth0_device,
	//&apsh4a2a_eth1_device,
	&cf_device,
	&nor_flash_device,
	&ax88796_device,
};

static int __init sh2004_io_init(void)
{
	pgprot_t prot;

	//prot = PAGE_KERNEL_PCC(1, _PAGE_PCC_IO16); /* TC=AREA5 */
	//cf_io_base    = (unsigned long)p3_ioremap(CF_BASE_PHY,    CF_IO_SIZE ,   prot.pgprot);


	printk("sh2004_io_init end!");
	return 0;
}
subsys_initcall (sh2004_io_init);

#define CF_PORT(x)	((x & ~0xfffff20f) == CF_OFFSET)
static void __iomem *sh2004_ioport_map(unsigned long offset, unsigned int size)
{

	if(CF_PORT(offset)) {			/* Compact Flash (1f0-1ff, 3f0-3ff) */
#ifdef CF_DEBUG
		printk("CF: ioport_map(0x%lx)=0x%lx\n", offset, cf_io_base + (offset & 0xfff));
#endif
		return (void __iomem *)(cf_io_base + (offset & 0xfff));
	} else {
		printk("No No No mapppppppppppppp¥n");
	}

	return (void __iomem *)(NULL);
}


#define IODELAY()	ndelay(200)
#if 0
static u8 sh2004_inb(unsigned long port)
{
	unsigned long addr;
	u8 ret;

	addr = (unsigned long)sh2004_ioport_map(port,1);
	ret = ctrl_inb(addr);
#ifdef CF_DEBUG
	if (CF_PORT(port)) printk("CF: inb(0x%lx:0x%lx)=0x%02x\n", port, addr, ret);
#endif
	return ret;
}

static u32 sh2004_inl(unsigned long port)
{
	unsigned long addr;

	addr = (unsigned long)sh2004_ioport_map(port,4);

	return ctrl_inl(addr);
}

static u32 sh2004_inl_p(unsigned long port)
{
	unsigned long v = sh2004_inl(port);

	IODELAY();
	return v;
}

static void sh2004_outb(u8 b, unsigned long port)
{
	unsigned long addr;

	addr = (unsigned long)sh2004_ioport_map(port,1);
	ctrl_outb(b, addr);
#ifdef CF_DEBUG
	if (CF_PORT(port)) printk("CF: outb(0x%02x, 0x%lx:0x%lx)\n", b, port, addr);
#endif
}

static void sh2004_outl(u32 b, unsigned long port)
{
	unsigned long addr;

	addr = (unsigned long)sh2004_ioport_map(port,4);

	ctrl_outl(b, addr);
}

static void sh2004_outl_p(u32 b, unsigned long port)
{
	sh2004_outl(b, port);
	IODELAY();
}

#endif
#if 0
static void __iomem *sh2004_ioremap(unsigned long phys, unsigned long size, unsigned long flags)
{
	pgprot_t prot;

	if (phys >= P4SEG || (phys < P3SEG && phys >= P2SEG))
		return (void __iomem *)phys;
	if (phys >= SMC_DATA_REG_BASE && phys+size < SMC_DATA_REG_BASE+SMC_IO_SIZE)
		return (void __iomem *)(phys & (SMC_IO_SIZE-1));
	if (phys >= SMC_BASE_PHY && phys+size < SMC_BASE_PHY+SMC_IO_SIZE)
		return (void __iomem *)(phys & (SMC_IO_SIZE-1));
	if (phys >= 16*1024*1024 || size > 16*1024*1024 || phys+size > 16*1024*1024)
		return NULL;
	prot = PAGE_KERNEL_PCC(0, _PAGE_PCC_ATR16);
	return p3_ioremap(phys+PC104_BASE_PHY, size, prot.pgprot);
}
#endif

static int sh2004_irq_demux(int irq)
{
#if 0
	if ((unsigned int)irq < 15)
		return irq+1;
#endif
	return irq;
}

/*
 * Initialize the board
 */
static void __init sh2004_setup(char **cmdline_p)
{
#define BCR2_A		0xFF800004
	//ctrl_outw(0xA9F9, BCR2_A); // AREA4 8bit-bus
	__raw_writew(0xA9F9, BCR2_A); // AREA4 8bit-bus
#define WCR1		0xff800008
#define A4IW_VALUE	(3UL << 16)
#define A4IW_MASK	(7UL << 16)

#define WCR2		0xff80000c
#define A4W_VALUE	(3UL << 17)
#define A4W_MASK	(7UL << 17)

#define WCR3		0xff800010
	unsigned long wcr1, new_wcr1, wcr2, new_wcr2;
	unsigned long wcr3, bcr2;

#if 1
	wcr1 = __raw_readl(WCR1);
	new_wcr1 = (wcr1 & ~A4IW_MASK) | A4IW_VALUE;
	__raw_writel(new_wcr1, WCR1);
	printk("WCR1 0x%08lx -> 0x%08lx\n", wcr1, new_wcr1);

	wcr2 = __raw_readl(WCR2);
	new_wcr2 = (wcr2 & ~A4W_MASK) | A4W_VALUE;
	//__raw_writel(new_wcr2, WCR2);
	printk("WCR2 0x%08lx -> 0x%08lx\n", wcr2, new_wcr2);

#endif
	//bcr2 = ctrl_inw(BCR2_A);
	bcr2 = __raw_readw(BCR2_A);
	printk("BCR2 0x%08lx \n", bcr2);

	//wcr1 = ctrl_inl(WCR1);
	wcr1 = __raw_readw(WCR1);
	printk("WCR1 0x%08lx \n", wcr1);
	//wcr2 = ctrl_inl(WCR2);
	wcr2 = __raw_readw(WCR2);
	printk("WCR2 0x%08lx \n", wcr2);
	//wcr3 = ctrl_inl(WCR3);
	wcr3 = __raw_readw(WCR3);
	printk("WCR3 0x%08lx \n", wcr3);

	//plat_irq_setup_pins(IRQ_MODE_IRQ);

	printk(KERN_INFO "SH-2004 Setup...done.\n");
}

/*
 * The Machine Vector
 */

struct sh_machine_vector mv_sh2004 __initmv = {
	.mv_name		= "sh2004",
	.mv_setup		= sh2004_setup,
	//.mv_nr_irqs		= NR_IRQS,
#if 0
	.mv_inb			= sh2004_inb,
	.mv_inl			= sh2004_inl,
	.mv_outb		= sh2004_outb,
	.mv_outl		= sh2004_outl,

	.mv_inl_p		= sh2004_inl_p,
	.mv_outl_p		= sh2004_outl_p,
#endif
	.mv_init_irq		= init_sh7763rdp_IRQ,

	.mv_ioport_map		= sh2004_ioport_map,

	//.mv_ioremap		= sh2004_ioremap,
	.mv_irq_demux		= sh2004_irq_demux,

	//.mv_machine_halt	= sh2004_machine_halt,
};

#define CONFIG_CIS_BASE		0xD0005000

static int __init apsh4a2a_devices_setup(void)
{
	volatile int i = 0;
	printk("%s:\n", __func__);

	pgprot_t prot;
	unsigned long paddrbase;
	//void *cf_ide_base;

	/* open I/O area window */
	paddrbase = virt_to_phys((void *)PA_AREA5_IO);
	prot = PAGE_KERNEL_PCC(1, _PAGE_PCC_IO16);
	cf_io_base = ioremap_prot(paddrbase, PAGE_SIZE, pgprot_val(prot));
	if (!cf_io_base) {
		printk("allocate_cf_area : can't open CF I/O window!\n");
		return -ENOMEM;
	}

	/* IDE cmd address : 0x1f0-0x1f7 and 0x3f6 */
	cf_resources[0].start += (unsigned long)cf_io_base;
	cf_resources[0].end   += (unsigned long)cf_io_base;
	cf_resources[1].start += (unsigned long)cf_io_base;
	cf_resources[1].end   += (unsigned long)cf_io_base;

	return platform_add_devices(apsh4a2a_devices,
				    ARRAY_SIZE(apsh4a2a_devices));
}

device_initcall(apsh4a2a_devices_setup);
