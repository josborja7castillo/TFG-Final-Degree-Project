#include <FreeRTOS.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <driverlib/uart.h>
#include <driverlib/timer.h>
#include <driverlib/gpio.h>
#include "remotelink/remotelink_messages.h"
#include "remotelink/remotelink.h"
#include "queue.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"
#include <utils/uartstdio.h>
#include <driverlib/sysctl.h>
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_uart.h"

#include "kwp2000.h"
#include "system_definitions.h"
#include "serial.h"


#define CHECK_BIT_STAT(var,bitPos) (( (var) >> (bitPos) ) & 1)
#define RESPONSE_TIME (55 / portTICK_PERIOD_MS)
#define INTERBYTE_TIME (20 / portTICK_PERIOD_MS)


#define TRANSMITTING 0
#define RECEIVING    1
#define IDLE 2

static uint8_t buffer_index,pendToRead;
static uint8_t task_state;
uint8_t status;
static uint8_t attempt;
uint8_t rx_counter;

uint8_t txBuffer[MSG_LEN];

//Global variables for queues and mode.
extern QueueHandle_t KWPISRQueue, KWPQueue,RXQueue,BTQueue;
extern bool OBDIIMode;
extern SemaphoreHandle_t semKWP;

Serial K,L;

void KWPTask(void *pvParameters)
{
    xSemaphoreTake(semKWP,portMAX_DELAY);
    memset(&K,0,sizeof(Serial));
    memset(&L,0,sizeof(Serial));
    K.module = 3;
    K.baudRate = 10400;
    K.RXpin = GPIO_PC6_U3RX;
    K.TXpin = GPIO_PC7_U3TX;
    K.usingGPIO = false;
    L.module = 2;
    L.baudRate = 10400;
    L.RXpin = GPIO_PD6_U2RX;
    L.TXpin = GPIO_PD7_U2TX;
    L.usingGPIO = false;
 	serial_enable(&K, 'C');
 	serial_enable(&L, 'D');
 	task_state = FAST;
 	buffer_index = attempt = 0;
 	rx_counter = 0;
 	while(1)
 	{
 	   if(task_state == FAST)
       {
 	      tWup();
          serial_useAsSerial(&K,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
          if(OBDIIMode)
          {
             STC_message[0] = 0xC1;
             STC_message[1] = 0x33;
             STC_message[4] = 0x66;
          }
          serial_enableInts(&K);
          status = TRANSMITTING;
          pendToRead = sizeof(STC_message) / sizeof(uint8_t);
          memcpy(txBuffer,STC_message,pendToRead);
          rx_counter = 0;
          uint8_t i;
          for(i = 0; i < pendToRead; i++)
             serial_send(&K,txBuffer[i]);
          uint8_t Fmt, Tgt, Src, response, KB1, KB2, Cs;
          if(xQueueReceive(RXQueue,&Fmt,RESPONSE_TIME) == errQUEUE_EMPTY){
             noResponseHandler(&task_state); continue; }
          if(xQueueReceive(RXQueue,&Tgt,INTERBYTE_TIME) == errQUEUE_EMPTY){
             noResponseHandler(&task_state); continue; }
          if(xQueueReceive(RXQueue,&Src,INTERBYTE_TIME) == errQUEUE_EMPTY){
             noResponseHandler(&task_state); continue; }
          if(xQueueReceive(RXQueue,&response,INTERBYTE_TIME) == errQUEUE_EMPTY){
             noResponseHandler(&task_state); continue; }
          if(xQueueReceive(RXQueue,&KB1,INTERBYTE_TIME) == errQUEUE_EMPTY){
             noResponseHandler(&task_state); continue; }
          if(xQueueReceive(RXQueue,&KB2,INTERBYTE_TIME) == errQUEUE_EMPTY){
             noResponseHandler(&task_state); continue; }
          if(xQueueReceive(RXQueue,&Cs,INTERBYTE_TIME) == errQUEUE_EMPTY){
             noResponseHandler(&task_state); continue; }
          if( (response != FAST_INIT_STCPR) || (attempt > 2) )
          {
             UARTprintf("KWP: fast init not being able to sync\n");
             task_state = SLOW;
             attempt = 0;
          }
          else
          {
             UARTprintf("KWP: fast init sequence completed\n");
             task_state = READY;
          }
          vTaskDelay(RESPONSE_TIME);
       }
       else if(task_state == SLOW)
       {
           serial_disableInts(&K);
           tIdle();
           char5BaudPut(SLOW_INIT_ADDR);
           serial_useAsSerial(&K,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
           serial_enableInts(&K);
           uint8_t response, KB1, KB2,not_addr;
           status = RECEIVING;
           if(xQueueReceive(RXQueue,&response,(TickType_t)300) == errQUEUE_EMPTY){
             noResponseHandler(&task_state); continue; }
           serial_useAsSerial(&K,(UART_CONFIG_WLEN_7 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_ONE));
           if(xQueueReceive(RXQueue,&KB1,INTERBYTE_TIME) == errQUEUE_EMPTY){
             noResponseHandler(&task_state); continue; }
           if(xQueueReceive(RXQueue,&KB2,INTERBYTE_TIME) == errQUEUE_EMPTY){
             noResponseHandler(&task_state); continue; }
           serial_useAsSerial(&K,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
           vTaskDelay(25/portTICK_PERIOD_MS);
           status = TRANSMITTING;
           rx_counter = 0;
           pendToRead = 1;
           uint8_t notKB2 = (KB2^0xFF);
           txBuffer[0] = notKB2;
           serial_send(&K, notKB2);
           if(xQueueReceive(RXQueue,&not_addr,(TickType_t)50) == errQUEUE_EMPTY){
             noResponseHandler(&task_state); continue; }
           if( (response != SLOW_INIT_STCPR) || (attempt > 2) )
           {
             UARTprintf("KWP: slow init failed. Imposible to initialize\n");
             UARTprintf("KWP: task will be suspended\n");
             vTaskSuspend(NULL); //Suspends Task.
           }
           else
           {
             UARTprintf("KWP: 5 baud init sequence completed\n");
             task_state = READY;
           }
           vTaskDelay(RESPONSE_TIME);
       }
       else if(task_state == READY)
       {
           uint8_t item;
           if(xQueueReceive(RXQueue,&item,RESPONSE_TIME) != errQUEUE_EMPTY)
           {

               uint16_t Cs = 0;
               DATA_MESSAGE msg;
               if( (item >> 7) & 0x01 )
               {
                    msg.dataLength = (expectedDataLength(&item) + 4);
                    #if USE_MALLOC
                    msg.ptrToData = (uint8_t *)calloc(msg.dataLength,sizeof(uint8_t));
                    #else
                    msg.ptrToData = (uint8_t *)pvPortMalloc(sizeof(uint8_t)*msg.dataLength);
                    #endif
                    msg.ptrToData[0] = item;
                    uint8_t i;
                    for (i = 1; i < msg.dataLength; i++)
                    {

                      if(xQueueReceive(RXQueue,&item,INTERBYTE_TIME) == pdTRUE)
                      {
                          msg.ptrToData[i] = item;
                          Cs += msg.ptrToData[i-1];
                          UARTprintf("READY i %d %X\n",i,item);
                      }
                      else if(i == (msg.dataLength - 1) )
                      {

                          serial_receiveNB(&K);
                          msg.ptrToData[i] = Cs & 0xFF;
                          status = IDLE;
                          continue;
                      }
                      else
                      {
                          UARTprintf("READY rx error\n");
                          continue;
                      }
                    }

                    //Final checksum calculation, if checksum is valid the message is eligible to be sent.
                    if( (Cs & 0x00FF) ==  msg.ptrToData[msg.dataLength-1] )
                     xQueueSend(BTQueue,&msg,0); //towards Bluetooth transmit queue.
                    else
                       UARTprintf("Checksum error\n");
                    #if USE_MALLOC
                    free(msg.ptrToData);
                    #else
                    vPortFree(msg.ptrToData);
                    #endif
               }

           }
           else
           {
               uint8_t numData = uxQueueMessagesWaiting(KWPQueue);
               if(numData>0)
               {
                  uint8_t i,item;
                  for(i = 0; i < numData; i++)
                  {
                      xQueueReceive(KWPQueue,&item,0); //originally ticktype_t INTERBYTE_TIME
                      txBuffer[i] = item;
                  }
                  pendToRead = numData;
                  status = TRANSMITTING;
                  for(i = 0; i <numData; i++)
                      serial_send(&K,txBuffer[i]);
                  rx_counter = 0;
               }
               else
               {
                   if(OBDIIMode)
                      generate_OBDII_Message();
                  else
                      generate_SiemensRHY_Message();
               }
           }

       }
 	}


 }



static uint8_t expectedDataLength(uint8_t *param)
{
    return (*param) & 0x3F;
}

/*
 * Routine to meet ISO 14230-2 specification regarding a five baud initialization.
 * Due to the fact that is impossible or not convenient to have such a slow
 * speed, it uses a GPIO module instead of an UART module.
 */
static void char5BaudPut(uint8_t var)
{
    //Start bit, both K and L.
    serial_send(&K,0);
    serial_send(&L,0);
    const TickType_t xDelay = 200 / portTICK_PERIOD_MS;
    vTaskDelay(xDelay);
    /*
        Every variable bit has to be 'manually' put in the line.
    */
    uint8_t i;
    for(i = 0; i < (sizeof(var)*8); i++)
    {
        if(CHECK_BIT_STAT(var,i))
        {
           serial_send(&K,1);
           serial_send(&L,1);
        }
        else
        {
            serial_send(&K,0);
            serial_send(&L,0);
        }

        vTaskDelay(xDelay);
    }
    //Putting a '1' onto L will put the line in a high value, similar to a pull up.
    serial_send(&L,1);
}

/*
 * Idle time to meet ISO 9141 and ISO 14230-2 requirements.
 */
static void tIdle(void)
{
    TickType_t xDelay = 300 / portTICK_PERIOD_MS;
    serial_useAsGPIO(&K);
    serial_useAsGPIO(&L);
    serial_send(&K,1);
    serial_send(&L,1);
    vTaskDelay(xDelay);
}

/*
 * Wake up time to be used in Fast Initialization.
 */
static void tWup(void)
{
    TickType_t xDelay;
    xDelay = 25 / portTICK_PERIOD_MS;
    //300 ms idle time.
    tIdle();
    //25 ms TiniL
    serial_send(&K,0);
    serial_send(&L,0);
    vTaskDelay(xDelay);
    serial_send(&K,1);
    serial_send(&L,1);
    vTaskDelay(xDelay);
    //50 ms of TWUP completed
}

/*
 * A simple handler when a timeout occurs. It is meant to be used only on
 * a initialization state.
 */
static void noResponseHandler(uint8_t *state)
{
    switch(*state)
    {
        case SLOW:
            if(attempt > 2)
            {
                UARTprintf("KWP: slow init failed. Imposible to initialize\n");
                UARTprintf("KWP: task will be suspended\n");
                vTaskSuspend(NULL);
            }
            else
                attempt++;
        break;
        case FAST:
            if(attempt > 2)
            {
                *state = SLOW;
                attempt = 0;
            }
            else
                attempt++;
        break;
    }
}

/*
 * Routine to generate and send an ISO 14230-4 OBD message over K Line.
 */
static void generate_OBDII_Message(void)
{
    uint16_t Checksum = 0;
    OBDII_Message[4] = pids_tx_buffer[buffer_index];
    buffer_index = (buffer_index + 1) % BUFFER_SIZE;
    uint8_t i;
    for (i = 0; i < (MSG_LEN-1); i++)
        Checksum += OBDII_Message[i];
    OBDII_Message[MSG_LEN-1] = (Checksum & 0x00FF);

    //A new message has been created from the circular buffer,
    // now it is time to send it.
    pendToRead = MSG_LEN;
    memcpy(txBuffer,OBDII_Message,pendToRead);

    for(i = 0; i < MSG_LEN; i++)
		serial_send(&K,OBDII_Message[i]);
    status = TRANSMITTING;
    rx_counter = 0;
}

/*
 * Routine to generate and send an approximate accurate ISO 14230-3 message,
 * used on SIEMENS RHY ECUs.
 */
static void generate_SiemensRHY_Message(void)
{
    uint16_t Checksum = 0;
    uint8_t i;
    for (i = 0; i < (MSG_LEN-1); i++)
        Checksum += nonOBD_Message[i];
    nonOBD_Message[MSG_LEN-1] = (Checksum & 0x00FF);

    //A new message has been created from the circular buffer,
    // now it is time to send it.
    pendToRead = MSG_LEN;
    memcpy(txBuffer,nonOBD_Message,pendToRead);
    for(i = 0; i < MSG_LEN; i++)
        serial_send(&K,nonOBD_Message[i]);
    status = TRANSMITTING;
    rx_counter = 0;
}

void RXHandlerTask(void *pvParameters)
{
    status = TRANSMITTING;
    while(1)
    {
        uint8_t item;
        if(xQueueReceive(KWPISRQueue,&item,portMAX_DELAY))
        {
            if(status == RECEIVING)
            {
                xQueueSend(RXQueue,&item,INTERBYTE_TIME);
            }
            else if(status == TRANSMITTING)
            {
                /*
                * If the system is in 'TRANSMITTING' state, and read data
                * doesn't match with what was sent, simply wait for another transmission
                * and discard what is coming. If it is equal, the system waits for another byte
                * until all sent bytes have been read, when state is changed onto 'RECEIVING'.
                */
                if(item == txBuffer[rx_counter])
                    rx_counter++;
                else
                    UARTprintf("RX error\n");
                if(rx_counter == pendToRead)
                {
                    status = RECEIVING;
                    rx_counter = 0;
                }
            }
            /*
             * If the system is not either in TRANSMITTING neither RECEIVING, it will be in IDLE.
             * This means that the data received was rubbish and should not be taken care of. Thus, it
             * will be dequeued and nothing else.
             */
        }
    }
}

//C alike Interrupt Service Routine (ISR) to maximize compatibility.
void KWPHandler(void)
{

    portBASE_TYPE xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    uint32_t ui32Ints;
    ui32Ints = MAP_UARTIntStatus(KWP_INT_CLEAR, true);
    MAP_UARTIntClear(KWP_INT_CLEAR, ui32Ints);
    while(UARTCharsAvail(KWP_INT_CLEAR))
    {
        int32_t line = (int32_t)UARTCharGetNonBlocking(KWP_INT_CLEAR);
        uint8_t read = line & 0xFF;
        xQueueSendFromISR(KWPISRQueue,&read,&xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
