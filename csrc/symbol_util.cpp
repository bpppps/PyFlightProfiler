#include "symbol_util.h"
#include <stdio.h>

static unsigned long nm_offset = 0;

#ifdef __cplusplus
extern "C" {
#endif

void set_nm_symbol_offset(unsigned long offset) { nm_offset = offset; }

void *get_symbol_address_by_nm_offset(unsigned long nm_addr) {
  return (void *)(nm_offset + nm_addr);
}

#ifdef __cplusplus
}
#endif
