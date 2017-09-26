#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <asm/cputype.h>
#include <asm/io.h>
//#define ID_AA64DFR0_EL1 (0xd28)

MODULE_LICENSE("GPL v2");
#define MFS_REG_START 0x152E0000
#define MFS_REG_SIZE 0x10000

//extern int perf_num_counters(void);
//extern int armpmu_get_max_events(void);
//extern void* __initdata;
extern void __iomem* mfc_regs_base;
extern void s5p_mfc_dump_regs2(void __iomem *dev);
extern spinlock_t* mfc_irqlock;
extern struct mutex* mfc_mutexx;


int init_module(void)
{
   unsigned long flags;
   void __iomem * base;
   //base = mfc_regs_base;
   //unsigned int i = 0;
   printk(KERN_INFO "Mapping IOMEM\n");
   base = ioremap(MFS_REG_START, MFS_REG_SIZE);
   if(base == NULL) {
      printk(KERN_ERR "ioremap failed\n");
      return -1;
   }
   printk(KERN_INFO "Mapping IOMEM ok %x -> %p\n", MFS_REG_START, base);
   mutex_lock(mfc_mutexx);
   spin_lock_irqsave(mfc_irqlock, flags);
   writel(0xFFFFFFFF, base + 0x1060);
   //readl(base + 0x0);
   spin_unlock_irqrestore(mfc_irqlock, flags);
   mutex_unlock(mfc_mutexx);
   //readl(base + 0x2068);
   //ioread8(base);
   //s5p_mfc_dump_regs2(base);
   //writel(0xFFFFFFFF, base + 0x1060);
   //print_hex_dump(KERN_ERR, "", DUMP_PREFIX_ADDRESS, 32, 4, base + 0, 0x80, false);
   //printk(KERN_INFO "IOMEM[0] %x\n", *(unsigned int*)base);
   //readl(base);
   //for(i=0; i<MFS_REG_SIZE; ++i)
   //{
   //   readl(base + i);
   //}
   /*
   unsigned long int dfr = 0;
   int ne = 0;
   int nc = 0;

	printk(KERN_INFO "fdt_phys %p\n", __initdata);

	printk(KERN_INFO "Hello world 1.\n");
   asm volatile("msr pmuserenr_el0, %0" :: "r" (1));

   nc = perf_num_counters();
	printk(KERN_INFO "nc %x\n", nc);

   ne = armpmu_get_max_events();
	printk(KERN_INFO "ne %x\n", ne);

   dfr = read_cpuid(ID_AA64DFR0_EL1);
	printk(KERN_INFO "dfr %lx\n", dfr);
	printk(KERN_INFO "dfr test %lx\n", (dfr >> 8) & 0xf);
   */
	return 0;
}

void cleanup_module(void)
{
   //asm volatile("msr pmuserenr_el0, %0" :: "r" (0));
	printk(KERN_INFO "Goodbye world 1.\n");
}
