/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#include <arch/bao.h>
#include <asm_defs.h>
#include <arch/csrs.h>

.extern hyp_root_data_cap

.text 

.macro VM_EXIT
#ifdef __CHERI_PURE_CAPABILITY__
    cspecialrw c31, sscratchc, c31
#else
    csrrw   x31, sscratch, x31
#endif

    
    STORE_REG   REG_NAME(1), 0*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(2), 1*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(3), 2*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(4), 3*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(5), 4*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(6), 5*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(7), 6*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(8), 7*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(9), 8*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(10), 9*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(11), 10*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(12), 11*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(13), 12*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(14), 13*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(15), 14*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(16), 15*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(17), 16*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(18), 17*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(19), 18*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(20), 19*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(21), 20*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(22), 21*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(23), 22*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(24), 23*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(25), 24*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(26), 25*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(27), 26*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(28), 27*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(29), 28*REGLEN(REG_NAME(31))
    STORE_REG   REG_NAME(30), 29*REGLEN(REG_NAME(31))
    
    #ifdef __CHERI_PURE_CAPABILITY__
        cmove csp, c31
        cspecialrw c31, sscratchc, c31
        STORE_REG   REG_NAME(31), 30*REGLEN(RB(sp))
    #else
        mv      sp, x31
        csrrw   x31, sscratch, x31
        STORE(x31, 30*REGLEN, sp)
    #endif

    csrr    t0, CSR_HSTATUS
    STORE(t0, 31*REGLEN, sp)
    csrr    t0, sstatus
    STORE(t0, 32*REGLEN, sp)
    #ifdef __CHERI__
    cspecialr ct0, ddc
    STORE_REG RB(t0), 33*REGLEN(RB(sp))
    #endif
    #ifdef __CHERI_PURE_CAPABILITY__
        cspecialr ct0, sepcc
    #else
        csrr    t0, sepc
    #endif
    STORE_REG RB(t0), 34*REGLEN(RB(sp))

    #ifdef __CHERI_PURE_CAPABILITY__
        clgc        ct1, hyp_root_data_cap
        clc         ct1, 0(ct1)
        li          sp, BAO_CPU_BASE
        li          t0, CPU_STACK_OFF + CPU_STACK_SIZE
        cfromptr    csp, ct1, sp
        csetbounds  csp, csp, t0
        cincoffset  csp, csp, t0
        cmove cgp, cnull
    #else
        li      sp, BAO_CPU_BASE
        li      t0, CPU_STACK_OFF + CPU_STACK_SIZE
        add     sp, sp, t0
        .option push
        .option norelax
        la  gp, __global_pointer$
        .option pop  
    #endif

.endm



.macro VM_ENTRY
#ifdef __CHERI_PURE_CAPABILITY__
    cspecialr c31, sscratchc
#else
    csrr   x31, sscratch
#endif

    LOAD(t0, 31*REGLEN, t6)
    csrw   CSR_HSTATUS, t0
    LOAD(t0, 32*REGLEN, t6)
    csrw   sstatus, t0
#ifdef __CHERI__
    LOAD_REG   REG_NAME(1), 33*REGLEN(REG_NAME(31))
    cspecialw ddc, c1
#endif
    LOAD_REG   REG_NAME(1), 34*REGLEN(REG_NAME(31))
#ifdef __CHERI_PURE_CAPABILITY__
    cspecialw sepcc, c1
#else
    csrw   sepc, x1
#endif

    LOAD_REG   REG_NAME(1), 0*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(2), 1*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(3), 2*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(4), 3*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(5), 4*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(6), 5*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(7), 6*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(8), 7*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(9), 8*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(10), 9*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(11), 10*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(12), 11*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(13), 12*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(14), 13*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(15), 14*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(16), 15*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(17), 16*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(18), 17*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(19), 18*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(20), 19*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(21), 20*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(22), 21*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(23), 22*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(24), 23*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(25), 24*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(26), 25*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(27), 26*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(28), 27*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(29), 28*REGLEN(REG_NAME(31))
    LOAD_REG   REG_NAME(30), 29*REGLEN(REG_NAME(31)) 
    LOAD_REG   REG_NAME(31), 30*REGLEN(REG_NAME(31))  

    sret
    j   .
.endm

.balign 0x4
.global _hyp_trap_vector	
_hyp_trap_vector:
    VM_EXIT
    csrr    t0, scause
    bltz    t0, 1f
    #ifdef __CHERI_PURE_CAPABILITY__
        cllc   cra, sync_exception_handler
        cjalr  cra
    #else
        call    sync_exception_handler
    #endif
    j       2f
1:
    #ifdef __CHERI_PURE_CAPABILITY__
        cllc   cra, interrupts_arch_handle
        cjalr  cra
    #else
        call    interrupts_arch_handle
    #endif
2:
.global vcpu_arch_entry
vcpu_arch_entry:
    VM_ENTRY
