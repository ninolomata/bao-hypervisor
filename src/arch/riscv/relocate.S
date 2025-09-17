/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#include <arch/bao.h>
#include <arch/csrs.h>
#include <arch/page_table.h>
#include <asm_defs.h>

/**
 * Switch to a new address:
 *
 *      a0: virtual address of new cpu space
 *      a1: physical address of new translation tables
 */
.globl switch_space
switch_space:

    /* Get current CPU space stack start */
    li a6, BAO_CPU_BASE
    li t0, CPU_STACK_OFF + CPU_STACK_SIZE
    add a6, a6, t0

    /* Calculate stack size */
    #ifdef __CHERI_PURE_CAPABILITY__
    cgetaddr a7, csp
    #else
    mv a7, sp
    #endif
    sub a5, a6, a7

    /* Get current CPU space current sp */
    #ifdef __CHERI_PURE_CAPABILITY__
        cmove ca6, csp
    #else
        mv a6, sp
    #endif

    /* Get new CPU space current sp */
    li t0, CPU_STACK_OFF + CPU_STACK_SIZE
    #ifdef __CHERI_PURE_CAPABILITY__
        sub t1, t0, a5
        cincoffset ca0, ca0, t1
    #else
        add a0, a0, t0
        sub a0, a0, a5
    #endif

    /* Copy stack */
    li a4, 0
1:
    /* Copy two words */
    #ifdef __CHERI_PURE_CAPABILITY__
    LOAD_REG ca2, (ca6)
    STORE_REG ca2, (ca0)
    #else
    LOAD_REG a2, 0(a6)
    STORE_REG a2, 0(a0)
    #endif
    
    /* Increment addressess and count accordingly */
    #ifdef __CHERI_PURE_CAPABILITY__
    cincoffset ca6, ca6, REGLEN
    cincoffset ca0, ca0, REGLEN
    cincoffset ca4, ca4, REGLEN
    #else
    add a6, a6, REGLEN
    add a0, a0, REGLEN
    add a4, a4, REGLEN
    #endif
    
    /* If count is less then size, repeat */
    blt a4, a5, 1b
    
    /* Restore original addresses */
    #ifdef __CHERI_PURE_CAPABILITY__
        sub t1, a6, a4
        sub t2, a0, a4
        cincoffset ca6, ca6, t1
        cincoffset ca0, ca0, t2
    #else
        sub a6, a6, a4
        sub a0, a0, a4
    #endif

    /* Update root page table pointer */
    srl a1, a1, PAGE_SHIFT
    li  t0, SATP_MODE_DFLT
    or  a1, a1, t0
    csrw satp, a1

    /**
     * Invalidate TLB: we can do this directly here without sbi support because we don't really
     * need any shootdown as all harts must go through here.
     */
    sfence.vma
    #ifdef __CHERI_PURE_CAPABILITY__
    cret
    #else
    ret
    #endif
