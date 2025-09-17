/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#include <arch/bao.h>
#include <arch/csrs.h>
#include <arch/page_table.h>
#include <asm_defs.h>
#include <platform_defs.h>
#ifdef __CHERI__
#include <arch/cheri.h>
#endif

#define PT_SIZE PAGE_SIZE
#if RV64
#define PT_LVLS 3
#define PTE_INDEX_SHIFT(LEVEL) ((9 * (PT_LVLS - 1 - (LEVEL))) + 12)
#else
#define PT_LVLS 2
#define PTE_INDEX_SHIFT(LEVEL) ((10 * (PT_LVLS - 1 - (LEVEL))) + 12)
#endif 

.macro DEFINE_SYMVAL name, target
    .globl \name
    .type  \name, @object
    .size  \name, 8
\name:
    .dword \target
.endm

/* Load XLEN-sized load, with CHERI capabilities or not  */
.macro LOAD rd, off, rs1
#ifdef __CHERI_PURE_CAPABILITY__
  #if __riscv_xlen == 64
    cld \rd, \off(c\rs1)
  #else
    clw \rd, \off(c\rs1)
  #endif
#else
  #if __riscv_xlen == 64
    ld  \rd, \off(\rs1)
  #else
    lw  \rd, \off(\rs1)
  #endif
#endif
.endm

/* Load XLEN-sized store, with CHERI capabilities or not  */

.macro STORE rs2, off, rs1
#ifdef __CHERI_PURE_CAPABILITY__
  #if __riscv_xlen == 64
    csd \rs2, \off(c\rs1)
  #else
    csw \rs2, \off(c\rs1)
  #endif
#else
  #if __riscv_xlen == 64
    sd  \rs2, \off(\rs1)
  #else
    sw  \rs2, \off(\rs1)
  #endif
#endif
.endm

/* Load address */
.macro LOAD_ADDR rd, sym
#ifdef __CHERI_PURE_CAPABILITY__
    /* Early boot we need to use the PCC capability*/
    cllc c\rd, \sym
#else
    la   \rd, \sym
#endif
.endm

/**
 * Calculates the index or offset of a page table entry for given virtual address(addr) at a given
 * level of page table.
 */
.macro PTE_INDEX_ASM	index, addr, level
	srl \index, \addr, PTE_INDEX_SHIFT(\level) 
    li  s0, ((PAGE_SIZE/REGLEN_BYTES)-1)
	and \index, \index, s0
    li  s0, REGLEN_BYTES
	mul \index, \index, s0
.endm

/**
 * Calculates the pointer to a pte given the page table pointer(pt), the page table level (levle)
 * and the target virtual address (va)
 */
.macro PTE_PTR  pte, pt, level, va
    PTE_INDEX_ASM	s1, \va, \level
#ifdef __CHERI_PURE_CAPABILITY__
    cincoffset RB(\pte),  RB(\pt), s1
#else
    add     \pte, s1, \pt
#endif
.endm

/**
 * Creates a page table entry (pte) for a given physical address (pa) and set of flags.
 */
.macro PTE_FILL    pte, pa, flags
    srl     \pte, \pa, 2
    or      \pte, \pte, \flags
.endm

.macro LD_SYM      rd, sym, rs
    LOAD_ADDR \rs, \sym
    LOAD \rd, 0, \rs
.endm

.section ".rodata", "a"
/* Symbol values */
DEFINE_SYMVAL _ipi_cpumsg_handlers_sym, _ipi_cpumsg_handlers
DEFINE_SYMVAL _ipi_cpumsg_handlers_id_start_sym, _ipi_cpumsg_handlers_id_start
DEFINE_SYMVAL _ipi_cpumsg_handlers_id_end_sym, _ipi_cpumsg_handlers_id_end
DEFINE_SYMVAL _vm_image_start_sym, _vm_image_start
DEFINE_SYMVAL _vm_image_end_sym, _vm_image_end
DEFINE_SYMVAL _image_end_sym, _image_end
DEFINE_SYMVAL _image_load_end_sym, _image_load_end
DEFINE_SYMVAL _image_start_sym, _image_start
_image_noload_start_sym: .8byte _image_noload_start
_dmem_beg_sym: .8byte _dmem_beg
_enter_vas_sym: .8byte _enter_vas
_bss_start_sym: .8byte _bss_start
_bss_end_sym: .8byte _bss_end
_extra_allocated_phys_mem_sym: .8byte extra_allocated_phys_mem

