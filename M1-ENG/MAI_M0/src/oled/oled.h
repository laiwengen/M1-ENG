/*
 * oled.h
 *
 * Created: 2014/3/12 15:51:12
 *  Author: Airj
 */ 


#ifndef OLED_H_
#define OLED_H_

#include "asf.h"
#include "compiler.h"
#include "./config/conf_oled.h"
//extern uint8_t tmp_min ;
//extern uint8_t tmp_hour;
#define MAX_Column 256
#define MAX_ROW 64
#define XMAX 256
#define YMAX 64
#include "system/system.h"


typedef struct oled_status
{
	system_status_t system_status;
	uint8_t battery_level;
	uint32_t pm2_5;
	uint32_t pm10;
	bool is_hold;
	uint32_t time;
	uint8_t aqi_level;
	bool is_charge;
	bool show_low_power;
} oled_status_t;
static inline void oled_enable(void)
{
	ioport_set_pin_high(OLED_ENABLE);
}

static inline void oled_disable(void)
{
	ioport_set_pin_low(OLED_ENABLE);
}

void oled_init(void);
void oled_update(oled_status_t * oled_status);
void oled_inform_check_result(void);
void oled_show_check_level(uint8_t level);
#endif /* OLED_H_ */