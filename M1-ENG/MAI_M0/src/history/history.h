/*
 * history.h
 *
 * Created: 2014/3/19 9:26:53
 *  Author: Airj
 */ 


#ifndef HISTORY_H_
#define HISTORY_H_
#include "compiler.h"

typedef struct test
{
	uint32_t time;
	uint32_t data;
} history_t;

bool history_read_next(history_t * data, bool isfirst);
void history_write(history_t * data);
void history_store_rtc(uint32_t rtc);
uint32_t history_resume_rtc(void);

#endif /* HISTORY_H_ */