/*
 * usb_dev_serial.c
 *
 *  Created on: 27/01/2013 Updated 03/2016
 *      Author: Jose Manuel Cano, Ignacio Herrero, Eva Gonzalez
 *
 *      Este modulo realiza la integracion de la biblioteca USB del TIVAWAre co FreeRTOS
 *      Su API es parecida a la que gestiona el puerto serie desde el FreeRTOS
 *      El codigo esta basado en el ejemplo usb_dev_serial.c para el TM4C123GXL
 * 		Que se referencia mas abajo
 */

//*****************************************************************************
//
// usb_dev_serial.c - Main routines for the USB CDC serial example.
//
// Copyright (c) 2012 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 9453 of the EK-LM4F120XL Firmware Package.
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>

#include <usblib/usblib.h>
#include <usblib/usb-ids.h>
#include <usblib/usbcdc.h>
#include <usblib/device/usbdevice.h>
#include <usblib/device/usbdcdc.h>



#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_uart.h"
#include "inc/hw_sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "driverlib/usb.h"
#include "driverlib/rom.h"
#include "usb_serial_structs.h"
#include "utils/uartstdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>USB Serial Device (usb_dev_serial)</h1>
//!
//!
//! Assuming you installed StellarisWare in the default directory, a
//! driver information (INF) file for use with Windows XP, Windows Vista and
//! Windows7 can be found in C:/StellarisWare/windows_drivers. For Windows
//! 2000, the required INF file is in C:/StellarisWare/windows_drivers/win2K.
//
//*****************************************************************************

//*****************************************************************************
//
// Note:
//
// This example is intended to run on Stellaris evaluation kit hardware
// where the UARTs are wired solely for TX and RX, and do not have GPIOs
// connected to act as handshake signals.  As a result, this example mimics
// the case where communication is always possible.  It reports DSR, DCD
// and CTS as high to ensure that the USB host recognizes that data can be
// sent and merely ignores the host's requested DTR and RTS states.  "TODO"
// comments in the code indicate where code would be required to add support
// for real handshakes.
//
//*****************************************************************************

//*****************************************************************************
//
// Configuration and tuning parameters.
//
//*****************************************************************************

//*****************************************************************************
//
// The system tick rate expressed both as ticks per second and a millisecond
// period.
//
//*****************************************************************************
#define SYSTICKS_PER_SECOND 100
#define SYSTICK_PERIOD_MS (1000 / SYSTICKS_PER_SECOND)

//*****************************************************************************
//
// Variables tracking transmit and receive counts.
//
//*****************************************************************************
volatile unsigned long g_ulUARTTxCount = 0;

volatile unsigned long g_ulUARTRxCount = 0;
#ifdef DEBUG
unsigned long g_ulUARTRxErrors = 0;
#endif

//*****************************************************************************
//
// Variables globales
//
//*****************************************************************************

static volatile bool g_bUSBConfigured = false;


// Etsit: colas del freertos...
/* The queue used to hold received characters. */
static xQueueHandle xRxedChars;

/* The queue used to hold characters waiting transmission. */
static xQueueHandle xCharsForTx;

//*****************************************************************************
//
// Take as many bytes from the transmit buffer as we have space for and move
// them into the USB UART's transmit FIFO.
//
//*****************************************************************************
static portBASE_TYPE
USBSendToRxQueue(void )
{
    uint32_t ulRead;
    uint8_t ucChar;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;


    //
    // If there is space in the Message Queue, try to read some characters
    // from the receive buffer to fill it again.
    //
    while(!xQueueIsQueueFullFromISR(xRxedChars))
    {
        //
        // Get a character from the buffer.
        //
        ulRead = USBBufferRead((tUSBBuffer *)&g_sRxBuffer, &ucChar, 1);

        //
        // Did we get a character?
        //
        if(ulRead)
        {
            //
            // Etsit: Enviar a la cola de recepcion
            //
    		xQueueSendFromISR( xRxedChars, &ucChar, &xHigherPriorityTaskWoken );

            //
            // Update our count of bytes transmitted via the UART.
            //
            g_ulUARTTxCount++;
        }
        else
        {
            //
            // We ran out of characters so exit the function.
            //
            return xHigherPriorityTaskWoken;
        }
    }
    return xHigherPriorityTaskWoken;
}