.data
.align 3
/**
 * barrier is used to minimal synchronization in boot - other cores wait for bsp to set it.
 */
_barrier: .8byte 0

/**
 * 	The following code MUST be at the base of the image, as this is bao's entrypoint. Therefore
 * .boot section must also be the first in the linker script. DO NOT implement any code before the
 * _reset_handler in this section.
 */
 .section ".boot", "ax"
.globl _reset_handler

_reset_handler:
    /**
     * Force enter in cap mode if not already in it.
     * This is needed to ensure that the boot code runs in cap mode.
     */
    #ifdef __CHERI_PURE_CAPABILITY__
    .option push
    .option nocapmode
        cspecialr ct0, pcc
        li t1, 1
        csetflags ct0, ct0, t1
        lla t1, 1f
        csetaddr ct0, ct0, t1
    #ifdef __riscv_xcheri_mode_dependent_jumps
        jr.cap ct0
    #else
        cjr ct0
    #endif
    1:
    .option pop
    #endif
    /**
    * The previous boot stage should pass the following arguments:
    *      a0 -> hart id
    *      a1 -> config binary load addr
    * The following registers are reserved to be passed to init function as arguments:
    *      a0 -> hart id
    *      a1 -> contains image base load address
    *      a2 -> config binary load address (originally passed in a1)
    *
    * The remaining code must use t0-t6 as scratchpad registers in the main flow and s0-s5 in
    * auxiliary routines. s6-s11 are used to hold constants a3-a7 are used as arguments and return
    * values (can be also corrupted in auxiliary routines).
    */
    mv      a2, a1
    LOAD_ADDR a1,_image_start
    LD_SYM  s6, _extra_allocated_phys_mem_sym, s6

    /**
     * Setup stvec early. In case of we cause an exception in this boot code we end up at a known
     * place.
     */
    LOAD_ADDR t0, _hyp_trap_vector
    #ifdef __CHERI_PURE_CAPABILITY__
    cgetaddr t1, ct0
    and t1, t1, ~STVEC_MODE_MSK
    or t1, t1, STVEC_MODE_DIRECT
    csetaddr ct0, ct0, t1
    cspecialw stcc, ct0
    #else
    la      t0, _hyp_trap_vector
    and     t0, t0, ~STVEC_MODE_MSK
    or      t0, t0, STVEC_MODE_DIRECT
    csrw    stvec, t0
    #endif

    /**
     * Bring processor to known supervisor state: make sure interrupts and memory translation are
     * disabled.
     */

    csrw   sstatus, zero
    csrw   sie, zero
    csrw   sip, zero
    csrw   satp, zero 


#if defined(CPU_MASTER_FIXED)
/**
 * All cpus set the CPU_MASTER with the same value to avoid synchronization.    
 */
li      t1, CPU_MASTER_FIXED
LOAD_ADDR t0, CPU_MASTER
STORE(t1, 0, t0)
#else
/**
 * The first hart to grab the lock is CPU_MASTER.
 */
.pushsection .data
_boot_lock:
    .4byte 0
.popsection
LOAD_ADDR t0, _boot_lock
li      t1, 1
1:
#ifdef __CHERI_PURE_CAPABILITY__
    clr.w    t2, (ct0)
#else
    lr.w    t2, (t0)
#endif
    bnez    t2, 2f
#ifdef __CHERI_PURE_CAPABILITY__
    csc.w    t2, t1, (ct0)
#else
    sc.w    t2, t1, (t0)
#endif   
    bnez    t2, 1b
