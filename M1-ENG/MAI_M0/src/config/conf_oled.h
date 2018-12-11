/*
 * conf_oled.h
 *
 * Created: 2014/3/12 15:56:13
 *  Author: Airj
 */ 


#ifndef CONF_OLED_H_
#define CONF_OLED_H_

#define OLED_ENABLE IOPORT_CREATE_PIN(PORTR,1)
#define OLED_DC IOPORT_CREATE_PIN(PORTB,0)
#define OLED_RD IOPORT_CREATE_PIN(PORTB,1)
#define OLED_WR IOPORT_CREATE_PIN(PORTB,2)
#define OLED_RES IOPORT_CREATE_PIN(PORTB,3)
#define OLED_CS IOPORT_CREATE_PIN(PORTA,7)
#define OLED_DATA_PORT IOPORT_PORTC

#define OLED_XMAX


#endif /* CONF_OLED_H_ */