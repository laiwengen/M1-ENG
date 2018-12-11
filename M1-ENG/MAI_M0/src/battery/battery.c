/*
 * battery.c
 *
 * Created: 2014/3/12 20:18:22
 *  Author: john
 */ 

#include "battery.h"
#include "ad/ad.h"
#include "conf_adc.h"
#include "device/device.h"
#include "conf_battery.h"
#include <avr/eeprom.h>

void battery_init(void)
{
	ioport_set_pin_dir(BATTERY_PIN_CHARGE, IOPORT_DIR_INPUT);
	ioport_set_pin_dir(BATTERY_PIN_ADC, IOPORT_DIR_INPUT);
}

void battery_store_last_level(bool pre)
{
	static bool done = false;
	if (battery_is_charging())
	{
		return;
	}
	if (pre)
	{
		done = false;
		return;
	}
	if (done)
	{
		return;
	}
	eeprom_busy_wait();
	eeprom_write_word(BATTERY_EEPROM_LAST_LEVEL_ADDRESS,battery_get_level());
	eeprom_busy_wait();
	eeprom_write_dword(BATTERY_EEPROM_LAST_TIME_ADDRESS,rtc_get_time());
	done = true;
}

static inline uint16_t battery_get_level_uncharging(uint8_t isCharging)
{
	static uint16_t last_ad_resault = 3800;
	const static uint16_t level_map[] = {3600,3650,3720,3770,3800,3850,3900,3950};
	uint16_t i;
	uint16_t current_resault;
	static uint8_t last_level = 3;
	ad_swap_channel(ADCCH_BATTERY);
	current_resault = ad_read();
	adc_disable(&ADCA);
	if (device_is_high(DEVICE_FAN_ENABLE))
	{
		current_resault += 13;
	}
	//ad_swap_channel(ADCCH_BATTERY);
	last_ad_resault = (current_resault + ((uint32_t)last_ad_resault))/2;
	volatile uint32_t voltage = last_ad_resault*2.45;//signed >>1 / 0.204 (voltage divider)
	if (isCharging)
	{
		voltage *= 0.97;
	}
	//last_ad_resault = (current_resault + ((uint32_t)last_ad_resault)*31)>>5;
	for (i=0;i<sizeof(level_map)/sizeof(uint16_t);i++)
	{
		if (voltage < level_map[i])
		{
			break;
		}
	}
	if((i&1) == 0)
	{
		last_level = i;
	}
	else if (abs((int8_t)i-(int8_t)last_level)>1)
	{
		last_level = (i>>1)<<1;
	}
	return last_level;
}
static inline uint16_t battery_get_level_charging(void)
{
	//volatile uint32_t time, level;
	//eeprom_busy_wait();
	//level = eeprom_read_word(BATTERY_EEPROM_LAST_LEVEL_ADDRESS);
	//if (level>8)
	//{
	//level=1;
	//}
	//eeprom_busy_wait();
	//volatile uint32_t currentTime = rtc_get_time();
	//uint32_t chargeStartTime = eeprom_read_dword(BATTERY_EEPROM_LAST_TIME_ADDRESS);
	//if (chargeStartTime == UINT32_MAX||chargeStartTime == 0)
	//{
	//chargeStartTime = currentTime;
	//eeprom_busy_wait();
	//eeprom_write_dword(BATTERY_EEPROM_LAST_TIME_ADDRESS,currentTime);
	//
	//}
	//time = min(UINT32_MAX>>1,currentTime - chargeStartTime);
	//return min(8, level + (time>>11));
	return (battery_get_level_uncharging(true));
}

uint16_t battery_get_level(void)
{
	if (battery_is_charging())
	{
		return battery_get_level_charging();
	}
	else
	{
		return battery_get_level_uncharging(false);
	}
}