LOAD_ADDR t0, CPU_MASTER
STORE a0, 0, t0
2:
#endif

    /* Setup bootstrap page tables. Assuming sv39 or sv32 support. */ 

 	/* Skip initialy global page tables setup if not hart */
    LD_SYM  t0, CPU_MASTER, t0
	bne     a0, t0, wait_for_bsp
    LOAD_ADDR a3, _page_tables_start
    LOAD_ADDR a4, _page_tables_end

    #ifdef __CHERI_PURE_CAPABILITY__
        cincoffset  ca3, ca3, s6
        cincoffset  ca4, ca4, s6
        cllc        cra, clear
        cjalr       cra
    #else
        add     a3, a3, s6
        add     a4, a4, s6
        call	clear
    #endif	 

    LOAD_ADDR t0, root_l1_pt
    #ifdef __CHERI_PURE_CAPABILITY__
        cincoffset  ct0, ct0, s6
    #else
        add         t0, t0, s6
    #endif
#if RV64
    LOAD_ADDR t1, root_l2_pt
    add         t1, t1, s6
    PTE_FILL    t1, t1, PTE_TABLE
    li          t2, BAO_VAS_BASE
    PTE_PTR     t2, t0, 1, t2
    STORE t1, 0, t2
    LOAD_ADDR t0, root_l2_pt
    #ifdef __CHERI_PURE_CAPABILITY__
        cincoffset  ct0, ct0, s6
    #else
        add         t0, t0, s6
    #endif
#endif
    LD_SYM      t1, _image_start_sym, t1
    PTE_PTR     t1, t0, (PT_LVLS - 1), t1
    LD_SYM      t2, _image_load_end_sym, t2
    PTE_PTR     t2, t0, (PT_LVLS - 1), t2
    LOAD_ADDR   t0, _image_start
    li t3, PTE_HYP_FLAGS | PTE_PAGE
    PTE_FILL    t0, t0, t3
1:
    bge     t1, t2, 2f
    STORE   t0, 0, t1
    #ifdef __CHERI_PURE_CAPABILITY__
        cincoffset ct1, ct1, REGLEN_BYTES
    #else
        add     t1, t1, REGLEN_BYTES
    #endif
    add     t0, t0, (PAGE_SIZE >> 2)
    j       1b
2:
#if RV64
    LOAD_ADDR t0, root_l2_pt
#else
    LOAD_ADDR t0, root_l1_pt
#endif
    #ifdef __CHERI_PURE_CAPABILITY__
        cincoffset  ct0, ct0, s6
    #else
        add         t0, t0, s6
    #endif
    LD_SYM      t2, _image_end_sym, t2
    PTE_PTR     t2, t0, (PT_LVLS - 1 ), t2
    bge         t1, t2, 3f
    LOAD_ADDR   t0, _image_noload_start
    li t3, PTE_HYP_FLAGS | PTE_PAGE
    PTE_FILL    t0, t0, t3
    j 1b
3:
    fence   w, w
    LOAD_ADDR t0, _barrier
    li      t1, 1
    STORE t1, 0, t0
    j       map_cpu

wait_for_bsp:
    LOAD_ADDR t0, _barrier
    li      t1, 1
1:
    LOAD t2, 0, t0
    blt     t2, t1, 1b

