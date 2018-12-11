/*
 * battery.h
 *
 * Created: 2014/3/12 20:18:09
 *  Author: john
 */ 


#ifndef BATTERY_H_
#define BATTERY_H_

#include "asf.h"
#include "compiler.h"

#define BATTERY_PIN_CHARGE IOPORT_CREATE_PIN(PORTD, 0)
#define BATTERY_PIN_ADC IOPORT_CREATE_PIN(PORTA, 2)

void battery_init(void);

static inline bool battery_is_charging(void)
{
	return ioport_get_pin_level(BATTERY_PIN_CHARGE);
}

void battery_store_last_level(bool pre);


uint16_t battery_get_level(void);


#endif /* BATTERY_H_ */