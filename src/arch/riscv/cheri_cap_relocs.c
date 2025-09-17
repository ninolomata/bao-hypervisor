#ifdef __CHERI__
#include <cheri_init_globals.h>
#include <arch/cheri_utils.h>
#include <stdint.h>

void * hyp_root_data_cap = (void *)(intcap_t)0;
void * hyp_root_code_cap = (void *)(intcap_t)0;
void * hyp_null_cap = (void *)(intcap_t)0;

void cheri_init_caps_reloc(void* root_data_cap, void* root_code_cap) {
  cheri_init_globals_3(root_data_cap, root_code_cap, root_data_cap);
  hyp_root_data_cap = root_data_cap;
  hyp_root_code_cap = root_code_cap;
  hyp_null_cap = INVALID_CAP(0);
}
#endif
