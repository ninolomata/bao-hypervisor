#ifndef __CHERI_UTILS_H
#define __CHERI_UTILS_H
#ifdef __CHERI__
#include <stddef.h>
extern void * hyp_root_data_cap;
extern void * hyp_root_code_cap;
extern void * hyp_null_cap;

#define INVALID_CAP(addr)                                        \
({                                                               \
    (void*) (__builtin_cheri_address_set(NULL,(uintptr_t) addr));\
})

void * cheri_seal_cap( void * unsealed_cap,
                        size_t otype );
void * cheri_unseal_cap( void * unsealed_cap );
void * cheri_build_data_cap( ptraddr_t address,
                                size_t size,
                                size_t perms );
void * cheri_build_code_cap( ptraddr_t address,
                                size_t size,
                                size_t perms );
void * cheri_build_code_cap_unbounded( ptraddr_t address,
                                        size_t perms );
void * cheri_derive_data_cap( void * src,
                                ptraddr_t address,
                                size_t size,
                                size_t perms );
void * cheri_derive_code_cap( void * src,
                                ptraddr_t address,
                                size_t size,
                                size_t perms );
void cheri_print_cap( void * cap );
void cheri_print_scrs( void );
const char * cheri_trap_tval_to_str(unsigned int tval);
void* cheri_build_integer(unsigned int val);

#endif /* __CHERI__ */
#endif /* ifndef __CHERI_UTILS_H */
