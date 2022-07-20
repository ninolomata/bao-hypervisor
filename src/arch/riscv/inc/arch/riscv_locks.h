/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 * Copyright (c) 2021 Christoph Müllner <cmuellner@linux.com>
 */

#ifndef __RISCV_LOCKS_H__
#define __RISCV_LOCKS_H__

#include <bao.h>

#define TICKET_SHIFT	16

typedef struct {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
       uint16_t next;
       uint16_t owner;
#else
       uint16_t owner;
       uint16_t next;
#endif
} __attribute__((aligned(4)))spinlock_t;

#define SPINLOCK_INITVAL	\
	(spinlock_t) { 0, 0 }

#define SPIN_LOCK_INIT(x)	\
	x = SPINLOCK_INITVAL

#define SPIN_LOCK_INITIALIZER	\
	SPINLOCK_INITVAL

#define DEFINE_SPIN_LOCK(x)	\
	spinlock_t SPIN_LOCK_INIT(x)

bool spin_lock_check(spinlock_t *lock);

bool spin_trylock(spinlock_t *lock);

void spin_lock(spinlock_t *lock);

void spin_unlock(spinlock_t *lock);

#endif