static tLineCoding ConfigLinea={
		 .ui32Rate=9600,
		 .ui8Stop=1,
		 .ui8Parity=USB_CDC_PARITY_NONE,
		 .ui8Databits=8,
};
//Esto es para recordar la configuracion. Si no, da errores en algunos dispositivos...

//*****************************************************************************
//
// Handles CDC driver notifications related to control and setup of the device.
//
// \param pvCBData is the client-supplied callback pointer for this channel.
// \param ulEvent identifies the event we are being notified about.
// \param ulMsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to perform control-related
// operations on behalf of the USB host.  These functions include setting
// and querying the serial communication parameters, setting handshake line
// states and sending break conditions.
//
// \return The return value is event-specific.
//
//*****************************************************************************

uint32_t USBControlHandler(void *pvCBData, uint32_t ui32Event,
                        uint32_t ui32MsgValue, void *pvMsgData)
{
    //unsigned long ulIntsOff;

    //
    // Which event are we being asked to process?
    //
    switch(ui32Event)
    {
        //
        // We are connected to a host and communication is now possible.
        //
        case USB_EVENT_CONNECTED:
            g_bUSBConfigured = true;

            //
            // Flush our buffers.
            //
            USBBufferFlush(&g_sTxBuffer);
            USBBufferFlush(&g_sRxBuffer);
            break;

        //
        // The host has disconnected.
        //
        case USB_EVENT_DISCONNECTED:
            g_bUSBConfigured = false;
            break;

        //
        // Return the current serial communication parameters.
        //
        case USBD_CDC_EVENT_GET_LINE_CODING:
            //GetLineCoding(pvMsgData);
        	memcpy(pvMsgData,&ConfigLinea,sizeof(ConfigLinea));//Como es un puerto emulado, respondo con la configuraci�n memorizada.
            break;

        //
        // Set the current serial communication parameters.
        //
        case USBD_CDC_EVENT_SET_LINE_CODING:
            //SetLineCoding(pvMsgData);
        	ConfigLinea=*((tLineCoding *)pvMsgData);	//Como es un puerto emulado, memorizo la configuracion por si me preguntan...
            break;

        //
        // Set the current serial communication parameters.
        //
        case USBD_CDC_EVENT_SET_CONTROL_LINE_STATE:
            //SetControlLineState((unsigned short)ui32MsgValue);
            break;

        //
        // Send a break condition on the serial line.
        //
        case USBD_CDC_EVENT_SEND_BREAK:
            //SendBreak(true);
            break;

        //
        // Clear the break condition on the serial line.
        //
        case USBD_CDC_EVENT_CLEAR_BREAK:
            //SendBreak(false);
            break;

        //
        // Ignore SUSPEND and RESUME for now.
        //
        case USB_EVENT_SUSPEND:
        case USB_EVENT_RESUME:
            break;

        //
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        //
        default:
#ifdef DEBUG
            while(1);
#else
            break;
#endif

    }

    return(0);
}

//*****************************************************************************
//
// Handles CDC driver notifications related to the transmit channel (data to
// the USB host).
//
// \param ulCBData is the client-supplied callback pointer for this channel.
// \param ulEvent identifies the event we are being notified about.
// \param ulMsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to notify us of any events
// related to operation of the transmit data channel (the IN channel carrying
// data to the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************

