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

#define START_CODE	'S'					//����ͨ����ʼ��
#define END_CODE	'E'					//����ͨ�Ž�����

//#define HCHO_CALIBRATION_CODE	'N'		//��ȩ��ֵ��¼(ͬʱ��¼(��ѹ,Ũ��),���붯̬����
#define PM25_CALIBRATION_CODE	'M'		//PM25��ֵ����������¼(�����Ϊ��׼ֵ)

#define COMMAND_POWERON		'P'			//ģ�⿪�ػ���
#define PM25_UNLOCK_CODE		'L'			//ģ��HOLD������
#define COMMAND_SWITCH		'W'			//ģ���л���
//
//#define COMMAND_HOLDPOWER	'H'			//������Ļ����(�ػ���ָ�
//
//#define COMMAND_CHECK_HCHO	'n'			//���������ȩ�����뵱ǰ�����Ƿ����һ������
#define PM25_CHECK_CODE	'm'			//�������PM25�����뵱ǰ�����Ƿ����һ������
#define PM25_ERASE_CODE	'R'			//�����궨����
//#define COMMAND_HCHO_CALIBRATION_RESET 'R'	//�������м�ȩeeprom�궨���� ���س�ʼ״̬
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