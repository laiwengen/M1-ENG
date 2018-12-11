#include <string.h>
#include "system/system.h"
#include "serial/serial.h"
//#include "hcho/hcho.h"
//#define ANSWER_CORRECT_CODE					0xFF	//返回应答信息码
//#define MAX_SERIAL_BUFF						255		//数据包的最大长度


typedef struct
{
	uint8_t command_type;
	//命令的类型
	uint8_t length;
	//保存收到的数据长度
	uint8_t pointer;
	//当前的保存的位置
	uint8_t step;
	//串口处于的步骤
	//uint8_t crc;
	////整个数据的校验值
	//uint8_t over_time_counter;
	////超时计数器
	uint8_t buff[SIZE_FOR_UART_RX];
	//数据缓冲区
}SERIAL_RECEIVE_STRUCT;
SERIAL_RECEIVE_STRUCT g_receive_struct;

/*串口有关的常量*/
enum SERIAL_RECEIVE_STEP_TYPE
{
	SERIAL_SEARCH_START,				//当前串口处于查找开头码
	SERIAL_RECEIVE_TYPE,				//当前串口处于查找类型码
	SERIAL_RECEIVE_LENGTH,			//当前串口处于接收数据包的长度信息
	SERIAL_RECEIVE_DATA,				//当前串口处于接收数据
	SERIAL_SEARCH_END,				//当前串口处于查找结尾码
	SERIAL_RECEIVE_END				//当前串口数据包结束
};




uint8_t Analyse_Receive_Char(uint8_t c1)
//分析串口收到的数据
{
	uint8_t temp;
	temp=0;
	switch(g_receive_struct.step)
	{
		case SERIAL_SEARCH_START:
		//串口在寻找开头码
		if(c1==START_CODE)
		{
			//g_receive_struct.crc		=	g_receive_struct.crc+c1;
			//g_receive_struct.over_time_counter=0;//to do 启动定时器，做超时计算
			
			g_receive_struct.pointer	=	0;
			g_receive_struct.step		=	SERIAL_RECEIVE_TYPE;
			//已找到开头码,下一步接收类型码
		}
		break;
		
		case SERIAL_RECEIVE_TYPE:
		//第二个字节为类型码
		//g_receive_struct.crc		=	g_receive_struct.crc+c1;
		g_receive_struct.command_type	=	c1;
		//暂存类型码
		g_receive_struct.step 			=	SERIAL_RECEIVE_DATA;
		
		break;
		
		case SERIAL_RECEIVE_DATA:
		g_receive_struct.buff[g_receive_struct.pointer]=c1;
		g_receive_struct.pointer		=	g_receive_struct.pointer+1;
		//g_receive_struct.crc			=	g_receive_struct.crc+c1;
		if(g_receive_struct.pointer	==	SIZE_FOR_UART_RX)
		{
			g_receive_struct.step		=	SERIAL_SEARCH_END;
			
		}
		break;
		case SERIAL_SEARCH_END:
		if (c1==END_CODE)
		{
			g_receive_struct.step		=	SERIAL_RECEIVE_END;
			
			temp =	1;
		}
		else g_receive_struct.step		=	SERIAL_SEARCH_START;
		break;
		case SERIAL_RECEIVE_END:
		
		break;
		
		default:
		g_receive_struct.step			=	SERIAL_SEARCH_START;
		
		break;
	}
	return temp;
}



bool DealUART2Data(RevGasData_t *RevGasDataPointer,uint32_t ValueToDeal)
{
	uint32_t temp;
	uint32_t ValueReCount=0;
	int8_t i;
	
	i=SIZE_FOR_UART_RX-1;
	temp=1;
	for (;i>=0;i--)
	{
		ValueReCount=ValueReCount+(g_receive_struct.buff[i]-'0')*temp;
		temp=temp*10;
	}
	memset(g_receive_struct.buff,0x00,SIZE_FOR_UART_RX);
	if (ValueReCount==0)
	{
		return 0;
	}
	RevGasDataPointer->dataReceived=max(0,ValueReCount);
	//if ((RevGasDataPointer->CommandType == HCHO_CALIBRATION_CODE)||(RevGasDataPointer->CommandType == PM25_CALIBRATION_CODE))
	{
		WriteGasParameter(RevGasDataPointer,ValueToDeal);	//check and write (ppm,voltage) to eeprom
	}
	

	
	//	printf ("Hcho PPM memorized.\n");
	
	return	 1;
}

