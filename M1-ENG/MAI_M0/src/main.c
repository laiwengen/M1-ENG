/**
* \file
*
* \brief Empty user application template
*
*/

/**
* \mainpage User Application template doxygen documentation
*
* \par Empty user application template
*
* Bare minimum empty user application template
*
* \par Content
*
* -# Include the ASF header files (through asf.h)
* -# Minimal main function that starts with a call to board_init()
* -# "Insert application code here" comment
*
*/

/*
* Include header files for all drivers that have been imported from
* Atmel Software Framework (ASF).
*/
#include <asf.h>
//#include "debug/debug.h"
#include "conf_sys.h"
#include "device/device.h"
#include "system/system.h"
#include "oled/oled.h"
#include "history/history.h"

#include "ad/ad.h"
#include "algorithm/algorithm.h"
#include "battery/battery.h"


#include "conf_history.h"
#include "avr/eeprom.h"
#include "reset_cause.h"
#include "serial/serial.h"
#define MULTIPLY 0.77*0.67
#define TABLE_LENGTH 24
#define MUTIP_130 140
//lizonglei svn test
USART_t * usart2=&USARTD0;
usart_rs232_options_t usart2_options = {.baudrate = 115200UL,.charlength = USART_CHSIZE_8BIT_gc,.paritytype=USART_PMODE_DISABLED_gc,.stopbits = false};
volatile uint8_t g_main_unlock_step = LOCK_CALIBRATION;

typedef struct
{
	uint8_t downlast;
	uint8_t uplast;
	bool level;
} key_t;

key_t * g_power_key_pointer = NULL;

static system_status_t current_system_status = SYSTEM_POWERDOFF;

// ISR(_VECTOR(0))
// {
// 	eeprom_write_byte(HISTORY_EEPROM_WRITE_INDEX_ADDRESS,0xff&HISTORY_EEPROM_MASK);
// }
////////////////////////////////////////////////////
static inline void UARTD0_init(void)
{
	// RX on PC2 TX on PC3
	//ioport_set_pin_high(IOPORT_CREATE_PIN(PORTC, 2));
	////if (MASTER_MODE)
	//{
	//ioport_set_pin_dir(IOPORT_CREATE_PIN(PORTC, 2),IOPORT_DIR_INPUT);
	//ioport_set_pin_dir(IOPORT_CREATE_PIN(PORTC, 3),IOPORT_DIR_OUTPUT);
	//}
	//else
	{
		ioport_set_pin_dir(IOPORT_CREATE_PIN(PORTD, 2),IOPORT_DIR_INPUT);//  eliminate IO powering USB5V across USB protect IC.
		ioport_set_pin_dir(IOPORT_CREATE_PIN(PORTD, 3),IOPORT_DIR_INPUT);
	}
	pmic_enable_level(PMIC_LVL_MEDIUM);
	Enable_global_interrupt();
	
	stdio_serial_init(usart2,&usart2_options);
	usart_set_rx_interrupt_level(usart2,USART_INT_LVL_MED);
	
}

//old BAM line
//const static uint16_t dx[] = {90,170,323,333,440,873,273,1020,663,997,1927,1517,2470,5220,4893,7647,10313,14350,21667};
//const static uint16_t dy[] = {753,503,1908,1476,1405,2786,1677,2948,2841,1927,4174,2256,3126,3877,2186,3095,4436,11027,29600 };
//BAM laiwengen mapping 
//const static uint16_t dx[] = {1900,400,700,600,900,1800,1200,2500,2300,4400,2300,2800,4400,4800,4200,3600,6200,6800,6200,3000,3200};
//const static uint16_t dy[] = {9500,1678,3402,2916,4104,4041,3309,3650,3316,2995,1369,1140,2334,1246,2168,2496,2986,2776,3574,3000,4500};
//new BAM N1&M1 20160304
				
