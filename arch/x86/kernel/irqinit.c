// SPDX-License-Identifier: GPL-2.0
#include <linux/linkage.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/timex.h>
#include <linux/random.h>
#include <linux/kprobes.h>
#include <linux/init.h>
#include <linux/kernel_stat.h>
#include <linux/device.h>
#include <linux/bitops.h>
#include <linux/acpi.h>
#include <linux/io.h>
#include <linux/delay.h>

#include <linux/atomic.h>
#include <asm/timer.h>
#include <asm/hw_irq.h>
#include <asm/pgtable.h>
#include <asm/desc.h>
#include <asm/apic.h>
#include <asm/setup.h>
#include <asm/i8259.h>
#include <asm/traps.h>
#include <asm/prom.h>

#include <wl_debug.h>

/*
 * ISA PIC or low IO-APIC triggered (INTA-cycle or APIC) interrupts:
 * (these are usually mapped to vectors 0x30-0x3f)
 */

/*
 * The IO-APIC gives us many more interrupt sources. Most of these
 * are unused but an SMP system is supposed to have enough memory ...
 * sometimes (mostly wrt. hw bugs) we get corrupted vectors all
 * across the spectrum, so we really want to be prepared to get all
 * of these. Plus, more powerful systems might have more than 64
 * IO-APIC registers.
 *
 * (these are usually mapped into the 0x30-0xff vector range)
 */

/*
 * IRQ2 is cascade interrupt to second interrupt controller
 */
static struct irqaction irq2 = {
	.handler = no_action,
	.name = "cascade",
	.flags = IRQF_NO_THREAD,
};

DEFINE_PER_CPU(vector_irq_t, vector_irq) = {
	[0 ... NR_VECTORS - 1] = VECTOR_UNUSED,
};

void __init init_ISA_irqs(void)
{
	struct irq_chip *chip = legacy_pic->chip;
	int i;

	/*
	 * Try to set up the through-local-APIC virtual wire mode earlier.
	 *
	 * On some 32-bit UP machines, whose APIC has been disabled by BIOS
	 * and then got re-enabled by "lapic", it hangs at boot time without this.
	 */
	init_bsp_APIC();

	legacy_pic->init(0);

	for (i = 0; i < nr_legacy_irqs(); i++) // 16
		irq_set_chip_and_handler(i, chip, handle_level_irq);
}

void __init init_IRQ(void)
{
	int i;

	/*
	 * On cpu 0, Assign ISA_IRQ_VECTOR(irq) to IRQ 0..15.
	 * If these IRQ's are handled by legacy interrupt-controllers like PIC,
	 * then this configuration will likely be static after the boot. If
	 * these IRQ's are handled by more mordern controllers like IO-APIC,
	 * then this vector space can be freed and re-used dynamically as the
	 * irq's migrate etc.
	 */
	for (i = 0; i < nr_legacy_irqs(); i++) // nr_legacy_irqs() = 16
		per_cpu(vector_irq, 0)[ISA_IRQ_VECTOR(i)] = irq_to_desc(i);
	wl_printk("in arch/x86/kernel/irqinit.c init_IRQ()\nnr_legacy_irqs() = %d", nr_legacy_irqs());

	BUG_ON(irq_init_percpu_irqstack(smp_processor_id()));

	// x86_init = x86_init_ops defined in arch/x86/include/asm/x86_int.h
	// x86_init.irqs = x86_init_irqs
		/**
 		* struct x86_init_irqs - platform specific interrupt setup
		 * @pre_vector_init:		init code to run before interrupt vectors
		 *				are set up.
		 * @intr_init:			interrupt init code
		 * @trap_init:			platform specific trap setup
		 * @intr_mode_init:		interrupt delivery mode setup
		 */
		/* struct x86_init_irqs {
    			void (*pre_vector_init)(void);
				void (*intr_init)(void);
				void (*trap_init)(void);
				void (*intr_mode_init)(void);
			};*/
	x86_init.irqs.intr_init();

	// intr_init		= native_init_IRQ in arch/x86/kernel/x86_init.c, the later is defined below
}

void __init native_init_IRQ(void)
{
	/* Execute any quirks before the call gates are initialised: */
	x86_init.irqs.pre_vector_init(); // to init_ISA_irqs() in this file

	// defined in arch/x86/kernel/idt.c
	idt_setup_apic_and_irq_gates();
	// defined in arch/x86/kernel/apic/vector.c
	lapic_assign_system_vectors();

	if (!acpi_ioapic && !of_ioapic && nr_legacy_irqs())
		setup_irq(2, &irq2);
}