//static uint32_t withinRangeCount = 0;
//static uint32_t totalCheckCount = 0;
bool CheckCalibration(RevGasData_t *RevGasDataPointer,uint32_t CurrentValue)
{
	uint32_t temp;
	float dataReceived=0;
	int8_t i;
	

	i=SIZE_FOR_UART_RX-1;
	temp=1;
	for (;i>=0;i--)
	{
		dataReceived=dataReceived+(g_receive_struct.buff[i]-'0')*temp;
		temp=temp*10;
	}
	//uint32_t minLimitValue;
	//uint32_t maxLimitValue;
	////totalCheckCount = min(UINT32_MAX/100,totalCheckCount+1);
	//if (CurrentValue<10000)
	//{
	//maxLimitValue = min(99999,dataReceived*10+1000);
	//minLimitValue = max(0,dataReceived*10-1000);
	//}
	//else
	//{
	//minLimitValue = min(99999,dataReceived*9);
	//maxLimitValue = Min(109999,dataReceived*11);
	//}
	dataReceived = dataReceived*10;
	volatile float percent = 1-(dataReceived-CurrentValue)/dataReceived;
	static float lastpercent=0;
	float dealta = percent - lastpercent;
	volatile float percentAfterFC;
	//	 if (CurrentValue<maxLimitValue && CurrentValue > minLimitValue)
	//	{

	percentAfterFC = lastpercent + dealta*0.15;
	lastpercent = percentAfterFC;
	//	withinRangeCount = min(UINT32_MAX/100,withinRangeCount+1);
	
	
	//	}
	volatile uint8_t nLevel = max(1,min(6,abs(round(10-percentAfterFC*10))));
	
	oled_show_check_level(nLevel-1);
	
	return	 1;
}
bool ClearCheckCount(void)
{
	//withinRangeCount = 0;
	//totalCheckCount = 0;
	return	 1;
}
//write low and high ppm to eeprom
/*
hcho address:  HCHO_LOW_ADDRESS-HCHO_LOW_ADDRESS+16
*/
void WriteGasParameter(RevGasData_t *RevGasDataPointer,uint32_t ValueToRecord)
{
	uint32_t data;
	
	switch (RevGasDataPointer->CommandType)
	{
		
		
		case PM25_CALIBRATION_CODE:
		{
			
			data = pm252data(RevGasDataPointer->dataReceived *10);//the passing data only has 4 digits
			eeprom_busy_wait();
			eeprom_write_dword((uint32_t *)PM25_ADDRESS,data);
			data = ValueToRecord;//the passing data only has 4 digits
			
			//cofficient = cofficient *10000 / ValueToRecord;//multiply by 1000(the recording data in program has 5 digits,so *10)
			//cofficient = min (cofficient,2500);
			
			eeprom_busy_wait();
			eeprom_write_dword((uint32_t *)PM25_ADDRESS+1,data);
		}
		break;
		default:
		//invalid code or need not write,return
		return;
		//break;
	}
	
}
void erasePm25Clibration(void)
{
	eeprom_busy_wait();
	eeprom_write_dword((uint32_t *)PM25_ADDRESS,UINT32_MAX);
	
	
	//cofficient = cofficient *10000 / ValueToRecord;//multiply by 1000(the recording data in program has 5 digits,so *10)
	//cofficient = min (cofficient,2500);
	
	eeprom_busy_wait();
	eeprom_write_dword((uint32_t *)PM25_ADDRESS+1,UINT32_MAX);
}
uint8_t CheckUART2Rev(RevGasData_t *RevGasDataPointer)
{//check if a valid data stream transfered and if is ,return command type
	//	printf ("UART2 inited");
	
	if (g_receive_struct.step == SERIAL_RECEIVE_END)
	{
		g_receive_struct.step = SERIAL_SEARCH_START;
		RevGasDataPointer->CommandType = g_receive_struct.command_type;
		oled_inform_check_result();
		return g_receive_struct.command_type;
	}
	else return 0;
	//printf ("%X",0xab);
}

void pm25_get_correction(uint32_t ppm,system_status_t * current_system_status,uint8_t step)
{
	static int32_t pm25SteadyLast = 0;
	static int32_t lastPm25Value = 0;
	int32_t dealta = ppm - lastPm25Value;
	int32_t	pm25ValueAfterFCorrect = 0;
	
	pm25ValueAfterFCorrect = dealta*0.3 + lastPm25Value;
	lastPm25Value = pm25ValueAfterFCorrect;
	
	if (abs(pm25ValueAfterFCorrect-ppm) < Max(2500,pm25ValueAfterFCorrect/5))//
	{
		pm25SteadyLast = min(pm25SteadyLast+1,INT8_MAX-1);
	}
	else
	{
		pm25SteadyLast = max(pm25SteadyLast-2,0);
	}
	if ((g_RevGasData.CommandType == PM25_CALIBRATION_CODE) && pm25SteadyLast>=6 )
	{
		if ((*current_system_status == SYSTEM_REALTIME)&& step == READY_TO_CALIBRATION)	//only take effect when master and slave both in pm25 mode
		{
			DealUART2Data(&g_RevGasData,pm25ValueAfterFCorrect);//write cofficient
		}
		g_RevGasData.CommandType = 0; //clear flag
	}
	if ((g_RevGasData.CommandType == PM25_CHECK_CODE) )
	{

	}
}
uint32_t pm25_set_correction(uint32_t data)
{
	uint32_t masterData,selfCalibrationData;
	double tempData = data;
	double k;
	eeprom_busy_wait();
	masterData = eeprom_read_dword((uint32_t *)PM25_ADDRESS);
	eeprom_busy_wait();
	selfCalibrationData = eeprom_read_dword((uint32_t *)PM25_ADDRESS+1);
	if (masterData == UINT32_MAX || selfCalibrationData == UINT32_MAX)
	{
		return data;
	}	
	k = masterData * 1.0 / selfCalibrationData;
	if (k<0.2 )
	{
		return 20;
	}
	if (k > 3.0)
	{
		return 10;
	}

// 	#if TSI_MODE
// 	tempData = tempData * k ;
// 	#else
	uint32_t hightThreshold = max(pm252data(50000),selfCalibrationData);
	if (tempData>hightThreshold)
	{
		double tempK = (1+k)/2;
		tempData = (tempData- hightThreshold)*tempK + hightThreshold*k;
	}
	else
	{
		tempData = tempData * k ;
	}
	//#endif
	return (uint32_t)tempData;
}