#if TSI_MODE
const static uint32_t dx[] = {5000,5000,70000,10000};
const static uint32_t dy[] = {12000,6000,42000,10000};
#else
const static uint16_t dx[] = {1900,400,700,600,900,1800,1200,2500,2300,4400,2300,2800,4400,4800,4200,3600,6200,6800,6200,3000,3200};
const static uint16_t dy[] = {7125,1258,2551,2187,2378,4000,2500,5000,4000,6500,2500,1500,2254,1746,2100,2064,2986,2776,3574,3000,4500};
#endif 
static uint32_t data2pm25(Dust_Info_t * dustinfo,bool needCorrect,bool isPm10)
{
	uint8_t i=0;
	uint32_t pm25;
	Dust_Info_t * di = (dustinfo);
	uint32_t data = di->dust_value- di->toohigh_value;//dustinfo[i].tmp_value+dustinfo[i].toohigh_value
	uint32_t base;
	if (isPm10)
	{
		//data += di->dust_value >>2+ di[1].dust_value;
	}

	data = data >>2;

	if (DEBUG_MODE)
	{
		return data/10;
	}

	if (needCorrect)
	{
		pm25_get_correction(data,&current_system_status,g_main_unlock_step);
				
	}
	data = pm25_set_correction(data);
	//data += di->toohigh_value;//add by liujing 20171025 for new TSI mapping to reach 99999
	data += di->toohigh_value>>2;
	base = 0;


	for(i=0;i<(sizeof(dx)/sizeof(uint32_t))-1;i++)
	{
		if (data<dx[i])
		{
			break;
		}
		data -= dx[i];
		base += dy[i];
	}
	pm25 = base+ (data*dy[i])/dx[i];
	//pm25=data;
	if (pm25>99999UL)
	{
		pm25 = 99999UL;
	}
	return (uint32_t)pm25;
}

uint32_t pm252data(uint32_t pm25)
{
	uint32_t base = 0;
	uint32_t data = 0;
	uint8_t i;
	for(i=0;i<(sizeof(dy)/sizeof(uint16_t))-1;i++)
	{
		if (pm25<dy[i])
		{
			break;
		}
		pm25 -= dy[i];
		base += dx[i];
	}
	data = base+ (pm25*dx[i])/dy[i];
	return data;
}
//////////////////////////////////////////////////

static inline void circle_tc_init(void)
{
	tc_enable(&CIRCLE_TC);
	tc_set_wgm(&CIRCLE_TC,TC_WG_NORMAL);
	tc_write_period(&CIRCLE_TC, 2000);
	tc_write_clock_source(&CIRCLE_TC, TC_CLKSEL_DIV256_gc);
}

static inline bool is_time_for_new_circle(void)
{
	return tc_is_overflow(&CIRCLE_TC);
}

static void rtc_callback( uint32_t count)
{
	history_store_rtc(count);
	rtc_set_alarm_relative(10);
//	battery_store_last_level(true);
}