map_cpu:
    /* Calculate base phys address of CPU struct -> t0 */
    LOAD_ADDR t0, _dmem_phys_beg
    li      t1, (CPU_SIZE + (PT_SIZE*PT_LVLS))
    mul     t2, t1, a0
    #ifdef __CHERI_PURE_CAPABILITY__
        cincoffset  ct0, ct0, t2
        cmove       ca3, ct0
        cincoffset  ca4, ca3, t2
        cllc        cra, clear
        cjalr       cra
    #else
        add     t0, t0, t2
        mv      a3, t0
        add     a4, a3, t1
        call    clear
    #endif

    /* Calculate phys address page table -> t1 */
    li      t2, CPU_SIZE
    #ifdef __CHERI_PURE_CAPABILITY__
        cincoffset ct1, ct0, t2
    #else
        add     t1, t0, t2
    #endif

    /* Add root l1 page table pointer to root page table */
    LOAD_ADDR t2, root_l1_pt
    add         t2, t2, s6
    PTE_FILL    t2, t2, PTE_TABLE
    li          t3, BAO_VAS_BASE
    PTE_PTR     t3, t1, 0, t3
    STORE       t2, 0, t3

    li      t4, PAGE_SIZE

    #ifdef __CHERI_PURE_CAPABILITY__
    cincoffset  ct2, ct1, t4
    #else
    add         t2, t1, t4
    #endif
    PTE_FILL    t2, t2, PTE_TABLE
    li          t3, BAO_CPU_BASE
    PTE_PTR     t3, t1, 0, t3
    STORE       t2, 0, t3

#if RV64
    #ifdef __CHERI_PURE_CAPABILITY__
    cincoffset  ct1, ct1, t4
    cincoffset  ct2, ct1, t4
    #else
    add         t1, t1, t4
    add         t2, t1, t4
    #endif
    PTE_FILL    t2, t2, PTE_TABLE
    li          t3, BAO_CPU_BASE
    PTE_PTR     t3, t1, 1, t3
    STORE       t2, 0, t3
#endif
    #ifdef __CHERI_PURE_CAPABILITY__
        cincoffset  ct1, ct1, t4
    #else
        add         t1, t1, t4
    #endif
    li          t2, BAO_CPU_BASE
    PTE_PTR     t1, t1, 2, t2
    li t3, PTE_HYP_FLAGS | PTE_PAGE
    PTE_FILL    t2, t0, t3
    li          t3, CPU_SIZE+PT_SIZE
1:
    blez    t3, setup_cpu
    STORE   t2, 0, t1
    #ifdef __CHERI_PURE_CAPABILITY__
        cincoffset  ct1, ct1, REGLEN_BYTES
    #else
        add     t1, t1, REGLEN_BYTES
    #endif
    add     t2, t2, (PAGE_SIZE >> 2)
    sub     t3, t3, t4
    j       1b
    
setup_cpu:
    LOAD_ADDR   t0, _dmem_phys_beg
    li      t1, (CPU_SIZE + (PT_SIZE*PT_LVLS))
    mul     t1, t1, a0
    #ifdef __CHERI_PURE_CAPABILITY__
        cincoffset ct0, ct0, t1
    #else
        add     t0, t0, t1   
    #endif
    li      t1, CPU_SIZE
    #ifdef __CHERI_PURE_CAPABILITY__
        cincoffset ct0, ct0, t1
    #else
        add     t0, t0, t1
    #endif
    srl     t0, t0, PAGE_SHIFT
    li      t2, SATP_MODE_DFLT
    or      t0, t0, t2

    LD_SYM  t2, _enter_vas_sym, t2
    #ifdef __CHERI__
        auipcc ct3,0
        csetaddr ct3, ct3, t2
        cmove ct2, ct3
        and t4, t2, ~STVEC_MODE_MSK
        or t4, t4, STVEC_MODE_DIRECT
        csetaddr ct4, ct3, t4
        cspecialw stcc, ct4
    #else
        and     t3, t2, ~STVEC_MODE_MSK
        or      t3, t3, STVEC_MODE_DIRECT
        csrw    stvec, t3
    #endif

    sfence.vma
    csrw   satp, t0
    #ifdef __CHERI_PURE_CAPABILITY__
        cjr     ct2
    #else
        jr      t2
    #endif

