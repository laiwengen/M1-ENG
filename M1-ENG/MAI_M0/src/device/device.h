/*
 * device.h
 *
 * Created: 2014/3/14 19:46:51
 *  Author: Airj
 */ 


#ifndef DEVICE_H_
#define DEVICE_H_

#include "asf.h"
#include "conf_device.h"

static inline void device_init(void)
{
	
// 	ioport_set_pin_mode(DEVICE_KEY_POWER,IOPORT_PULL_UP);
// 	ioport_set_pin_mode(DEVICE_KEY_SELECT,IOPORT_PULL_UP);
// 	ioport_set_pin_mode(DEVICE_KEY_MODE,IOPORT_PULL_UP);
	ioport_set_pin_dir(DEVICE_KEY_POWER,IOPORT_DIR_INPUT);
	ioport_set_pin_dir(DEVICE_KEY_SELECT,IOPORT_DIR_INPUT);
	ioport_set_pin_dir(DEVICE_KEY_MODE,IOPORT_DIR_INPUT);
	
	//laser
	ioport_set_pin_dir(DEVICE_LASER_ENABLE,IOPORT_DIR_OUTPUT);
	ioport_set_pin_low(DEVICE_LASER_ENABLE);
	
	//fan
	ioport_set_pin_dir(DEVICE_FAN_ENABLE,IOPORT_DIR_OUTPUT);
	ioport_set_pin_low(DEVICE_FAN_ENABLE);
	
	//power
	ioport_set_pin_dir(DEVICE_VCC_12V_POWER_ENABLE,IOPORT_DIR_OUTPUT);
	ioport_set_pin_low(DEVICE_VCC_12V_POWER_ENABLE);
	
	//analog
	ioport_set_pin_dir(DEVICE_AVCC_3V_POWER_ENABLE,IOPORT_DIR_OUTPUT);
	ioport_set_pin_low(DEVICE_AVCC_3V_POWER_ENABLE);
	
}

static inline void device_enable(port_pin_t pin)
{
	ioport_set_pin_high(pin);
}

static inline void device_disable(port_pin_t pin)
{
	ioport_set_pin_low(pin);
}

static inline bool device_is_high(port_pin_t pin)
{
	return ioport_get_pin_level(pin); 
}


#endif /* DEVICE_H_ */