static inline void scan_key(system_status_t * current_system_status, bool * system_status_changed, bool * sub_function)
{
	enum key_index_t {POWER = 0, SELECT = 1, MODE = 2};
	static system_status_t last_system_status = SYSTEM_REALTIME;
	
	static key_t key[3];
	const static port_pin_t key_pin[] = {DEVICE_KEY_POWER,DEVICE_KEY_SELECT,DEVICE_KEY_MODE};
	uint8_t i;
	
	g_power_key_pointer = &key[0];
	
	for (i=0; i< sizeof(key)/sizeof(key_t);i++)
	{
		if (device_is_high(key_pin[i]))
		{
			if (key[i].level == false && key[i].downlast >= KEY_CLICK_TIME)
			{
				key[i].level = true;
				key[i].uplast = 0;
			}
			key[i].uplast = min( (key[i].uplast)+1, 0xF0);
		}
		else
		{
			if (key[i].level == true && key[i].uplast >= KEY_CLICK_TIME)
			{
				key[i].level = false;
				key[i].downlast = 0;
			}
			key[i].downlast = min( (key[i].downlast)+1, 0xF0);
		}
	}
	if (*current_system_status == SYSTEM_POWERDOFF)
	{
		if (key[POWER].downlast == KEY_LONG_HOLD_TIME && key[POWER].level == false)
		{
			*current_system_status = last_system_status;
			*system_status_changed = true;
		}
	}
	else
	{
		if ((key[POWER].downlast == KEY_LONG_HOLD_TIME && key[POWER].level == false)\
		|| (key[POWER].downlast > KEY_CLICK_TIME && key[POWER].downlast < KEY_LONG_HOLD_TIME && key[POWER].level == true && key[POWER].uplast == KEY_CLICK_TIME)\
		|| (key[POWER].downlast > KEY_IGNORE_HOLD_TIME && key[POWER].level == false))
		{
			last_system_status = *current_system_status;
			*current_system_status = SYSTEM_POWERDOFF;
			*system_status_changed = true;
		}
	}
	if (*current_system_status != SYSTEM_POWERDOFF)
	{
		if ((key[MODE].downlast == KEY_HOLD_TIME && key[MODE].level == false)\
		|| (key[MODE].downlast > KEY_CLICK_TIME && key[MODE].downlast < KEY_HOLD_TIME && key[MODE].level == true && key[MODE].uplast == KEY_CLICK_TIME))
		{
			if (*current_system_status != SYSTEM_HISTORY)
			{
				(*current_system_status) ++;
			}
			else
			{
				(*current_system_status) = SYSTEM_REALTIME;
			}
			*system_status_changed = true;
		}
	}
	if (*current_system_status != SYSTEM_POWERDOFF)
	{
		if ((key[SELECT].downlast == KEY_HOLD_TIME && key[SELECT].level == false)\
		|| (key[SELECT].downlast > KEY_CLICK_TIME && key[SELECT].downlast < KEY_HOLD_TIME && key[SELECT].level == true && key[SELECT].uplast == KEY_CLICK_TIME))
		{
			*sub_function = true;
		}
		else if ((key[SELECT].downlast >= KEY_HOLD_TIME && key[SELECT].level == false) && (*current_system_status == SYSTEM_HISTORY || *current_system_status == SYSTEM_HISTORY))
		{
			*sub_function = true;
		}
	}
}
static inline void check_uart(system_status_t * current_system_status, bool * system_status_changed,uint32_t currentPm25)
{
	if (battery_is_charging()==0)
	{
		return;
	}
	switch (CheckUART2Rev(&g_RevGasData))
	{
		
		case COMMAND_POWERON:
		{
			if (g_main_unlock_step==UNLOCK_CALIBRATION)
			{
				*current_system_status = SYSTEM_REALTIME;
				*system_status_changed = true;
				g_main_unlock_step = LOCK_CALIBRATION;
			}
			//else
			//{
				////last_system_status = *current_system_status;
				////if (*current_system_status == SYSTEM_WIFI_INIT || *current_system_status == SYSTEM_HCHO || *current_system_status == SYSTEM_TEM_HUM || *current_system_status == SYSTEM_POWERSAVE)
				////{
				////last_system_status = SYSTEM_REALTIME;
				////}
				////*current_system_status = SYSTEM_POWERDOFF;
				//*system_status_changed = true;
				////				hts221_shutdown();							//20150511
				////is_hts221_on = false;			//20150511
			//}
			
		}
		break;
		case COMMAND_SWITCH:
		{
			if (g_main_unlock_step==UNLOCK_CALIBRATION)
			{
				if (*current_system_status != SYSTEM_REALTIME)
				{
					(*current_system_status) = SYSTEM_REALTIME;
				}
				else
				{
					(*current_system_status) = SYSTEM_POWERSAVE;
				}
				*system_status_changed = true;
				g_main_unlock_step = LOCK_CALIBRATION;
			}
		}
		break;
		case PM25_CALIBRATION_CODE:
		//the staff is in data2pm25() --pm251sCorrect() to pass the modifier to all watchers (wifi,oled,and so on)
		//(*current_system_status) = SYSTEM_REALTIME;
		if (*current_system_status== SYSTEM_POWERDOFF)
		{
			*current_system_status= SYSTEM_REALTIME;
			
		}
		if (g_main_unlock_step==UNLOCK_CALIBRATION)
		{
		g_main_unlock_step = READY_TO_CALIBRATION;
		}
		else
		{
		g_main_unlock_step = LOCK_CALIBRATION;
				
		}
		
		*system_status_changed = true;
		break;
		case PM25_UNLOCK_CODE:
		//the staff is in data2pm25() --pm251sCorrect() to pass the modifier to all watchers (wifi,oled,and so on)
		//(*current_system_status) = SYSTEM_REALTIME;

	
			g_main_unlock_step = UNLOCK_CALIBRATION;

		*system_status_changed = true;
		break;
		case PM25_ERASE_CODE:
			if (g_main_unlock_step==UNLOCK_CALIBRATION)
			{
				g_main_unlock_step = LOCK_CALIBRATION;
				erasePm25Clibration();
				
			}
		*system_status_changed = true;
		break;
		
		
		
		default:
		break;
	}
}