_enter_vas:
	/* Remove temporary mapping - the L1 page holding it leaks */
	/* TODO: clear TLB entries for this mapping */
    /* now set stvec in virtual address space */
    #ifdef __CHERI_PURE_CAPABILITY__
        cllc ct0, _hyp_trap_vector
        and t1, t0, ~STVEC_MODE_MSK
        or t1, t1, STVEC_MODE_DIRECT
        csetaddr ct0, ct0, t1
        cspecialw stcc, ct0
    #else
        la      t0, _hyp_trap_vector
        and     t0, t0, ~STVEC_MODE_MSK
        or      t0, t0, STVEC_MODE_DIRECT
        csrw   stvec, t0
    #endif

    /* Init stack pointer and global pointer */
    #ifdef __CHERI_PURE_CAPABILITY__
        li          t0, BAO_CPU_BASE
        li          t1, CPU_STACK_OFF + CPU_STACK_SIZE
        cspecialr   ct2, ddc
        cfromptr    csp, ct2, t0
        csetbounds  csp, csp, t1
        cincoffset  csp, csp, t1    
    #else
        li      t0, BAO_CPU_BASE
        mv      tp, t0
        li      sp, CPU_STACK_OFF + CPU_STACK_SIZE
        add     sp, t0, sp
    #endif

    #ifdef __CHERI_PURE_CAPABILITY__
        cmove cgp, cnull
    #else
        .option push
        .option norelax
        la  gp, __global_pointer$
        .option pop
    #endif

    /* clear bss if hart 0 */
    LD_SYM  t0, CPU_MASTER, t0
    bne     a0, t0, wait_for_bsp_2
    LOAD_ADDR   a3, _bss_start_sym
    LOAD        a3, 0, a3
    LOAD_ADDR   a4, _bss_end_sym
    LOAD        a4, 0, a4
    #ifdef __CHERI_PURE_CAPABILITY__
        auipcc ct1, 0
        sub t2, a4, a3
        csetaddr ct1, ct1, a3
        csetbounds ct1, ct1, t2
        cmove ca3, ct1
        cllc cra, clear
        cjalr cra
    #else
        call    clear
    #endif

    /* Init Capabilities */
    #ifdef __CHERI__
    #if __CHERI_PURE_CAPABILITY__
    // Initialize and clear the DDC capability
        cspecialr cs0, ddc
        cmove ct0, cnull
        cspecialw ddc, ct0
    #else
        cspecialr cs0, ddc
    #endif
    #endif
    #ifdef __CHERI__
    // Initialize the CAP relocs
    cllc cra, cheri_init_caps_reloc
    cmove cs1, ca0
    cmove cs2, ca1
    cmove ca0, cs0
    li t0, CHERI_HYP_DATA_PERMS
    candperm ca0, ca0, t0
    cspecialr ca1, pcc
    li t0, CHERI_HYP_CODE_PERMS
    candperm ca1, ca1, t0
    cjalr cra
    cmove ca0, cs1
    cmove ca1, cs2
    #endif

    // Reduce the PCC Permissions to the hypervisor code permissions
    // Consider also shrinking the PCC bounds
    #ifdef __CHERI__
        cllc ct0, 1f
        li t1, CHERI_HYP_CODE_PERMS
        candperm ct0, ct0, t1
        cjr ct0
    1:
    #endif

    LD_SYM  t0, CPU_MASTER, t0
    bne     a0, t0, wait_for_bsp_2

    fence   w, w
    LOAD_ADDR   t0, _barrier
    li  t1, 2
    #ifdef __CHERI__
        cgetaddr t2, ct0
        cmove ct3, cs0
        csetaddr ct0, ct3, t2
    #endif
    STORE(t1, 0, t0)

wait_for_bsp_2:
    /* wait for hart 0 to finish clearing bss */
    LOAD_ADDR   t0, _barrier
    li  t1, 2
1:
    LOAD        t2, 0, t0
    blt t2, t1, 1b

    j   init

	/* This point should never be reached */
	j	.	

clear:
#if __CHERI_PURE_CAPABILITY__
    csb  zero, 0(ca3)
    cincoffset ca3, ca3, 1
#else
    sb  zero, 0(a3)
    add a3, a3, 1
#endif
    blt a3, a4, clear
#if __CHERI_PURE_CAPABILITY__
    cret
#else
    ret
#endif
