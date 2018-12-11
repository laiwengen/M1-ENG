/*
 * oled_driver.h
 *
 * Created: 2014/3/12 16:28:23
 *  Author: Airj
 */ 


#ifndef OLED_DRIVER_H_
#define OLED_DRIVER_H_

#define TIMEOUTCNT 10
#include <asf.h>
#include "conf_oled.h"
#include "compiler.h"
static inline uint8_t m2s(uint8_t dat)
{
	return ((dat & 0x01)<< PIN4_bp)|(((dat & 0x02)>>1)<<PIN0_bp)|(((dat & 0x04)>>2)<<PIN5_bp)|(((dat & 0x08)>>3)<<PIN1_bp)|(((dat & 0x10)>>4)<<PIN6_bp)|(((dat & 0x20)>>5)<<PIN2_bp)|(((dat & 0x40)>>6)<<PIN7_bp)|(((dat & 0x80)>>7)<<PIN3_bp);
}
static inline uint8_t s2m(uint8_t dat)
{
	return ((dat & 0x01)<< 1)|(((dat & 0x02)>>1)<<3)|(((dat & 0x04)>>2)<<5)|(((dat & 0x08)>>3)<<7)|(((dat & 0x10)>>4)<<1)|(((dat & 0x20)>>5)<<2)|(((dat & 0x40)>>6)<<4)|(((dat & 0x80)>>7)<<6);
	
}

uint8_t oled_rdat(void);
void oled_wdat(uint8_t dat);
uint8_t oled_rcmd(void);
void oled_wcmd(uint8_t dat);
static inline void oled_ic_reset(void)
{
	ioport_set_pin_low(OLED_CS);
	ioport_set_pin_low(OLED_RES);
	delay_us(20);
	ioport_set_pin_high(OLED_RES);
	delay_us(20);
	ioport_set_pin_high(OLED_CS);
}

#endif /* OLED_DRIVER_H_ */