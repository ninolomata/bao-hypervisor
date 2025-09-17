/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#ifndef __ARCH_CHERI_H__
#define __ARCH_CHERI_H__

#include <bao.h>

#define CHERI_CLEN __SIZEOF_CHERI_CAPABILITY__

// CHERI capability permissions
// Hardware permissions defined in the CHERI ISA
#define CHERI_HPERMS_GLOBAL               (1ULL << 0)
#define CHERI_HPERMS_PERMIT_EXECUTE       (1ULL << 1)
#define CHERI_HPERMS_PERMIT_LOAD          (1ULL << 2)
#define CHERI_HPERMS_PERMIT_STORE         (1ULL << 3)
#define CHERI_HPERMS_PERMIT_LOAD_CAP      (1ULL << 4)
#define CHERI_HPERMS_PERMIT_STORE_CAP     (1ULL << 5)
#define CHERI_HPERMS_PERMIT_STORE_LOCAL   (1ULL << 6)
#define CHERI_HPERMS_PERMIT_SEAL          (1ULL << 7)
#define CHERI_HPERMS_PERMIT_INVOKE        (1ULL << 8)
#define CHERI_HPERMS_PERMIT_UNSEAL        (1ULL << 9)
#define CHERI_HPERMS_PERMIT_ACC_SYS_REGS  (1ULL << 10)
// Software defined permissions
#define CHERI_SPERMS_0                    (1ULL << 15)
#define CHERI_SPERMS_1                    (1ULL << 16)
#define CHERI_SPERMS_2                    (1ULL << 17)
#define CHERI_SPERMS_3                    (1ULL << 18)

// Hypervisor capability permissions
#define CHERI_HYP_PERMS (CHERI_HPERMS_GLOBAL | \
                         CHERI_HPERMS_PERMIT_LOAD | \
                         CHERI_HPERMS_PERMIT_LOAD_CAP)
// Hypervisor data permissions
#define CHERI_HYP_DATA_PERMS (CHERI_HYP_PERMS | \
                              CHERI_HPERMS_PERMIT_STORE | \
                              CHERI_HPERMS_PERMIT_STORE_CAP | \
                              CHERI_HPERMS_PERMIT_STORE_LOCAL)
// Hypervisor code permissions
#define CHERI_HYP_CODE_PERMS (CHERI_HYP_PERMS | \
                              CHERI_HPERMS_PERMIT_EXECUTE | \
                              CHERI_HPERMS_PERMIT_ACC_SYS_REGS)
// Hypervisor peripherals permissions
#define CHERI_HYP_DEV_PERMS (CHERI_HPERMS_GLOBAL | \
                             CHERI_HPERMS_PERMIT_LOAD | \
                             CHERI_HPERMS_PERMIT_STORE)    


#define CHERI_SCR_PCC             0x0
#define CHERI_SCR_DDC             0x1

#define CHERI_SCR_UTCC            0x4
#define CHERI_SCR_UTDC            0x5
#define CHERI_SCR_USCRATCHC       0x6
#define CHERI_SCR_UEPCC           0x7

#define CHERI_SCR_STCC            0xC
#define CHERI_SCR_STDC            0xD
#define CHERI_SCR_SSCRATCHC       0xE
#define CHERI_SCR_SEPCC           0xF

#define CHERI_SCR_VSTCC           0x14
#define CHERI_SCR_VSTDC           0x15
#define CHERI_SCR_VSSCRATCHC      0x16
#define CHERI_SCR_VSEPCC          0x17

#define CHERI_SCR_MTCC            0x1C 
#define CHERI_SCR_MTDC            0x1D
#define CHERI_SCR_MSCRATCHC       0x1E
#define CHERI_SCR_MEPCC           0x1F

#define CHERI_TVAL_CAUSE_OFF      (0)
#define CHERI_TVAL_CAUSE_LEN      (5)
#define CHERI_TVAL_CAUSE_MSK      BIT_MASK(CHERI_TVAL_CAUSE_OFF, CHERI_TVAL_CAUSE_LEN)