static void circle_callback(void)
{
	static uint32_t circle_index=0;
	//atic system_status_t current_system_status = SYSTEM_POWERDOFF;
	static uint8_t oled_updata_index;
	static uint32_t power_save_calculate_index;
	static uint32_t last_operate_index;
	static bool is_hold = false;
	static uint8_t pre_power_off_left = 0;
	
	static bool need_recalculate = false;
	//static uint32_t pm25_times_100 = 0;
	//static uint32_t pm10 = 0;
	//static uint8_t sleep_delay = 0;
	static Dust_Info_t dust_info[2] = {{.xmin = 0,.xmax = 24, .sumValue = 0, .sumCnt = 0},{.xmin = 8, .xmax = 48, .sumValue = 0, .sumCnt = 0}};
	
	static oled_status_t oled_status;
	static history_t history;
	uint8_t battery_level;
	bool is_charging;
	
	volatile bool last_hold = false;
	volatile bool sub_function = false;
	volatile bool system_status_changed = false;
	volatile bool has_operation = false;
	volatile uint8_t need_calculate = false;
	
	
	if (reset_cause_is_external_reset())	//20140414
	{
		eeprom_write_byte(HISTORY_EEPROM_WRITE_INDEX_ADDRESS,0xff&HISTORY_EEPROM_MASK);
		eeprom_write_byte(HISTORY_EEPROM_READ_INDEX_ADDRESS,0xff);
		reset_cause_clear_causes(CHIP_RESET_CAUSE_EXTRST);
		current_system_status = SYSTEM_REALTIME;
	}
	
	circle_index ++;
	
	scan_key(&current_system_status,&system_status_changed, &sub_function);
	check_uart(&current_system_status,&system_status_changed,oled_status.pm2_5);
	has_operation = system_status_changed || sub_function;
	
	is_charging = battery_is_charging();
	if (is_charging)
	{
		//{
		//ioport_reset_pin_mode(IOPORT_CREATE_PIN(PORTD, 2));//  eliminate IO powering USB5V across USB protect IC.
		//ioport_reset_pin_mode(IOPORT_CREATE_PIN(PORTD, 3));
		//}
		//ioport_set_pin_dir(IOPORT_CREATE_PIN(PORTD, 2),IOPORT_DIR_OUTPUT);//  eliminate IO powering USB5V across USB protect IC.
		//ioport_set_pin_dir(IOPORT_CREATE_PIN(PORTD, 3),IOPORT_DIR_OUTPUT);
	}
	if (!(current_system_status == SYSTEM_POWERDOFF && !is_charging))
	{
		device_enable(DEVICE_VCC_12V_POWER_ENABLE);
		oled_enable();
	}
	if (system_status_changed)
	{
		is_hold = false;
	}
	if (has_operation)
	{
		power_save_calculate_index = ((circle_index - 1) & CIRCLE_POWERSAVE_CALCULATE_MASK);
		oled_updata_index = circle_index & CIRCLE_OLED_UPDATE_MASK;
		last_operate_index = circle_index;
	}
	
	if (current_system_status != SYSTEM_POWERSAVE)
	{
		#if !NO_AUTO_POWEROFF
		if (circle_index - last_operate_index > CIRCLE_AUTO_POWER_OFF_INDEX && current_system_status != SYSTEM_POWERDOFF)
		{
			if(!is_charging)		//20140411
			{
				current_system_status = SYSTEM_POWERDOFF; //auto turn off, 2min no operation
				oled_updata_index = circle_index & CIRCLE_OLED_UPDATE_MASK;
			}
			else
			{
				last_operate_index = circle_index;
			}
		}
		#endif
	}
	else
	{
		if ((circle_index & CIRCLE_POWERSAVE_CALCULATE_MASK)==power_save_calculate_index)
		{
			last_operate_index = circle_index;  //power save auto start
			is_hold = false;
		}
	}
	battery_level = battery_get_level();
//	battery_store_last_level(false);
	
	if (battery_level == 0 && !is_charging)
	{
		if (pre_power_off_left == 0 &&(/*!device_is_high(DEVICE_KEY_POWER)||*/current_system_status != SYSTEM_POWERDOFF))
		{
			pre_power_off_left = 50*3;
			oled_updata_index = circle_index & CIRCLE_OLED_UPDATE_MASK;
		}
		current_system_status = SYSTEM_POWERDOFF; //auto turn off for low battary
	}
	else if (is_charging)
	{
		pre_power_off_left = 0;
	}
	last_hold = is_hold;
	if (sub_function)
	{
		is_hold = !is_hold;
	}
	#if !NO_HOLD_NO_POWEROFF
	if ( (current_system_status == SYSTEM_REALTIME && circle_index - last_operate_index > CIRCLE_AUTO_HOLD_INDEX_REALTIME)\
	||  (current_system_status == SYSTEM_POWERSAVE && circle_index - last_operate_index > CIRCLE_AUTO_HOLD_INDEX_POWERSAVE)  )		//20140411
	{
		if(!is_charging)
		{
			is_hold = true; // auto hold
		}
		else
		{
			last_operate_index = circle_index;
		}
	}
	#endif
	if (current_system_status == SYSTEM_HISTORY)
	{
		bool no_data_left = false;
		if (has_operation)
		{
			no_data_left = !history_read_next(&history, system_status_changed);
			oled_status.pm2_5 = history.data;
		}
		oled_status.time = rtc_get_time() - history.time;
		if (no_data_left)
		{
			current_system_status = SYSTEM_REALTIME;
			is_hold = false;
		}
	}
	if (current_system_status == SYSTEM_REALTIME || current_system_status == SYSTEM_POWERSAVE)
	{
		need_calculate = true;
		if (need_recalculate)
		{
			need_recalculate = false;
			algorithm_calculate(dust_info,2,true);
		}
		
		if (is_hold && !last_hold && sub_function)
		{
			history_t his_tmp = {.time = rtc_get_time(), .data = data2pm25(dust_info,false,false)};
			history_write(&his_tmp);
		}
	}
	
	if (pre_power_off_left != 0)
	{
		pre_power_off_left --;
		if (pre_power_off_left == 0)
		{
			oled_updata_index = circle_index & CIRCLE_OLED_UPDATE_MASK;
		}
	}
	
	if (((circle_index & CIRCLE_OLED_UPDATE_MASK)==oled_updata_index))
	{
		oled_status.is_charge = is_charging;
		if (pre_power_off_left!=0)
		{
			oled_status.show_low_power = true;
			battery_level = 0;
		}
		else
		{
			oled_status.show_low_power = false;
			
		}
		if (current_system_status == SYSTEM_HISTORY)
		{
			oled_status.pm2_5 = history.data;
		}
		else
		{
			//oled_status.pm2_5 = dust_info[0].dust_value>>2;
			//oled_status.pm2_5 = min(99999UL,dust_info[0].tmp_value);
			oled_status.pm2_5 = data2pm25(dust_info,true,false);	//2015 04 01
			oled_status.pm10 = min(99999,oled_status.pm2_5*1.3+data2pm25(dust_info+1,false,false));
		}
		
		
		oled_status.aqi_level = get_aqi_level(oled_status.pm2_5);
		oled_status.battery_level = battery_level;
		oled_status.is_hold = is_hold;
		oled_status.system_status = current_system_status;
		//oled_status.pm2_5_times_100 = rtc_get_time();
		//device_disable(DEVICE_LASER_ENABLE);
		//device_enable(DEVICE_LASER_ENABLE);
		//delay_us(10);//revise the laser poweron Waveform effect
		//
		//device_disable(DEVICE_LASER_ENABLE);
		oled_update(&oled_status);
		//device_disable(DEVICE_LASER_ENABLE);
		//device_enable(DEVICE_LASE	R_ENABLE);
		//delay_us(10);//revise the laser poweron Waveform effect
		//
		//device_disable(DEVICE_LASER_ENABLE);
		
	}
	else if (((circle_index+1) & CIRCLE_OLED_UPDATE_MASK) == oled_updata_index)
	{
		
		if (current_system_status != SYSTEM_POWERDOFF )
		{
			device_disable(DEVICE_LASER_ENABLE);
			algorithm_calibrate();
		}
	}
	else
	{
		if(!is_hold && need_calculate)
		{
			device_enable(DEVICE_AVCC_3V_POWER_ENABLE);
			device_enable(DEVICE_FAN_ENABLE);
			device_enable(DEVICE_LASER_ENABLE);
			//device_disable(DEVICE_LASER_ENABLE);
			delay_us(200);//revise the laser poweron Waveform effect
			algorithm_calculate(dust_info,2,need_recalculate);
			
		}
	}
	if (!(!is_hold && need_calculate))
	{
		device_disable(DEVICE_FAN_ENABLE);
		device_disable(DEVICE_LASER_ENABLE);
		device_disable(DEVICE_AVCC_3V_POWER_ENABLE);
	}
	if ((!is_hold)&&(current_system_status == SYSTEM_REALTIME ||current_system_status == SYSTEM_POWERSAVE))
	{
		device_enable(DEVICE_FAN_ENABLE);
		device_enable(DEVICE_AVCC_3V_POWER_ENABLE);
	}
	
	if (current_system_status == SYSTEM_POWERDOFF)
	{
		
		need_recalculate = true;
		//		UARTD0_init();
	}
	if (current_system_status == SYSTEM_POWERDOFF && !is_charging && pre_power_off_left==0)
	{
		device_disable(DEVICE_VCC_12V_POWER_ENABLE);
		oled_disable();
		if (device_is_high(DEVICE_KEY_POWER))
		{
			PORTE.INTFLAGS &= PIN0_bm;
			pmic_enable_level(PMIC_LVL_HIGH);
			g_main_unlock_step = LOCK_CALIBRATION;
			sleep_enable();
			sleep_enter();
			
			pmic_disable_level(PMIC_LVL_HIGH);
		}
	}
}