uint32_t USBTxHandler(void *pvCBData, uint32_t ui32Event,
                   uint32_t ui32MsgValue, void *pvMsgData)
{
	uint8_t cChar;
	uint32_t ulSpace;
	portBASE_TYPE xTaskWoken = pdFALSE;
    //
    // Which event have we been sent?
    //
    switch(ui32Event)
    {
        case USB_EVENT_TX_COMPLETE:
            //
            // Since we are using the USBBuffer, we don't need to do anything
            // here. etsit:�cambiar para adaptar? --> �Leer datos de la fifo?
            //
        	ulSpace = USBBufferSpaceAvailable((tUSBBuffer *)&g_sTxBuffer);

        	while(ulSpace--)
        	{
        		if (xQueueReceiveFromISR( xCharsForTx, &cChar, &xTaskWoken ) == pdTRUE)
        			USBBufferWrite((tUSBBuffer *)&g_sTxBuffer,(unsigned char *)&cChar, 1);
        		else
        			break;
        	}

            break;

        //
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        //
        default:
#ifdef DEBUG
            while(1);
#else
            break;
#endif

    }

    portEND_SWITCHING_ISR( xTaskWoken );
    return(0);
}

//*****************************************************************************
//
// Handles CDC driver notifications related to the receive channel (data from
// the USB host).
//
// \param ulCBData is the client-supplied callback data value for this channel.
// \param ulEvent identifies the event we are being notified about.
// \param ulMsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to notify us of any events
// related to operation of the receive data channel (the OUT channel carrying
// data from the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************

uint32_t USBRxHandler(void *pvCBData, uint32_t ui32Event,
                   uint32_t ui32MsgValue, void *pvMsgData)
{
    uint32_t ulCount;
    portBASE_TYPE xTaskWoken = pdFALSE;
    //
    // Which event are we being sent?
    //
    switch(ui32Event)
    {
        //
        // A new packet has been received.
        //
        case USB_EVENT_RX_AVAILABLE:
        {
            //
            // Feed some characters into the UART TX FIFO and enable the
            // interrupt so we are told when there is more space.
            //
        	xTaskWoken=USBSendToRxQueue();
            //etsit: Cambiar!! --> Mandar a tarea??
            //ROM_UARTIntEnable(USB_UART_BASE, UART_INT_TX);
            break;
        }

        //
        // We are being asked how much unprocessed data we have still to
        // process. We return 0 if the UART is currently idle or 1 if it is
        // in the process of transmitting something. The actual number of
        // bytes in the UART FIFO is not important here, merely whether or
        // not everything previously sent to us has been transmitted.
        //
        case USB_EVENT_DATA_REMAINING:
        {
            //
            // Get the number of bytes in the buffer and add 1 if some data
            // still has to clear the transmitter.
            //
            ulCount = (xQueueIsQueueFullFromISR(xRxedChars)) ? 1 : 0; //etsit: cambiar!
            return(ulCount);
        }

        //
        // We are being asked to provide a buffer into which the next packet
        // can be read. We do not support this mode of receiving data so let
        // the driver know by returning 0. The CDC driver should not be sending
        // this message but this is included just for illustration and
        // completeness.
        //
        case USB_EVENT_REQUEST_BUFFER:
        {
            return(0);
        }

        //
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        //
        default:
#ifdef DEBUG
            while(1);
#else
            break;
#endif
    }

    portEND_SWITCHING_ISR( xTaskWoken );	//Da igual que este no sea exactamente el final de la ISR, porque se genera por excepcion de menor prioridad
    return(0);
}

