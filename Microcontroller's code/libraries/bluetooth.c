/*
 * bluetooth.c
 *
 *  Created on: 5 abr. 2020
 *      Author: borja
 */
#include <FreeRTOS.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "queue.h"
#include "semphr.h"
#include "driverlib/uart.h"

#include "system_definitions.h"
#include "bluetooth.h"
#include "serial.h"
#include "kwp2000.h"

extern QueueHandle_t BTQueue,BTISRQueue;
extern QueueSetHandle_t BTQueueSet;
extern SemaphoreHandle_t semKWP;
extern TaskHandle_t kwpTaskHandler;
extern QueueHandle_t KWPQueue;

extern bool OBDIIMode;


static Serial BT;

static void BTHandler(void);

void BTTask (void *pvParameters)
{
    DATA_MESSAGE msg;
    memset(&BT,0,sizeof(Serial));
    BT.baudRate = 115200;
    BT.module = 1;
    BT.RXpin = GPIO_PB0_U1RX;
    BT.TXpin = GPIO_PB1_U1TX;
    BT.usingGPIO = false;
    serial_enable(&BT,'B');
    serial_useAsSerial(&BT,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    serial_enableInts(&BT);
    UARTIntRegister(BT_INT_CLEAR,BTHandler);
    QueueSetMemberHandle_t xActivatedMember;
    while(1)
    {
#if   BT_NT_TESTING
        xActivatedMember = xQueueSelectFromSet(BTQueueSet, portMAX_DELAY);
        if(xActivatedMember == BTISRQueue)
        {
            uint8_t numData = uxQueueMessagesWaiting(BTISRQueue);
            if(numData > 0)
            {
                #if USE_MALLOC
                            char *data = (uint8_t *)calloc(numData+1,sizeof(uint8_t));
                #else
                            char *data = (char *)pvPortMalloc(sizeof(uint8_t)*numData);
                #endif
                            uint8_t i;
                            for(i = 0; i < numData; i++)
                            {
                                uint8_t item;
                                xQueueReceive(BTISRQueue,&item,portMAX_DELAY);
                                data[i] = (uint8_t)item;
                            }
                            data[numData] = '\0';
                            translate(data);
                #if USE_MALLOC
                            free(data);
                #else
                            vPortFree(data);
                #endif
            }
        }

        else if(xActivatedMember == BTQueue)
        {
            xQueueReceive(BTQueue, &msg, 0); //In theory, data should be available.
            uint8_t numData = msg.dataLength;
            uint8_t i;
            for(i = 0; i < numData; i++)
            {
                uint8_t item = (uint8_t)*msg.ptrToData;
                msg.ptrToData++;
                serial_send(&BT,item);
            }
            //Just to help with data visualization.
            i = 0x0D;
            serial_send(&BT,i);
            i = 0x0A;
            serial_send(&BT,i);
        }
#else
        xActivatedMember = xQueueSelectFromSet(BTQueueSet, 100);
        if(xActivatedMember == BTISRQueue)
        {
            uint8_t numData = uxQueueMessagesWaiting(BTISRQueue);
            if(numData > 0)
            {
                #if USE_MALLOC
                            char *data = (uint8_t *)calloc(numData+1,sizeof(uint8_t));
                #else
                            char *data = (char *)pvPortMalloc(sizeof(uint8_t)*numData);
                #endif
                            uint8_t i;
                            for(i = 0; i < numData; i++)
                            {
                                uint8_t item;
                                xQueueReceive(BTISRQueue,&item,portMAX_DELAY);
                                data[i] = (uint8_t)item;
                            }
                            data[numData] = '\0';
                            translate(data);
                #if USE_MALLOC
                            free(data);
                #else
                            vPortFree(data);
                #endif
            }
        }

        else if(xActivatedMember == BTQueue)
        {
            xQueueReceive(BTQueue, &msg, 0); //In theory, data should be available.
            uint8_t numData = msg.dataLength;
            uint8_t i;
            for(i = 0; i < numData; i++)
            {
                uint8_t item = (uint8_t)*msg.ptrToData;
                msg.ptrToData++;
                serial_send(&BT,item);
            }
        }
        else
        {
            uint8_t v;
            uint16_t cs;
            v = (v+1)%0xFF;
            uint8_t data[9];
            data[0] = 0x82;
            data[1] = 0xF1;
            data[2] = 0x10;
            data[3] = 0x41;
            data[4] = 0x0D;
            data[5] = v;
            cs = data[0] + data[1] + data[2];
            cs += data[3] + data[4] + data[5];
            data[6] = cs & 0x00FF;
            data[7] = 0x0D;
            data[8] = 0x0A;
            //no checksum, but it is not necessary for this.
            uint8_t i;
            for(i = 0; i < 8; i++)
            {
                serial_send(&BT,data[i]);
            }
        }
#endif
    }
}

static void BTHandler (void)
{
    portBASE_TYPE xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    uint32_t ui32Ints;

    ui32Ints = MAP_UARTIntStatus(BT_INT_CLEAR, true);
    MAP_UARTIntClear(BT_INT_CLEAR, ui32Ints);


    if(ui32Ints & UART_INT_RX)
    {
        while(MAP_UARTCharsAvail(BT_INT_CLEAR))
        {
               uint32_t received = UARTCharGetNonBlocking(BT_INT_CLEAR);
               uint8_t item = (uint8_t)(received & 0xFF);
               xQueueSendFromISR(BTISRQueue,&item,&xHigherPriorityTaskWoken);
        }
    }

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

static void translate(char *msg)
{
    if(strncmp(msg,"RHY",2)==0)
    {
        OBDIIMode = false;
        xSemaphoreGive(semKWP);
    }
    else if(strncmp(msg,"OBD",2)==0)
    {
        OBDIIMode = true;
        xSemaphoreGive(semKWP);
    }
    else if(strncmp(msg,"STOP",2)==0)
    {
        vTaskDelete(kwpTaskHandler);
    }
    
    if(strncmp(msg,"CLEAR",2)==0)
    {
        //sID 04
        uint8_t i, item;
        for(i = 0; i < sizeof(clearDTC_Message); i++)
        {
            item = clearDTC_Message[i];
            xQueueSend(KWPQueue, &item, portMAX_DELAY);
        }
    }
    else if(strncmp(msg,"READ",2)==0)
    {
        //sID 03
        uint8_t i, item;
        for(i = 0; i < sizeof(readDTC_Message); i++)
        {
            item = readDTC_Message[i];
            xQueueSend(KWPQueue, &item, portMAX_DELAY);
        }
    }

}
