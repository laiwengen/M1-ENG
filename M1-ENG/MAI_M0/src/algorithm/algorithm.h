/*
 * algorithm.h
 *
 * Created: 2014/3/13 15:16:24
 *  Author: john
 */ 


#ifndef ALGORITHM_H_
#define ALGORITHM_H_

#include "compiler.h"
#include "asf.h"

typedef struct
{
	uint8_t xmin;
	uint8_t xmax;
	int16_t bigger_last;
	int16_t smaller_last;
	int32_t dust_value;
	int32_t tmp_value;
	int32_t toohigh_value;
	int32_t sumValue;
	int16_t sumCnt;
}Dust_Info_t;

static inline uint8_t get_aqi_level(uint32_t pm25)
{
	uint8_t i;
	const static uint8_t airlvl[]={35,75,110,150,250};
	
	uint16_t tmp = pm25/100;
	
	for(i=0;i<5;i++)
	{
		if(tmp<=airlvl[i])
		{
			break;
		}
	}
	return i;
}
void algorithm_calibrate(void);
void algorithm_init(void);
void algorithm_calculate(Dust_Info_t * dustinfo, uint8_t size, bool preclear);

#endif /* ALGORITHM_H_ */