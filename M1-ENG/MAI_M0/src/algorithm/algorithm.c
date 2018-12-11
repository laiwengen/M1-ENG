/*
* algorithm.c
*
* Created: 2014/3/13 15:16:28
*  Author: john
*/
#include "algorithm.h"
#include "ad/ad.h"
#include "conf_adc.h"
#include <math.h>
#include "device/device.h"
#include "system/system.h"

#define SAMPLENUM 2048
#define AVG_BUFFER_SIZE 0x40/*0x04*/
#define PMXNUM 2
#define KVALUE /*1e-5*/ 0.5e-5
#define MVALUE 4 /*1*/ //2014-10-29
#define PVALUE /*1*/1.2	//20140224


fifo_desc_t g_fifo_desc[PMXNUM];
uint32_t g_buffer_for_fifo[PMXNUM][AVG_BUFFER_SIZE];//
uint32_t g_ground = 200;
const uint32_t g_ground_light_strength = 1650;

void algorithm_init()
{
	
	fifo_init(&g_fifo_desc[0],g_buffer_for_fifo[0],AVG_BUFFER_SIZE);
	fifo_init(&g_fifo_desc[1],g_buffer_for_fifo[1],AVG_BUFFER_SIZE);
}
void algorithm_calibrate(void)
{
	uint16_t i;
	uint32_t sum = 0;
	int8_t offset = 10;//20150122
	ad_swap_channel(ADCCH_DUST_DATA);
	
	adc_enable(&ADCA);
	delay_us(10);
	for (i=0; i<(1<<14); i++)
	{
		sum += ad_read();
	}
	adc_disable(&ADCA);
	delay_us(10);
	g_ground = (sum>>14)+offset;

	// 	ad_swap_channel(ADCCH_LIGHT_STRENGTH);
	// 	adc_enable(&ADCA);
	// 	delay_us(10);
	// 	sum = 0;
	// 	for(i=0;i<8;i++)
	// 	{
	// 		adc_start_conversion(&ADCA, ADC_CH0);
	// 		adc_wait_for_interrupt_flag(&ADCA, ADC_CH0);
	// 		sum += adc_get_result(&ADCA, ADC_CH0);
	// 	}
	// 	adc_disable(&ADCA);
	// 	delay_us(10);
	// 	g_ground_light_strength = sum>>3;
}

static inline void caldust(Dust_Info_t * dustinfo, uint8_t size)
{
	uint16_t i, j;
	int16_t currentdata = g_ground,last1 = g_ground, last2 = g_ground;
	int16_t kangle=0;
	uint16_t width=0,hight=0;
	enum
	{
		WAVE_LOW=0,
		WAVE_HIGH,
		WAVE_RISE,
		WAVE_FALL,
	} statu = WAVE_LOW;
	
	for (j=0; j<size; j++)
	{
		Dust_Info_t * di = (dustinfo+j);
		di->tmp_value = 0;
	}
	
	if (adc_is_enabled(&ADCA))
	{
		adc_disable(&ADCA);
		delay_us(10);
	}
	ad_swap_channel(ADCCH_DUST_DATA);
	adc_enable(&ADCA);
	delay_us(10);
	adc_start_conversion(&ADCA, ADC_CH0);
	for(i=0; i<SAMPLENUM; i++)
	{
		last2 = last1;
		last1 = currentdata;
		
		adc_wait_for_interrupt_flag(&ADCA, ADC_CH0);
		currentdata = max(0,adc_get_signed_result(&ADCA, ADC_CH0));
		adc_start_conversion(&ADCA, ADC_CH0);
		
		
		if (statu == WAVE_LOW)
		{
			if ((currentdata>last2+30) && (currentdata>last1))
			{
				statu = WAVE_RISE;
			}
		}
		else if (statu == WAVE_HIGH)
		{
			if ((currentdata<g_ground) || (currentdata+(kangle)<last2) || (currentdata+(1000)<last2) || ((last1<=currentdata) && (last1=last2) && currentdata<(hight-(hight>>3))))
			{
				statu = WAVE_FALL;
			}
			if ((currentdata>last1+kangle))
			{
				statu = WAVE_RISE;
				
			}
		}
		
		
		if (statu == WAVE_RISE)
		{
			width = 0;
			hight = 0;
			kangle = currentdata-last1;
			statu = WAVE_HIGH;
		}
		else if (statu == WAVE_HIGH)
		{
			if (hight+g_ground<(currentdata))
			{
				hight = currentdata-g_ground;
			}
			width ++;
		}
		else if (statu == WAVE_FALL)
		{
			for (j=0; j<size; j++)
			{
				Dust_Info_t * di = (dustinfo+j);
				if (width>=di->xmin && width<di->xmax)
				{
					uint32_t tmp;
				/*	hight = UINT32_MAX;
					width = UINT32_MAX-100;*/
					tmp = (width*(uint32_t)hight*(uint32_t)hight)>>12;
					//tmp = INT32_MAX;
					//di->tmp_value += (width*(uint32_t)hight*(di->xmin + di->xmax))>>9;
					if ( INT32_MAX-tmp > di->tmp_value ) 
					{
						di->tmp_value += tmp;//20140327 10->14
					}
				}
			}
			statu = WAVE_LOW;
		}
	}
	// 	Dust_Info_t * di = (dustinfo+j);
	// 	for (j=0; j<size; j++)
	// 	{
	// 		di->tmp_value += pow( ((sum>>11) - g_ground), 2);
	// 	}
	
	adc_wait_for_interrupt_flag(&ADCA, ADC_CH0);
	currentdata = max(0,adc_get_signed_result(&ADCA, ADC_CH0));
	adc_disable(&ADCA);
	
	#if HIGH_LIGHT_ADDITION
	delay_us(10);
	ad_swap_channel(ADCCH_LIGHT_STRENGTH);
	adc_enable(&ADCA);
	delay_us(10);
	
	{
		int32_t light_strength;
		light_strength= 0;
		for(i=0;i<128;i++)
		{
			adc_start_conversion(&ADCA, ADC_CH0);
			adc_wait_for_interrupt_flag(&ADCA, ADC_CH0);
			light_strength += max(0,adc_get_signed_result(&ADCA, ADC_CH0));
		}
		light_strength = (int32_t)(light_strength>>7)-(int32_t)g_ground_light_strength;
		// 	 	if(light_strength<300)
		// 	 	{
		// 		 	adc_disable(&ADCA);
		// 		 	for (j=0; j<size; j++)
		// 		 	{
		// 			 	Dust_Info_t * di = (dustinfo+j);
		// 			 	di->tmp_value = 0;
		// 		 	}
		// 		 	return;
		// 	 	}
		for (j=0; j<size; j++)
		{
			Dust_Info_t * di = (dustinfo+j);
			//di->tmp_value += ((sum-g_ground_light_strength)*(sum-g_ground_light_strength)*(sum-g_ground_light_strength))>>16;
			/*light_strength >>=1;*/
			if (light_strength>0)
			{
				light_strength = light_strength<<11;//20140327  11->8     //20350437 8->7 // max(light_strength) = 400;
			}
			else
			{
				light_strength = 0;
			}
			di->toohigh_value = light_strength;	//20140325  >>13<<2 for data2pm25
		}
	}
	adc_disable(&ADCA);
	#endif
}

