/*
 * arch/arm/kernel/fcse.c
 *
 * Helper functions for using the ARM Fast Context Switch Extension with
 * processors supporting it.
 *
 * Copyright (C) 2008 Richard Cochran
 * Copyright (C) 2009-2011 Gilles Chanteperdrix <gch@xenomai.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/bitops.h>
#include <linux/memory.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/kernel_stat.h>
#include <linux/mman.h>
#include <linux/dcache.h>
#include <linux/fs.h>
#include <linux/hardirq.h>

#include <asm/fcse.h>
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>

#define NR_PIDS (TASK_SIZE / FCSE_PID_TASK_SIZE)
#define PIDS_LONGS ((NR_PIDS + BITS_PER_LONG - 1) / BITS_PER_LONG)

static DEFINE_RAW_SPINLOCK(fcse_lock);
static unsigned long fcse_pids_bits[PIDS_LONGS];

static inline void fcse_pid_reference_inner(unsigned pid)
{
	__set_bit(pid, fcse_pids_bits);
}

static inline void fcse_pid_dereference(struct mm_struct *mm)
{
	unsigned fcse_pid = mm->context.fcse.pid >> FCSE_PID_SHIFT;

	__clear_bit(fcse_pid, fcse_pids_bits);
}

static inline unsigned find_free_pid(unsigned long bits[])
{
	unsigned fcse_pid;

	fcse_pid = find_next_zero_bit(bits, NR_PIDS, 1);
	if (fcse_pid == NR_PIDS)
		/* Allocate zero pid last, since zero pid is also used by
		   processes with address space larger than 32MB in
		   best-effort mode. */
		if (!test_bit(0, bits))
			fcse_pid = 0;

	return fcse_pid;
}

void fcse_pid_free(struct mm_struct *mm)
{
	unsigned long flags;

	raw_spin_lock_irqsave(&fcse_lock, flags);
	fcse_pid_dereference(mm);
	raw_spin_unlock_irqrestore(&fcse_lock, flags);
}

int fcse_pid_alloc(struct mm_struct *mm)
{
	unsigned long flags;
	unsigned fcse_pid;

	raw_spin_lock_irqsave(&fcse_lock, flags);
	fcse_pid = find_free_pid(fcse_pids_bits);
	if (fcse_pid == NR_PIDS) {
		/* Allocate zero pid last, since zero pid is also used by
		   processes with address space larger than 32MB in
		   best-effort mode. */
		raw_spin_unlock_irqrestore(&fcse_lock, flags);
#ifdef CONFIG_ARM_FCSE_MESSAGES
		printk(KERN_WARNING "FCSE: %s[%d] would exceed the %lu processes limit.\n",
		       current->comm, current->pid, NR_PIDS);
#endif /* CONFIG_ARM_FCSE_MESSAGES */
		return -EAGAIN;
#endif /* CONFIG_ARM_FCSE_GUARANTEED */
	}
	fcse_pid_reference_inner(fcse_pid);
	raw_spin_unlock_irqrestore(&fcse_lock, flags);

	return fcse_pid;
}
