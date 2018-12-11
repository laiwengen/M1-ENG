/*
 * debug.c
 *
 * Created: 2014/3/6 9:54:44
 *  Author: Airj
 */ 
#include "debug.h"
#include "conf_debug.h"
#include "stdio.h"


uint16_t g_circleindex = 0;
uint16_t g_lastcmdindex = 0;
bool g_linestartsent = false;

uint8_t g_buffer_for_debug_fifo[RX_BUFFER_SIZE] = {0};//usart2
fifo_desc_t fifo_desc_debug;

void debug_init(void)
{
	fifo_init(&fifo_desc_debug,g_buffer_for_debug_fifo,RX_BUFFER_SIZE);
	{
		usart_rs232_options_t usart_options = {.baudrate = 115200L,.charlength = USART_CHSIZE_8BIT_gc,.paritytype=USART_PMODE_DISABLED_gc,.stopbits = false};
		usart_serial_init(&DEBUG_USART,&usart_options);
		ioport_set_pin_dir(DEBUG_TXD,IOPORT_DIR_OUTPUT);
		ioport_set_pin_dir(DEBUG_RXD,IOPORT_DIR_INPUT);
		usart_set_rx_interrupt_level(&USARTD0,USART_INT_LVL_HI);
	}
}
void debug_print_string(const char *str)
{
	uint8_t package_size = 0;
	while(*(str+package_size)!='\0')
	{
		package_size++;
	}
	usart_serial_write_packet(&DEBUG_USART, str, package_size);
	g_lastcmdindex = g_circleindex;
	g_linestartsent = false;
}

void debug_print_uint16(uint16_t value)
{
	char s[4] = {0};
	char map[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	s[0] = map[(value>>12)&0xf];
	s[1] = map[(value>>8)&0xf];
	s[2] = map[(value>>4)&0xf];
	s[3] = map[value&0xf];
	usart_serial_write_packet(&DEBUG_USART, s, 4);
}

void debug_print_bytes(const uint8_t * bytes, const uint8_t len)
{
	usart_serial_write_packet(&DEBUG_USART, bytes, len);
	g_lastcmdindex = g_circleindex;
	g_linestartsent = false;
}


uint8_t debug_read_rx_buffer(uint8_t * data, uint8_t maxlen)
{
	uint8_t current_len = 0;
	uint8_t tmp;
	while(maxlen-->0 && fifo_pull_uint8(&fifo_desc_debug,&tmp) == FIFO_OK)
	{
		*(data+current_len) = tmp;
		current_len++;
	}
	return current_len;
}

bool debug_rx_no_data(void)
{
	return fifo_is_empty(&fifo_desc_debug);
}

uint8_t debug_has_new_cmd(uint8_t stop)
{
	uint8_t i;
	for (i=0; i<fifo_get_used_size(&fifo_desc_debug); i++)
	{
		if (fifo_desc_debug.buffer.u8ptr[(fifo_desc_debug.read_index + i) & (fifo_desc_debug.mask >> 1)] == stop)
		{
			return i+1;
		}
	}
	return 0;
}

void debug_next_circle(void)
{
	g_circleindex ++;
	if (g_linestartsent == false)
	{
		if (g_circleindex - g_lastcmdindex > 3)
		{
			usart_serial_putchar(&DEBUG_USART,'>');
			g_linestartsent = true;
		}
	}
}


ISR(USARTD0_RXC_vect)
{
	uint8_t data;
	usart_serial_getchar(&USARTD0,&data);
	if (data == 0x08)
	{
		if (!fifo_is_empty(&fifo_desc_debug))
		{
			fifo_desc_debug.write_index  = (fifo_desc_debug.write_index - 1) & fifo_desc_debug.mask;
			usart_serial_putchar(&USARTD0,data);
		}
	}
	else
	{
		fifo_push_uint8(&fifo_desc_debug, data);
		usart_serial_putchar(&USARTD0,data);
	}
	//g510_rx_queue_put(data);
}
