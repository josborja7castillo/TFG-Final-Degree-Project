/*
 * serial.c
 *
 *  Created on: 5 apr. 2020
 *      Author: Borja
 */

#ifndef GEN_SERIAL_H_
#define GEN_SERIAL_H_

#include <stdint.h>


/*
 * Generic Arduino-like Serial class. It can be used to both perform operations as GPIO and serial.
 * Constructor parameters are the following: "m" indicates which module (0-7) will be used, "br" is the baudrate to be used
 * (only on serial mode), "gpio" is the ASCII letter that belongs to be GPIO port ('A'-'F').
 * "rx_pin" and "tx_pin" will set the pins into UART mode. They must be alike "GPIO_Pportpin_UmoduleTX/RX".
 */


typedef struct{
    uint8_t module;
    uint32_t gpio_addr, gpio_periph;
    uint32_t TXpin, RXpin;
    uint32_t baudRate;
    uint32_t module_base_addr, module_periph;
    uint8_t pin1, pin2;
    bool usingGPIO;
}Serial;

void serial_enable(Serial *s, uint8_t gpio);
void serial_useAsGPIO(Serial *s);
void serial_useAsSerial(Serial *s, uint32_t config);
void serial_enableInts(Serial *s);
void serial_disableInts(Serial *s);
void serial_send(Serial *s, unsigned char data);
unsigned char serial_receive(Serial *s);
unsigned char serial_receiveNB(Serial *s);



#endif /* SERIAL_H_ */
