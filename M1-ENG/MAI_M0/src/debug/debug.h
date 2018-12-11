/*
 * debug.h
 *
 * Created: 2014/3/6 9:40:46
 *  Author: Airj
 */ 


#ifndef DEBUG_H_
#define DEBUG_H_

#include <asf.h>
#include "compiler.h"


void debug_init(void);
void debug_print_string(const char * str);
void debug_print_uint16(uint16_t);
bool debug_rx_no_data(void);
uint8_t debug_has_new_cmd(uint8_t stopchar);
void debug_print_bytes(const uint8_t * bytes, const uint8_t len);
uint8_t debug_read_rx_buffer(uint8_t * data, uint8_t maxlen);
void debug_next_circle(void);


#define RX_BUFFER_SIZE 0x80

#endif /* DEBUG_H_ */