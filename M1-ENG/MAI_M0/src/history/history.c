/*
 * history.c
 *
 * Created: 2014/3/19 9:34:51
 *  Author: Airj
 */ 

#include "history/history.h"
#include "asf.h"
#include "conf_history.h"
#include <avr/eeprom.h>

void history_get_index(uint8_t * read_index, uint8_t * write_index)
{
//	eeprom_busy_wait();
// 	*read_index = eeprom_read_byte(HISTORY_EEPROM_READ_INDEX_ADDRESS);
// 	eeprom_busy_wait();
// 	*write_index = eeprom_read_byte(HISTORY_EEPROM_WRITE_INDEX_ADDRESS);
}
static inline bool history_read_with_index(history_t * history, uint8_t index)
{
	volatile uint8_t read_index, write_index, size;
	uint16_t indexs;
	eeprom_busy_wait();
	indexs = eeprom_read_word(HISTORY_EEPROM_WRITE_INDEX_ADDRESS);
	read_index = indexs >> 8;
	write_index = indexs & 0xff;
	size = (write_index - read_index)&HISTORY_EEPROM_MASK;
	if (index >= size)
	{
		return false;
	}
	eeprom_busy_wait();
	history->time = eeprom_read_dword(HISTORY_EEPROM_ADDRESS_BASE+((write_index - index - 1)&(HISTORY_EEPROM_MASK>>1))*sizeof(history_t));
	eeprom_busy_wait();
	history->data = eeprom_read_dword(HISTORY_EEPROM_ADDRESS_BASE+((write_index - index - 1)&(HISTORY_EEPROM_MASK>>1))*sizeof(history_t)+sizeof(history->time));
	return true;
}

bool history_read_next(history_t * data, bool isfirst)
{
	static uint8_t read_offset;
	if (isfirst)
	{
		read_offset = 0;
	}
	if (history_read_with_index(data, read_offset))
	{
		read_offset ++;
		return true;
	}
	else
	{
		if (read_offset == 0)
		{
			return false;
		}
		read_offset = 0;
		return history_read_next(data,false);
	}
}

void history_write(history_t * data)
{
	volatile uint8_t read_index, write_index, size;
	uint16_t indexs;
	eeprom_busy_wait();
	indexs = eeprom_read_word(HISTORY_EEPROM_WRITE_INDEX_ADDRESS);
	read_index = indexs >> 8;
	write_index = indexs & 0xff;
	size = (write_index - read_index)&HISTORY_EEPROM_MASK;
	if (size >= HISTORY_EEPROM_SIZE)
	{
		eeprom_busy_wait();
		eeprom_write_byte(HISTORY_EEPROM_READ_INDEX_ADDRESS,(read_index+1)&HISTORY_EEPROM_MASK);
	}
	eeprom_busy_wait();
	eeprom_write_dword(HISTORY_EEPROM_ADDRESS_BASE+((write_index)&(HISTORY_EEPROM_MASK>>1))*sizeof(history_t),data->time);
	eeprom_busy_wait();
	eeprom_write_dword(HISTORY_EEPROM_ADDRESS_BASE+((write_index)&(HISTORY_EEPROM_MASK>>1))*sizeof(history_t)+sizeof(data->time),data->data);
	eeprom_busy_wait();
	eeprom_write_byte(HISTORY_EEPROM_WRITE_INDEX_ADDRESS,(write_index+1)&HISTORY_EEPROM_MASK);
}

void history_store_rtc(uint32_t rtc)
{
	eeprom_busy_wait();
	eeprom_write_dword(HISTORY_EEPROM_RTC_ADDRESS,rtc);
}

uint32_t history_resume_rtc(void)
{
	eeprom_busy_wait();
	return eeprom_read_dword(HISTORY_EEPROM_RTC_ADDRESS);
}