void algorithm_calculate(Dust_Info_t * dustinfo, uint8_t size, bool preclear)
{
	uint8_t	j;
	uint8_t i;
	uint32_t pm25bufferavg;//
	int32_t deltadust;//
	double deltapm25;//
	
	
	if (!preclear)
	{
		caldust(dustinfo, size);
	}
	
	for(i=0; i<size; i++)
	{
		if (preclear)
		{
			dustinfo[i].dust_value = 0;
			fifo_flush(&g_fifo_desc[i]);
			continue;
		}
		
		if (fifo_is_full(&g_fifo_desc[i]))
		{
			uint32_t tmp;
			fifo_pull_uint32(&g_fifo_desc[i],&tmp);
		}
		fifo_push_uint32(&g_fifo_desc[i], dustinfo[i].tmp_value+dustinfo[i].toohigh_value);
		pm25bufferavg = 0;
		for (j=0; j<sizeof(g_buffer_for_fifo[i])/sizeof(uint32_t);j++)
		{
			pm25bufferavg += g_buffer_for_fifo[i][j];
		}
		dustinfo[i].tmp_value = pm25bufferavg*PVALUE/fifo_get_used_size(&g_fifo_desc[i]);
		
		
		#if 0
		dustinfo[i].dust_value = dustinfo[i].tmp_value;
		return;
#endif
		if (dustinfo[i].tmp_value > dustinfo[i].dust_value)
		{
			dustinfo[i].bigger_last = min(dustinfo[i].bigger_last+1,UINT16_MAX -1);
			dustinfo[i].smaller_last = 0;
		}
		else
		{
			dustinfo[i].smaller_last = min(dustinfo[i].smaller_last+1,UINT16_MAX -1);
			dustinfo[i].bigger_last = 0;
		}
		
		deltadust = dustinfo[i].tmp_value - dustinfo[i].dust_value;
		if (deltadust<0)
		{
			deltadust = -deltadust;
		}
		deltapm25 = deltadust*KVALUE*(1+(max(dustinfo[i].smaller_last,dustinfo[i].bigger_last))*MVALUE);
		deltapm25 = fmin( (deltadust+1)>>1, deltapm25);	//2014.2.14
		
		if (dustinfo[i].tmp_value > dustinfo[i].dust_value)
		{
			dustinfo[i].dust_value+=deltapm25;
		}
		else
		{
			dustinfo[i].dust_value-=deltapm25;
		}
		dustinfo[i].tmp_value = deltapm25;
		/**/
	}
}