#define CHERI_TVAL_CAP_IDX_OFF    (5)
#define CHERI_TVAL_CAP_IDX_LEN    (6)
#define CHERI_TVAL_CAP_IDX_MSK    BIT_MASK(CHERI_TVAL_CAP_IDX_OFF, CHERI_TVAL_CAP_IDX_LEN)

#define EXCEPTION_CODE_NONE                             (0x0)
#define EXCEPTION_CODE_LENGTH_VIOLATION                 (0x01)
#define EXCEPTION_CODE_TAG_VIOLATION                    (0x02)
#define EXCEPTION_CODE_SEAL_VIOLATION                   (0x03)
#define EXCEPTION_CODE_TYPE_VIOLATION                   (0x04)
#define EXCEPTION_CODE_SW_VIOLATION                     (0x08)
#define EXCEPTION_CODE_UNALIGNED_BASE                   (0x0B)
#define EXCEPTION_CODE_GLOBAL_VIOLATION                 (0x10)
#define EXCEPTION_CODE_PERMIT_EXE_VIOLATION             (0x11)
#define EXCEPTION_CODE_PERMIT_LD_VIOLATION              (0x12)
#define EXCEPTION_CODE_PERMIT_ST_VIOLATION              (0x13)
#define EXCEPTION_CODE_PERMIT_LD_CAP_VIOLATION          (0x14)
#define EXCEPTION_CODE_PERMIT_ST_CAP_VIOLATION          (0x15)
#define EXCEPTION_CODE_PERMIT_ST_LOCAL_VIOLATION        (0x16)
#define EXCEPTION_CODE_PERMITE_ACC_SYS_REGS_VIOLATION   (0x18)
#define EXCEPTION_CODE_PERMITE_INVOKE                   (0x19)

/* If you don’t already have these: */

#define SCRS_GEN_ACCESSORS_NAMED(scr_name, scr_id)                                         \
    static inline void* scr_##scr_name##_read(void)                                        \
    {                                                                                      \
        void * _tmp_cap;                                                                   \
        __asm__ volatile("cspecialr %0, " XSTR(scr_id) : "=C"(_tmp_cap)::"memory");        \
        return _tmp_cap;                                                                   \
    }                                                                                       \
    static inline void scr_##scr_name##_write(void* scr_value)                              \
    {                                                                                       \
        void *_tmp_cap;                                                                     \
        __asm__ volatile (                                                                  \
            "cmove    %0, %1\n\t"        /* tmp = scr_value (force into cap reg) */         \
            "cspecialw " XSTR(scr_id) " , %0\n\t"      /* write SCR(id) from tmp */         \
            : "=&C"(_tmp_cap)                                                               \
            : "C"(scr_value)                                                                \
            : "memory"                                                                      \
        );                                                                                  \
    }
                                                                                         
#define SCRS_GEN_ACCESSORS(scr) SCRS_GEN_ACCESSORS_NAMED(scr, scr)

#ifndef __ASSEMBLER__
    SCRS_GEN_ACCESSORS(ddc)
    SCRS_GEN_ACCESSORS(stcc)
    SCRS_GEN_ACCESSORS(sepcc)
    SCRS_GEN_ACCESSORS(stdc)
    SCRS_GEN_ACCESSORS(sscratchc)
    SCRS_GEN_ACCESSORS_NAMED(vstcc, CHERI_SCR_VSTCC)
    SCRS_GEN_ACCESSORS_NAMED(vsepcc, CHERI_SCR_VSEPCC)
    SCRS_GEN_ACCESSORS_NAMED(vstdc, CHERI_SCR_VSTDC)
    SCRS_GEN_ACCESSORS_NAMED(vsscratchc, CHERI_SCR_VSSCRATCHC)
    static inline void* pcc_read(void)                                      \
    {                                                                       \
        void * _tmp_cap;                                                    \
        __asm__ volatile("cspecialr %0, pcc" : "=C"(_tmp_cap)::"memory");   \
        return _tmp_cap;                                                    \
    }

#endif
#endif