//*****************************************************************************
//
// This is the main application entry function.
//
//*****************************************************************************
void USBSerialInit(  portBASE_TYPE txQueueLength,  portBASE_TYPE rxQueueLength)
{

	portENTER_CRITICAL();
	{



    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    ROM_GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_5 | GPIO_PIN_4);


    //
    // Not configured initially.
    //
    g_bUSBConfigured = false;


    //
    // Initialize the transmit and receive buffers.
    //
    USBBufferInit((tUSBBuffer *)&g_sTxBuffer);
    USBBufferInit((tUSBBuffer *)&g_sRxBuffer);

    //
    // Set the USB stack mode to Device mode with VBUS monitoring.
    //
    USBStackModeSet(0, eUSBModeForceDevice, 0);

    //
    // Pass our device information to the USB library and place the device
    // on the bus.
    //
    USBDCDCInit(0, (tUSBDCDCDevice *)&g_sCDCDevice);

    ROM_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_USB0);	//USB debe seguir funcionando durante el bajo consumo
    ROM_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOD);	//USB debe seguir funcionando durante el bajo consumo
    ROM_IntPrioritySet(INT_USB0, configMAX_SYSCALL_INTERRUPT_PRIORITY);	//Maximo nivel de prioridad que puede usas API RTOS

    // Crea las colas de transmision y recepcion....
    xRxedChars = xQueueCreate( rxQueueLength, ( unsigned portBASE_TYPE ) sizeof( signed portCHAR ) );
    xCharsForTx = xQueueCreate( txQueueLength, ( unsigned portBASE_TYPE ) sizeof( signed portCHAR ) );

    //Comprueba que se hayan creado antes de continuar...
    configASSERT((xRxedChars!=NULL)&&(xCharsForTx!=NULL));

	}
    portEXIT_CRITICAL();
}


portBASE_TYPE USBSerialGetChar( portCHAR *pcRxedChar, portTickType xBlockTime )
{
	/* Get the next character from the buffer.  Return false if no characters
	are available, or arrive before xBlockTime expires. */
	return  xQueueReceive( xRxedChars, pcRxedChar, xBlockTime );

}
/*-----------------------------------------------------------*/

portBASE_TYPE USBSerialPutChar( portCHAR cOutChar, portTickType xBlockTime )
{
	portBASE_TYPE xReturn=0;
    uint32_t ulSpace;

	/* Send the next character to the queue of characters waiting transmission,
	then enable the UART Tx interrupt, just in case UART transmission has already
	completed and switched itself off. */

	//Seria mas eficiente con un semaforo??
    //Esto mira si hay espacio en el buffer USB, si no, manda a la cola.

	xReturn=pdTRUE;

	ulSpace = USBBufferSpaceAvailable((tUSBBuffer *)&g_sTxBuffer);

	if (ulSpace)
	{

		USBBufferWrite((tUSBBuffer *)&g_sTxBuffer,(unsigned char *)&cOutChar, 1);
	}

	else
	{
		xReturn = xQueueSend( xCharsForTx, &cOutChar, xBlockTime );
	}

	return xReturn;
}

int32_t USBSerialRead( uint8_t *buffer, int32_t size, portTickType xBlockTime )
{
	int32_t i;
	/* Get the next character from the buffer.  Return false if no characters
	are available, or arrive before xBlockTime expires. */
	for(i=0;i<size;i++)
	{
		if (!xQueueReceive( xRxedChars, buffer, xBlockTime ))
		{
			break; //If queue empty after timeout, stop
		}
		buffer++;
	}
	return i; //Return how many bytes were actually read
}
/*-----------------------------------------------------------*/


/* Si la funcion va a utilizarse desde varias tareas DEBERIA ser protegida con un Mutex */
int32_t USBSerialWrite( uint8_t *buffer, int32_t size, portTickType xBlockTime )
{
	int32_t i;
	uint32_t ulSpace;


	ulSpace = USBBufferSpaceAvailable((tUSBBuffer *)&g_sTxBuffer);

	if (ulSpace>=size)
	{	/* Send Everything to USB Buffer */
		USBBufferWrite((tUSBBuffer *)&g_sTxBuffer,buffer, size);
		return size; //Return how many bytes were writen
	}
	else
	{
		/* Fill the USB buffer */
		USBBufferWrite((tUSBBuffer *)&g_sTxBuffer,buffer,ulSpace);

		/* Send the remaining data to the Tx queue */
		buffer+=ulSpace;
		for(i=ulSpace;i<size;i++)
		{
			if(!xQueueSend( xCharsForTx, buffer, xBlockTime ))
			{
				break;
			}
			buffer++;
		}

	}

	return i; //Return how many bytes were actually writen
}












