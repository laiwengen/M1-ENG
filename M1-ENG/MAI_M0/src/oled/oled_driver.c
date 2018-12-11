/*
 * oled_driver.c
 *
 * Created: 2014/3/13 11:09:35
 *  Author: Airj
 */ 

#include "oled/oled_driver.h"

uint8_t oled_rcmd(void)
{
	uint8_t rst;
	ioport_set_pin_high(OLED_WR);
	ioport_set_port_dir(OLED_DATA_PORT,0xff,IOPORT_DIR_INPUT);
	ioport_set_group_high(OLED_DATA_PORT,0xff);
	ioport_set_pin_low(OLED_DC);
	ioport_set_pin_low(OLED_CS);
	ioport_set_pin_low(OLED_RD);
	rst = s2m(ioport_get_port_level(OLED_DATA_PORT,0xff));
	ioport_set_pin_high(OLED_RD);
	ioport_set_pin_high(OLED_CS);
	return rst;
}
void oled_wcmd(uint8_t com)

{
	uint8_t i=0;
	while(oled_rcmd()&0x80)
	{
		i++;
		if (i>TIMEOUTCNT)
		{
			return;
		}
	}
	ioport_set_port_dir(OLED_DATA_PORT,0xff,IOPORT_DIR_OUTPUT);
	ioport_set_pin_low(OLED_DC);
	ioport_set_pin_low(OLED_CS);
	ioport_set_pin_low(OLED_WR);
	ioport_set_port_level(OLED_DATA_PORT,0xff,m2s(com));
	ioport_set_pin_high(OLED_WR);
	ioport_set_pin_high(OLED_CS);
}
void oled_wdat(uint8_t dat)
//void oled_wdat(uint8_t dat)
{
	uint8_t i=0;
	while(oled_rcmd()&0x80)
	{
		i++;
		if (i>TIMEOUTCNT)
		{
			return;
		}
	}
	ioport_set_port_dir(OLED_DATA_PORT,0xff,IOPORT_DIR_OUTPUT);
	ioport_set_pin_high(OLED_DC);
	ioport_set_pin_low(OLED_CS);
	ioport_set_pin_low(OLED_WR);
	ioport_set_port_level(OLED_DATA_PORT,0xff,(dat));
	ioport_set_pin_high(OLED_WR);
	ioport_set_pin_high(OLED_CS);
}
uint8_t oled_rdat(void)
{
	uint8_t rst;
	ioport_set_pin_high(OLED_WR);
	ioport_set_port_level(OLED_DATA_PORT,0xff,0x00);
	ioport_set_pin_high(OLED_DC);
	ioport_set_pin_low(OLED_CS);
	ioport_set_pin_low(OLED_RD);
	delay_us(1);
	rst = (ioport_get_port_level(OLED_DATA_PORT,0xff));
	ioport_set_pin_high(OLED_RD);
	ioport_set_pin_high(OLED_CS);
	return rst;
}
