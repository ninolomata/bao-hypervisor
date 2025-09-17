#ifdef __CHERI__

#include <arch/cheri_utils.h>
#include <arch/cheri.h>
#include <stdint.h>

const char* cheri_tval_exceptions[] = {
            "None",                                     // 0
            "Length Violation",                         // 1
            "Untagged Capability",                      // 2
            "Sealed",                                   // 3
            "Type",                                     // 4
            "Reserved",                                 // 5
            "Reserved",                                 // 6
            "Reserved",                                 // 7
            "SW Permission",                            // 8
            "Reserved",                                 // 9
            "Representability",                         // 0xa
            "Unaligned Base",                           // 0xb
            "Reserved",                                 // 0xc
            "Reserved",                                 // 0xd
            "Reserved",                                 // 0xe
            "Reserved",                                 // 0xf
            "Global",                                   // 0x10
            "Execute Permission",                       // 0x11
            "Load Permission",                          // 0x12
            "Store Permission",                         // 0x13
            "Load Capability Permission",               // 0x14
            "Store Capability Permission",              // 0x15
            "Store Local Capability Permission",        // 0x16
            "Seal Permission",                          // 0x17
            "Access System Registers Permission",       // 0x18
            "CInvoke Permission",                       // 0x19
            "Access CInvoke IDC",                       // 0x1a
            "Unseal Permission",                        // 0x1b
            "Set CID Permission",                       // 0x1c
            "Reserved",                                 // 0x1d
            "Reserved",                                 // 0x1e
            "Reserved"                                  // 0x1f
        };

inline void * cheri_seal_cap( void * unsealed_cap,
                              size_t otype )
{
    void * sealer = __builtin_cheri_address_set( hyp_root_data_cap, otype );

    return __builtin_cheri_seal( unsealed_cap, sealer );
}

inline void * cheri_unseal_cap( void * sealed_cap )
{
    size_t otype = __builtin_cheri_type_get( sealed_cap );
    void * unsealer = __builtin_cheri_address_set( hyp_root_code_cap, otype );

    return __builtin_cheri_unseal( sealed_cap, unsealer );
}

inline void * cheri_build_data_cap( ptraddr_t address,
                                    size_t size,
                                    size_t perms )
{
    void * returned_cap = hyp_root_data_cap;

    returned_cap = __builtin_cheri_perms_and( returned_cap, perms );
    returned_cap = __builtin_cheri_offset_set( returned_cap, address );
    returned_cap = __builtin_cheri_bounds_set( returned_cap, size );
    return returned_cap;
}

inline void * cheri_build_code_cap( ptraddr_t address,
                                    size_t size,
                                    size_t perms )
{
    void * returned_cap = hyp_root_code_cap;

    returned_cap = __builtin_cheri_perms_and( returned_cap, perms );
    returned_cap = __builtin_cheri_address_set( returned_cap, address );
    returned_cap = __builtin_cheri_bounds_set( returned_cap, size );

    return returned_cap;
}

inline void * cheri_build_code_cap_unbounded( ptraddr_t address,
                                              size_t perms )
{
    void * returned_cap = hyp_root_code_cap;

    returned_cap = __builtin_cheri_perms_and( returned_cap, perms );
    returned_cap = __builtin_cheri_offset_set( returned_cap, address );

    return returned_cap;
}

inline void * cheri_derive_data_cap( void * src,
                                     ptraddr_t address,
                                     size_t size,
                                     size_t perms )
{
    void * returned_cap = src;

    returned_cap = __builtin_cheri_perms_and( returned_cap, perms );
    returned_cap = __builtin_cheri_address_set( returned_cap, address );
    returned_cap = __builtin_cheri_bounds_set( returned_cap, size );
    return returned_cap;
}

inline void * cheri_derive_code_cap( void * src,
                                     ptraddr_t address,
                                     size_t size,
                                     size_t perms )
{
    void * returned_cap = src;

    returned_cap = __builtin_cheri_perms_and( returned_cap, perms );
    returned_cap = __builtin_cheri_address_set( returned_cap, address );
    returned_cap = __builtin_cheri_bounds_set( returned_cap, size );

    return returned_cap;
}

void cheri_print_cap( void * cap )
{
    console_printk(    "{\r\n"          \
              "\tv: %d  \r\n"         \
              "\tf: %d \r\n"          \
              "\tsealed: %d\r\n"      \
              "\taddr: 0x%lx \r\n"    \
              "\tbase: 0x%lx \r\n"    \
              "\tlength: 0x%lx\r\n"   \
              "\toffset: 0x%lx \r\n"  \
              "\tperms: 0x%lx\r\n"    \
              "\totype: 0x%lx \r\n"   \
              "} \r\n",
            ( uint8_t ) __builtin_cheri_tag_get( cap ),
            ( uint8_t ) __builtin_cheri_flags_get( cap ),
            ( uint8_t ) __builtin_cheri_sealed_get( cap ),
            __builtin_cheri_address_get( cap ),
            __builtin_cheri_base_get( cap ),
            __builtin_cheri_length_get( cap ),
            __builtin_cheri_offset_get( cap ),
            __builtin_cheri_perms_get( cap ),
            __builtin_cheri_type_get( cap )
            );
}

const char * cheri_trap_tval_to_str(unsigned int tval){
    return cheri_tval_exceptions[tval & 0x1F];
}

#endif /* __CHERI__ */