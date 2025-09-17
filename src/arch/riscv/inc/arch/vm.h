/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#ifndef __ARCH_VM_H__
#define __ARCH_VM_H__

#include <bao.h>
#include <irqc.h>
#include <arch/sbi.h>

#define REG_RA  (1)
#define REG_SP  (2)
#define REG_GP  (3)
#define REG_TP  (4)
#define REG_T0  (5)
#define REG_T1  (6)
#define REG_T2  (7)
#define REG_S0  (8)
#define REG_S1  (9)
#define REG_A0  (10)
#define REG_A1  (11)
#define REG_A2  (12)
#define REG_A3  (13)
#define REG_A4  (14)
#define REG_A5  (15)
#define REG_A6  (16)
#define REG_A7  (17)
#define REG_S2  (18)
#define REG_S3  (19)
#define REG_S4  (20)
#define REG_S5  (21)
#define REG_S6  (22)
#define REG_S7  (23)
#define REG_S8  (24)
#define REG_S9  (25)
#define REG_S10 (26)
#define REG_S11 (27)
#define REG_T3  (28)
#define REG_T4  (29)
#define REG_T5  (30)
#define REG_T6  (31)

struct arch_vm_platform {
    union vm_irqc_dscrp {
        struct {
            paddr_t base;
        } plic;
        struct {
            struct {
                paddr_t base;
            } aplic;
            struct {
                paddr_t base;
            } imsic;
        } aia;
    } irqc;
};

struct vm_arch {
#if (IRQC == PLIC)
    struct vplic vplic;
#elif ((IRQC == APLIC) || (IRQC == AIA))
    struct vaplic vaplic;
#else
#error "unknown IRQC type " IRQC
#endif
};

struct vcpu_arch {
    vcpuid_t hart_id;
    struct sbi_hsm sbi_ctx;
};

struct arch_regs {
    union {
        regaddr_t x[31];
        struct {
            regaddr_t ra;
            regaddr_t sp;
            regaddr_t gp;
            regaddr_t tp;
            regaddr_t t0;
            regaddr_t t1;
            regaddr_t t2;
            regaddr_t s0;
            regaddr_t s1;
            regaddr_t a0;
            regaddr_t a1;
            regaddr_t a2;
            regaddr_t a3;
            regaddr_t a4;
            regaddr_t a5;
            regaddr_t a6;
            regaddr_t a7;
            regaddr_t s2;
            regaddr_t s3;
            regaddr_t s4;
            regaddr_t s5;
            regaddr_t s6;
            regaddr_t s7;
            regaddr_t s8;
            regaddr_t s9;
            regaddr_t s10;
            regaddr_t s11;
            regaddr_t t3;
            regaddr_t t4;
            regaddr_t t5;
            regaddr_t t6;
        };
    };

    unsigned long hstatus;
#if __CHERI__
    unsigned long hstatus_pad;
#endif
    unsigned long sstatus;
#if __CHERI__
    unsigned long sstatus_pad;
    void* ddc;
#endif
    regaddr_t sepc;
} __attribute__((__packed__, aligned(sizeof(regaddr_t))));

void vcpu_arch_entry(void);

static inline void vcpu_arch_inject_hw_irq(struct vcpu* vcpu, irqid_t id)
{
    virqc_inject(vcpu, id);
}

static inline void vcpu_arch_inject_irq(struct vcpu* vcpu, irqid_t id)
{
    virqc_inject(vcpu, id);
}

#endif /* __ARCH_VM_H__ */
