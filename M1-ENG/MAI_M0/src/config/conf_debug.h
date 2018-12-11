/*
 * conf_debug.h
 *
 * Created: 2014/3/6 9:58:46
 *  Author: Airj
 */ 


#ifndef CONF_DEBUG_H_
#define CONF_DEBUG_H_

#define DEBUG_PORT PORTD
#define DEBUG_USART USARTD0
#define DEBUG_TXD IOPORT_CREATE_PIN(PORTD,3)
#define DEBUG_RXD IOPORT_CREATE_PIN(PORTD,2)


#endif /* CONF_DEBUG_H_ */