ISR(PORTE_INT0_vect)
{
	sleep_disable();
	g_power_key_pointer->downlast = 0;
	g_power_key_pointer->uplast = KEY_CLICK_TIME;
	// 	device_enable(DEVICE_AVCC_3V_POWER_ENABLE);
	// 	delay_ms(10);
	// 	algorithm_init();
	// 	device_disable(DEVICE_AVCC_3V_POWER_ENABLE);
}

ISR(PORTD_INT0_vect)
{
	sleep_disable();
}


ISR(USARTD0_RXC_vect)
{
	uint8_t data;
	usart_serial_getchar(usart2,&data);
	//Analyse_Receive_Char(data);
	//testdata=data;
	Analyse_Receive_Char(data);

}
#if 0
#include "util/delay.h"

static inline uint16_t battery_get_level_uncharging(void)
{
	static uint16_t last_ad_resault = 2000;
	const static uint16_t level_map[] = {1326,1361,1378,1389,1406,1423,1457,1472};
	uint16_t i;
	uint16_t current_resault;
	
	ad_swap_channel(ADCCH_BATTERY);
	current_resault = ad_read();
	adc_disable(&ADCA);
	
	//if (!device_is_high(DEVICE_FAN_ENABLE))
	//{
	//current_resault -= (current_resault>>8)*5;
	//}
	////ad_swap_channel(ADCCH_BATTERY);
	//last_ad_resault = (current_resault + ((uint32_t)last_ad_resault)*31)>>5;
	//for (i=0;i<sizeof(level_map)/sizeof(uint16_t);i++)
	//{
	//if (last_ad_resault < level_map[i])
	//{
	//break;
	//}
	//}
	return current_resault;
}

