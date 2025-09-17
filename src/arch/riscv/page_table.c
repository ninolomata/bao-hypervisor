/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#include <bao.h>
#include <page_table.h>

#ifdef __CHERI__
#include <arch/cheri_utils.h>
#include <arch/cheri.h>
#endif

#if (RV32)
struct page_table_dscr sv32_pt_dscr = {
    .lvls = 2,
    .lvl_wdt = (size_t[]){ 32, 22 },
    .lvl_off = (size_t[]){ 22, 12 },
    .lvl_term = (bool[]){ true, true },
};
struct page_table_dscr sv32x4_pt_dscr = {
    .lvls = 2,
    .lvl_wdt = (size_t[]){ 34, 22 },
    .lvl_off = (size_t[]){ 22, 12 },
    .lvl_term = (bool[]){ true, true },
};
struct page_table_dscr* hyp_pt_dscr = &sv32_pt_dscr;
struct page_table_dscr* vm_pt_dscr = &sv32x4_pt_dscr;
#elif (RV64)
struct page_table_dscr sv39_pt_dscr = {
    .lvls = 3,
    .lvl_wdt = (size_t[]){ 39, 30, 21 },
    .lvl_off = (size_t[]){ 30, 21, 12 },
    .lvl_term = (bool[]){ true, true, true },
};
struct page_table_dscr sv39x4_pt_dscr = {
    .lvls = 3,
    .lvl_wdt = (size_t[]){ 41, 30, 21 },
    .lvl_off = (size_t[]){ 30, 21, 12 },
    .lvl_term = (bool[]){ true, true, true },
};
struct page_table_dscr* hyp_pt_dscr = &sv39_pt_dscr;
struct page_table_dscr* vm_pt_dscr = &sv39x4_pt_dscr;
#endif

pte_t* pt_get_pte(struct page_table* pt, size_t lvl, vaddr_t va)
{
    size_t pte_index = pt_getpteindex_by_va(pt, va, 0);
    #ifdef __CHERI_PURE_CAPABILITY__
        ptraddr_t pte_pa = (paddr_t ) pt->root;
        pte_t* pte = cheri_build_data_cap(pte_pa, PAGE_SIZE, CHERI_HYP_DEV_PERMS);
        pte = (pte_t*)__builtin_cheri_address_set((void *)pte, (ptraddr_t)&(pt->root[pte_index]));
    #else
        pte_t* pte = &(pt->root[pte_index]);
    #endif

    for (size_t i = 0; i < lvl; i++) {
        if (!pte_valid(pte)) {
            return NULL;
        }
        #ifdef __CHERI_PURE_CAPABILITY__
            pte_pa = pte_addr(pte);
        #else
            pte = (pte_t*)pte_addr(pte);
        #endif
        size_t index = pt_getpteindex_by_va(pt, va, i + 1);
        #ifdef __CHERI_PURE_CAPABILITY__
            pte = cheri_build_data_cap(pte_pa, PAGE_SIZE, CHERI_HYP_DEV_PERMS);
            pte = (pte_t*)__builtin_cheri_address_set((void *)pte, (ptraddr_t)&pte[index]);
        #else
            pte = &pte[index];
        #endif
    }

    return pte;
}

pte_t* pt_get(struct page_table* pt, size_t lvl, vaddr_t va)
{
    uintptr_t pte = (uintptr_t)pt_get_pte(pt, lvl, va);
    return (pte_t*)(pte & ~(pt_size(pt, lvl) - 1));
}

bool pte_page(struct page_table* pt, pte_t* pte, size_t lvl)
{
    UNUSED_ARG(pt);
    UNUSED_ARG(lvl);

    return ((*pte & PTE_VALID) != 0) && ((*pte & PTE_RWX) != 0);
}
