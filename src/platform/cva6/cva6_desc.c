#include <platform.h>
#include <interrupts.h>

struct platform platform = {

    .cpu_num = 1,

    .region_num = 1,
    .regions =  (struct mem_region[]) {
        {
            .base = 0x80200000,
            .size = 0x40000000 - 0x200000
        }
    },

    .console = {
        .base = 0x10000000,
    },

    .arch = {
        #if (IRQC == PLIC)
        .irqc.plic.base = 0xc000000,        
        #elif (IRQC == APLIC)
        .irqc.aia.aplic.base = 0xd000000,
        #else 
        #error "unknown IRQC type " IRQC
        #endif
    }
};
