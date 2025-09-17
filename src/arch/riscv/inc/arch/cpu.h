/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#ifndef __ARCH_CPU_H__
#define __ARCH_CPU_H__

#include <bao.h>
#ifdef __CHERI__
#include <arch/cheri.h>
#include <arch/cheri_utils.h>
#endif

#define CPU_HAS_EXTENSION(EXT) (DEFINED(EXT))

extern cpuid_t CPU_MASTER;

struct cpu_arch {
    unsigned hart_id;
    unsigned plic_cntxt;
};

static inline struct cpu* cpu(void)
{
 void * tp_cap = cheri_build_data_cap(BAO_CPU_BASE, BAO_VM_BASE-BAO_CPU_BASE ,CHERI_HYP_DATA_PERMS);
  return tp_cap;
}

#endif /* __ARCH_CPU_H__ */
