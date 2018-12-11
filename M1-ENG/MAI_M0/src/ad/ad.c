/*
 * ad.c
 *
 * Created: 2014/3/20 9:57:58
 *  Author: Airj
 */ 

#include "ad/ad.h"
#include "config/conf_adc.h"

void ad_swap_channel( enum adcch_positive_input channel)
{
	struct adc_channel_config adcch_conf;
	struct adc_config adc_conf;
	if (adc_is_enabled(&ADCA))
	{
		adc_disable(&ADCA);
		delay_us(10);
	}
	adcch_read_configuration(&ADCA, ADC_CH0, &adcch_conf);
	adc_read_configuration(&ADCA, &adc_conf);
	if (channel==ADCCH_BATTERY)
	{
		adc_set_conversion_parameters(&adc_conf, ADC_SIGN_ON, ADC_RES_12, ADC_REF_BANDGAP);
		adc_set_clock_rate(&adc_conf, 20000UL);
	}
	else
	{
		adc_set_conversion_parameters(&adc_conf, ADC_SIGN_ON, ADC_RES_12, ADC_REF_AREFA);
		adc_set_clock_rate(&adc_conf, 2000000UL);
	}
	adcch_set_input(&adcch_conf, channel, ADCCH_NEG_NONE, 1);	
	adc_write_configuration(&ADCA, &adc_conf);
	adcch_write_configuration(&ADCA, ADC_CH0, &adcch_conf);
	delay_us(10);
}

uint16_t ad_read(void)
{
	uint16_t rst;
	
	if (!adc_is_enabled(&ADCA))
	{
		adc_enable(&ADCA);
		delay_us(10);
	}
	adc_start_conversion(&ADCA, ADC_CH0);
	adc_wait_for_interrupt_flag(&ADCA, ADC_CH0);
	rst = max(0,adc_get_signed_result(&ADCA, ADC_CH0));
	return  rst;
}
