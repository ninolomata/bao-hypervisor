/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#ifndef __ARCH_BAO_H__
#define __ARCH_BAO_H__

#define RV64 (RV_XLEN == 64)
#define RV32 (RV_XLEN == 32)

#ifdef __CHERI_PURE_CAPABILITY__
#define OP(op) c##op
#define RB(rb) c##rb
#define REG_NAME(num) c##num
#define CHERI_FACTOR 2
#define LOAD_ADDR(rd, symbol) cllc c##rd, symbol
#define STORE_REG csc
#define LOAD_REG clc
#else
#define OP(op) op
#define RB(rb) rb
#define REG_NAME(num) x##num
#define LOAD_ADDR(rd, symbol) la rd, symbol
#define CHERI_FACTOR 1
#define STORE_REG sd
#define LOAD_REG ld
#endif

#define MEM_LOAD_OP(op, rd, off, rs) OP(op) rd, off(RB(rs))
#define MEM_STORE_OP(op, rs2, off, rs1) OP(op) rs2, off(RB(rs1))
#define REGLEN_BYTES (RV_XLEN / 8)
#define REGLEN (REGLEN_BYTES * CHERI_FACTOR)



#if (RV64)
#define LOAD(rd, off, rs1) MEM_LOAD_OP(ld, rd, off, rs1)
#define STORE(rs2, off, rs1)  MEM_STORE_OP(sd, rs2, off, rs1)
#elif (RV32)
#define LOAD(rd, off, rs1) MEM_LOAD_OP(lw, rd, off, rs1)
#define STORE(rs2, off, rs1)  MEM_STORE_OP(sw, rs2, off, rs1)
#endif

#if (RV64)
// This layout assumes Sv39 is available as mandated by the RVA23S64 profile
#define BAO_VAS_BASE (0xffffffc000000000)
#define BAO_CPU_BASE (0xffffffc040000000)
#define BAO_VM_BASE  (0xffffffe000000000)
#define BAO_VAS_TOP  (0xfffffff000000000)
#else
// Because sv32 only lowest level only supports 4MiB pages, this should be enough for each section
// otherwise we need allow for N shared PTEs for each section. For now, it seems to suffice. We also
// are assuming, for now that all available physical memory resides in 0x0 - 0xefffffff of virtual
// memory. Otherwise we need to implement a "highmem"-like mechanism.
#define BAO_VAS_BASE (0xc0000000)
#define BAO_CPU_BASE (0xcf400000)
#define BAO_VM_BASE  (0xcf800000)
#define BAO_VAS_TOP  (0xcfc00000)
#endif

#define PAGE_SIZE        (0x1000)
#define STACK_SIZE       (PAGE_SIZE)
#define VM_SHARED_PT_LVL (0)

#ifndef __ASSEMBLER__

#endif /* !__ASSEMBLER__ */

#endif /* __ARCH_BAO_H__ */
