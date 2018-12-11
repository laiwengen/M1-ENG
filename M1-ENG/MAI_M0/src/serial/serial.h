/*
 * serial_comm.h
 *
 * Created: 2014/10/17
 *  Author: Liujing
 */ 

#ifndef SERIAL_H_
#define SERIAL_H_

#include "serial/serial.h"
#include "usart.h"
#include "asf.h"
#include "string.h"
#include "avr/eeprom.h"
#include "math.h"
//#include "hcho/hcho.h"
#include "oled/oled.h"
#define HCHO_LOW_ADDRESS 0x210
#define PM25_ADDRESS 0x230

#define START_CODE	'S'					//串口通信起始码
#define END_CODE	'E'					//串口通信结束码

//#define HCHO_CALIBRATION_CODE	'N'		//甲醛高值记录(同时记录(电压,浓度),参与动态修正
#define PM25_CALIBRATION_CODE	'M'		//PM25数值修正倍数记录(传入的为标准值)

#define COMMAND_POWERON		'P'			//模拟开关机键
#define PM25_UNLOCK_CODE		'L'			//模拟HOLD键单击
#define COMMAND_SWITCH		'W'			//模拟切换键
//
//#define COMMAND_HOLDPOWER	'H'			//保持屏幕常亮(关机后恢复
//
//#define COMMAND_CHECK_HCHO	'n'			//检查主机甲醛数据与当前数据是否符合一定公差
#define PM25_CHECK_CODE	'm'			//检查主机PM25数据与当前数据是否符合一定公差
#define PM25_ERASE_CODE	'R'			//擦除标定数据
//#define COMMAND_HCHO_CALIBRATION_RESET 'R'	//消除所有甲醛eeprom标定数据 返回初始状态
#define SIZE_FOR_UART_RX 4
enum g_unlock_step
{
	LOCK_CALIBRATION,
	UNLOCK_CALIBRATION,
	READY_TO_CALIBRATION

};
typedef struct
{
	//bool is_received;
	uint32_t dataReceived;
	uint8_t  CommandType;
//	uint32_t lowVoltage;
//	uint32_t highVoltage;
}RevGasData_t;
RevGasData_t g_RevGasData;
extern bool IsAlwaysPowerOn;
uint8_t CheckUART2Rev(RevGasData_t *RevGasDataPointer);
//void UARTD0_init(void);
uint8_t Analyse_Receive_Char(uint8_t c1);
bool DealUART2Data(RevGasData_t *RevGasDataPointer,uint32_t ValueToRecord);
void WriteGasParameter(RevGasData_t *RevGasDataPointer,uint32_t ValueToRecord);

bool CheckCalibration(RevGasData_t *RevGasDataPointer,uint32_t ValueToDeal);
bool ClearCheckCount(void);
void pm25_get_correction(uint32_t ppm,system_status_t * current_system_status,uint8_t step);
uint32_t pm25_set_correction(uint32_t data);
uint32_t pm252data(uint32_t pm25);
void erasePm25Clibration(void);
#endif