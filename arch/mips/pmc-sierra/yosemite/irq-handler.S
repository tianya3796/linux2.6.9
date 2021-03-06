/*
 * Copyright 2003 PMC-Sierra Inc.
 * Author: Manish Lachwani (lachwani@pmc-sierra.com
 *
 * First-level interrupt router for the PMC-Sierra Titan board
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * Titan supports Hypertransport or PCI but not both. Hence, one interrupt
 * line is shared between the PCI slot A and Hypertransport. This is the
 * Processor INTB #0.
 */

#include <linux/config.h>
#include <asm/asm.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/regdef.h>
#include <asm/stackframe.h>

		.align	5
		NESTED(titan_handle_int, PT_SIZE, sp)
		SAVE_ALL
		CLI
		.set	at
		.set	noreorder
		mfc0	t0, CP0_CAUSE
		mfc0	t2, CP0_STATUS

		and	t0, t2

		andi	t1, t0, STATUSF_IP2	/* INTB0 hardware line */
		bnez	t1, ll_pcia_irq		/* 64-bit PCI */
		andi	t1, t0, STATUSF_IP3	/* INTB1 hardware line */
		bnez	t1, ll_pcib_irq		/* second 64-bit PCI slot */
		andi	t1, t0, STATUSF_IP4	/* INTB2 hardware line */
		bnez	t1, ll_duart_irq	/* UART	*/
		andi    t1, t0, STATUSF_IP5	/* SMP inter-core interrupts */
		bnez    t1, ll_smp_irq
		andi	t1, t0, STATUSF_IP6
		bnez	t1, ll_ht_irq		/* Hypertransport */
		andi	t1, t0, STATUSF_IP7	/* INTB5 hardware line */
		bnez	t1, ll_timer_irq	/* Timer */

		nop
		nop

		/* Extended interrupts */
                mfc0    t0, CP0_CAUSE
                cfc0    t1, CP0_S1_INTCONTROL

                sll     t2, t1, 8

                and     t0, t2
                srl     t0, t0, 16

		.set	reorder

		j	spurious_interrupt
		nop
		END(titan_handle_int)

		.align	5

ll_pcia_irq:
		li	a0, 2
		move	a1, sp
#ifdef CONFIG_HYPERTRANSPORT
		jal	ll_ht_smp_irq_handler
#else
		jal	do_IRQ
#endif
		j	ret_from_irq

ll_pcib_irq:
		li	a0, 3
		move	a1, sp
		jal	do_IRQ
		j	ret_from_irq

ll_duart_irq:
		li	a0, 4
		move	a1, sp
		jal	do_IRQ
		j	ret_from_irq

ll_smp_irq:
		li	a0, 5
		move	a1, sp
#ifdef CONFIG_SMP
		jal	jaguar_mailbox_irq
#else
		jal	do_IRQ
#endif
		j	ret_from_irq

ll_ht_irq:
		li	a0, 6
		move	a1, sp
		jal	ll_ht_smp_irq_handler
		j	ret_from_irq

ll_timer_irq:
		li	a0, 7
		move	a1, sp
		jal	do_IRQ
		j	ret_from_irq
