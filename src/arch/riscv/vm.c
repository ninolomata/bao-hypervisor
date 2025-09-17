/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#include <vm.h>
#include <page_table.h>
#include <arch/csrs.h>
#include <irqc.h>
#include <arch/instructions.h>
#include <string.h>
#include <config.h>

#ifdef __CHERI__
#include <arch/cheri.h>
#include <arch/cheri_utils.h>
#endif

void vm_arch_init(struct vm* vm, const struct vm_config* vm_config)
{
    paddr_t root_pt_pa;
    mem_translate(&cpu()->as, (vaddr_t)vm->as.pt.root, &root_pt_pa);

    unsigned long hgatp = (root_pt_pa >> PAGE_SHIFT) | (HGATP_MODE_DFLT) |
        ((vm->id << HGATP_VMID_OFF) & HGATP_VMID_MSK);

    csrs_hgatp_write(hgatp);

    virqc_init(vm, &vm_config->platform.arch.irqc);
}

void vcpu_arch_init(struct vcpu* vcpu, struct vm* vm)
{
    UNUSED_ARG(vm);

    vcpu->arch.sbi_ctx.lock = SPINLOCK_INITVAL;
    vcpu->arch.sbi_ctx.state = vcpu->id == 0 ? STARTED : STOPPED;
}

void vcpu_arch_reset(struct vcpu* vcpu, vaddr_t entry)
{
    memset(&vcpu->regs, 0, sizeof(struct arch_regs));
    #ifdef __CHERI_PURE_CAPABILITY__
        scr_sscratchc_write((void*)&vcpu->regs);
    #else
        csrs_sscratch_write((uintptr_t)&vcpu->regs);
    #endif

    vcpu->regs.hstatus = HSTATUS_SPV | (1ULL << HSTATUS_VGEIN_OFF);
    vcpu->regs.sstatus = SSTATUS_SPP_BIT | SSTATUS_FS_DIRTY | SSTATUS_XS_DIRTY;
    #ifdef __CHERI_PURE_CAPABILITY__
        vcpu->regs.sepc =__builtin_cheri_address_set(hyp_root_code_cap, entry);
        vcpu->regs.ddc =__builtin_cheri_address_set(hyp_root_data_cap, entry);
        vcpu->arch.hart_id = vcpu->id;
        vcpu->regs.a0 = __builtin_cheri_address_set(vcpu->regs.a0, vcpu->id);
    #else
        vcpu->regs.a0 = vcpu->arch.hart_id = vcpu->id;
    #endif
    vcpu->regs.a1 = 0; // according to sbi it should be the dtb load address

    /* if (CPU_HAS_EXTENSION(CPU_EXT_SSTC)) {
        csrs_stimecmp_write(~0U);
        csrs_henvcfg_set(HENVCFG_STCE);
    } else {
        csrs_henvcfg_clear(HENVCFG_STCE);
    } */

    csrs_hcounteren_write(HCOUNTEREN_TM);
    csrs_htimedelta_write(0);
    csrs_vsstatus_write(SSTATUS_SD | SSTATUS_FS_DIRTY | SSTATUS_XS_DIRTY);
    csrs_hie_write(0);
    csrs_vscause_write(0);
    csrs_vstval_write(0);
    csrs_hvip_write(0);
    csrs_vsatp_write(0);
    #ifdef __CHERI_PURE_CAPABILITY__
        /* scr_vstcc_write(hyp_null_cap);
        scr_vsepcc_write(hyp_null_cap);
        scr_vstdc_write(hyp_null_cap);
        scr_vsscratchc_write(hyp_null_cap); */
    #else
        csrs_vstvec_write(0);
        csrs_vsepc_write(0);
        csrs_vsscratch_write(0);
    #endif
}

void* vcpu_readreg(struct vcpu* vcpu, unsigned long reg)
{
    if ((reg <= 0) || (reg > 31)) {
        return 0;
    }
    return vcpu->regs.x[reg - 1];
}

unsigned long vcpu_readreg_raw(struct vcpu* vcpu, unsigned long reg)
{
    #ifdef __CHERI_PURE_CAPABILITY__
        return __builtin_cheri_address_get(vcpu_readreg(vcpu, reg));
    #else
        return (unsigned long) vcpu_readreg(vcpu, reg);
    #endif
}

void vcpu_writereg(struct vcpu* vcpu, unsigned long reg, void* val)
{
    if ((reg <= 0) || (reg > 31)) {
        return;
    }
    vcpu->regs.x[reg - 1] = val;
}

void vcpu_writereg_raw(struct vcpu* vcpu, unsigned long reg, unsigned long val)
{
#ifdef __CHERI_PURE_CAPABILITY__
    void* wr_reg = NULL;
    wr_reg = __builtin_cheri_address_set(wr_reg, val);
    vcpu_writereg(vcpu, reg, wr_reg);
#else
    vcpu_writereg(vcpu, reg, val);
#endif
}

#ifdef __CHERI_PURE_CAPABILITY__
void* vcpu_readpc(struct vcpu* vcpu)
#else
unsigned long vcpu_readpc(struct vcpu* vcpu)
#endif
{
    return vcpu->regs.sepc;
}

#ifdef __CHERI_PURE_CAPABILITY__
void vcpu_writepc(struct vcpu* vcpu, void* pc)
{
    vcpu->regs.sepc = (void *) pc;
}
#else
void vcpu_writepc(struct vcpu* vcpu, unsigned long pc)
{
    vcpu->regs.sepc = pc;
}
#endif

bool vcpu_arch_is_on(struct vcpu* vcpu)
{
    return vcpu->arch.sbi_ctx.state == STARTED;
}
