#ifndef __INC_ARRAY_H
#define __INC_ARRAY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int get_endianness();
float float_from_byte_array(uint8_t *byte_array, int start_index);
uint16_t short_from_byte_array(uint8_t *byte_array, int start_index);
int float_into_byte_array(uint8_t *byte_array, int index, float flt);

#ifdef __cplusplus
}
#endif

#endif /*  __INC_ARRAY_H */
