/*
 * serial.c
 *
 *  Created on: 5 April. 2020
 *      Author: Borja
 */
#include <FreeRTOS.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <driverlib/uart.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"

#include "serial.h"

void serial_enable(Serial *s, uint8_t gpio)
{

    s->module_base_addr = UART0_BASE + (s->module*0x1000);
    s->module_periph = SYSCTL_PERIPH_UART0 + s->module;
    if ( (gpio >= 'A') && (gpio < 'G'))
    {
        if(gpio < 'E')
        {
            s->gpio_addr = GPIO_PORTA_BASE + (gpio-'A')*0x1000;
            s->gpio_periph = SYSCTL_PERIPH_GPIOA + (gpio-'A');
        }
        else if (gpio >= 'E')
        {
            s->gpio_addr = GPIO_PORTE_BASE + (gpio-'A')*0x1000;
            s->gpio_periph = SYSCTL_PERIPH_GPIOA + (gpio-'A');
        }
    }

    switch(s->module)
    {
    case 1:
    case 7:
        s->pin1 = GPIO_PIN_0; s->pin2 = GPIO_PIN_1;
        break;
    case 2:
    case 3:
        s->pin1 = GPIO_PIN_6; s->pin2 = GPIO_PIN_7;
        break;
    case 4:
    case 5:
    case 6:
        s->pin1 = GPIO_PIN_4; s->pin2 = GPIO_PIN_5;
        break;
    }
    SysCtlPeripheralEnable(s->module_periph);
    SysCtlPeripheralEnable(s->gpio_periph);
    SysCtlPeripheralSleepEnable(s->module_periph);
    SysCtlPeripheralSleepEnable(s->gpio_periph);
}

void serial_useAsGPIO(Serial *s)
{
	GPIOPinTypeGPIOOutput(s->gpio_addr,s->pin2);
	s->usingGPIO = true;
}

void serial_useAsSerial(Serial *s, uint32_t config)
{
	GPIOPinTypeUART(s->gpio_addr, (s->pin1) | (s->pin2));
    GPIOPinConfigure(s->TXpin);
    GPIOPinConfigure(s->RXpin);

    //UARTFIFOEnable(s->module_base_addr);
    //UARTFIFODisable(s->module_base_addr);
    UARTFIFOLevelSet(s->module_base_addr,UART_FIFO_TX1_8,UART_FIFO_RX1_8);
    UARTConfigSetExpClk(s->module_base_addr,SysCtlClockGet(),s->baudRate,
                        config);
    GPIOPinTypeUART(s->gpio_addr,(s->pin1) | (s->pin2));
    s->usingGPIO = false;
}

void serial_enableInts(Serial *s)
{
	//Enables HW interrupts. Do not forget to assign a proper ISR (Interrupt Service Routine) either "statically" or "dynamically".
    // C-like Interrupts clears.
    UARTFIFOLevelSet(s->module_base_addr, UART_FIFO_TX1_8, UART_FIFO_RX1_8);
    UARTIntEnable(s->module_base_addr, UART_INT_RX);
    uint32_t which;
    switch(s->module)
    {
    case 0:
        which = INT_UART0;
        break;
    case 1:
        which = INT_UART1;
        break;
    case 2:
        which = INT_UART2;
        break;
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        which = (s->module-3)+INT_UART3;
        break;
    }
    IntPrioritySet(which,configMAX_SYSCALL_INTERRUPT_PRIORITY);
    IntEnable(which);
}

void serial_disableInts(Serial *s)
{
    //Disables HW interrupts.
    UARTIntDisable(s->module_base_addr, UART_INT_RX);
}

void serial_send(Serial *s, unsigned char data)
{
    // Sends 8 bits of data, despite the fact the GPIO is being used or the UART.
    // In case GPIO output is being used instead of UART's, it will only place a '1' or
    // '0', as expected.
    if(!(s->usingGPIO))
        UARTCharPut(s->module_base_addr,data);
    else
    {
        if(data != 0)
            GPIOPinWrite(s->gpio_addr,s->pin2,s->pin2);
        else
            GPIOPinWrite(s->gpio_addr,s->pin2,0);
    }

}

unsigned char serial_receive(Serial *s)
{
    // Returns current UART input data registers value or one GPIO pin value.

	if(!(s->usingGPIO))
        return UARTCharGet(s->module_base_addr);
    else
        return ( (s->pin1) & GPIOPinRead(s->gpio_addr,s->pin1));
}

unsigned char serial_receiveNB(Serial *s)
{
    // Returns current UART FIFO data, -1 if empty.
    // It does not make sense to use this function with GPIO.
    // But it is programmer's choice.
    if(!(s->usingGPIO))
        return UARTCharGetNonBlocking(s->module_base_addr);
    else
        return ( (s->pin1) & GPIOPinRead(s->gpio_addr,s->pin1));
}

