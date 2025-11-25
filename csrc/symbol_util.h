#ifndef __SYMBOL_UTIL_H__
#define __SYMBOL_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

void set_nm_symbol_offset(unsigned long offset);

void *get_symbol_address_by_nm_offset(unsigned long nm_addr);

#ifdef __cplusplus
}
#endif

#endif