void main(void )
{
	sysclk_init();
	rtc_init();
	rtc_set_time(history_resume_rtc());
	rtc_set_callback(rtc_callback);
	rtc_set_alarm_relative(10);
	pmic_enable_level(CONFIG_RTC_OVERFLOW_INT_LEVEL);
	circle_tc_init();
	device_init();
	ad_init();
	oled_init();
	
	
	device_enable(DEVICE_AVCC_3V_POWER_ENABLE);
	device_enable(DEVICE_ANALOG_ENABLE);
	device_enable(DEVICE_FAN_ENABLE);
	device_enable(DEVICE_LASER_ENABLE);
	device_enable(DEVICE_VCC_12V_POWER_ENABLE);
	
	oled_enable();
	_delay_ms(2000);
	oled_status_t ost;
	ost.system_status = SYSTEM_REALTIME;
	ost.aqi_level = 0;
	ost.battery_level = 4;
	ost.is_charge = false;
	ost.is_hold = false;
	ost.pm10 =0;
	ost.show_low_power = false;
	ost.time = 1000;
	while(1)
	{
		ost.pm2_5 = battery_get_level_uncharging();
		oled_update(&ost);
		_delay_ms(1000);
	}
}
#endif

#if 1
int main (void)
{
	sysclk_init();
	rtc_init();
	rtc_set_time(history_resume_rtc());
	rtc_set_callback(rtc_callback);
	rtc_set_alarm_relative(10);
	pmic_enable_level(CONFIG_RTC_OVERFLOW_INT_LEVEL);
	circle_tc_init();
	device_init();
	ad_init();
	UARTD0_init();
	oled_init();
	device_enable(DEVICE_VCC_12V_POWER_ENABLE);
	oled_enable();
	Enable_global_interrupt();
	{
		PORTD.INT0MASK = PIN0_bm;
		PORTD.INTCTRL = 0x03;
		PORTE.INT0MASK = PIN2_bm;
		PORTE.INTCTRL = 0x03;
		PORTE.PIN2CTRL = 0X02;
	}
	sleep_set_mode(SLEEP_SMODE_PSAVE_gc);
	
	
	// 	device_enable(DEVICE_AVCC_3V_POWER_ENABLE);
	// 	delay_ms(10);
	// 	algorithm_init();
	// 	device_disable(DEVICE_AVCC_3V_POWER_ENABLE);
	algorithm_init();
	
	while(true)
	{
		if (!is_time_for_new_circle())
		{
			continue;
		}
		tc_clear_overflow(&CIRCLE_TC);
		circle_callback();
	}
	// Insert application code here, after the board has been initialized.
}
#endif