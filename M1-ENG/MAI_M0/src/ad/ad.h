/*
 * ad.h
 *
 * Created: 2014/3/17 12:14:49
 *  Author: Airj
 */ 


#ifndef AD_H_
#define AD_H_

#include "asf.h"
#include "compiler.h"


void ad_swap_channel( enum adcch_positive_input channel);
static inline void ad_init(void)
{
	struct adc_config adc_conf;

	adc_read_configuration(&ADCA, &adc_conf);
	ioport_set_pin_dir(IOPORT_CREATE_PIN(PORTA,0),IOPORT_DIR_INPUT);
	adc_set_conversion_parameters(&adc_conf, ADC_SIGN_ON, ADC_RES_12, ADC_REF_AREFA);
	adc_set_conversion_trigger(&adc_conf, ADC_TRIG_MANUAL, 1, 0);
	adc_set_clock_rate(&adc_conf, 2000000UL);
	ad_swap_channel(ADCCH_BATTERY);
	adc_write_configuration(&ADCA, &adc_conf);
}

uint16_t ad_read(void);

#endif /* AD_H_ */