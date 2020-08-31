/*
* Main file for backend.
*/
#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "utils/uartstdio.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "utils/cpu_usage.h"

#include "remotelink/remotelink.h"
#include "remotelink/serialprotocol.h"

#include "libraries/system_definitions.h"
#include "libraries/bluetooth.h"
#include "libraries/kwp2000.h"

uint32_t g_ui32CPUUsage;
uint32_t g_ulSystemClock;

/* Declare some variables to hold the necessary queues. */
QueueHandle_t BTQueue,KWPQueue,KWPISRQueue,BTISRQueue,RXQueue;
QueueSetHandle_t BTQueueSet;
SemaphoreHandle_t semKWP;

TaskHandle_t kwpTaskHandler;

bool OBDIIMode;

void create_IPCs(void)
{
    if( (BTISRQueue=xQueueCreate(64,sizeof(uint8_t))) == NULL) while(1);
    if( (BTQueue=xQueueCreate(10,sizeof(DATA_MESSAGE))) == NULL) while(1);
    if( (KWPQueue=xQueueCreate(64,sizeof(uint8_t))) == NULL) while(1);
    if( (KWPISRQueue=xQueueCreate(64,sizeof(uint8_t))) == NULL) while(1);
    if( (RXQueue=xQueueCreate(64,sizeof(uint8_t))) == NULL) while(1);
    if( (semKWP=xSemaphoreCreateBinary()) == NULL) while(1);
    BTQueueSet = xQueueCreateSet(74);
    xQueueAddToSet(BTQueue, BTQueueSet);
    xQueueAddToSet(BTISRQueue, BTQueueSet);
}





#ifdef DEBUG
void __error__(char *nombrefich, uint32_t linea)
{
    while(1)
    {
    }
}
#endif


void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName)
{
    //
    // This function can not return, so loop forever.  Interrupts are disabled
    // on entry to this function, so no processor interrupts will interrupt
    // this loop.
    //
    while(1)
    {
    }
}

void vApplicationTickHook( void )
{
    static uint8_t count = 0;

    if (++count == 10)
    {
        g_ui32CPUUsage = CPUUsageTick();
        count = 0;
    }
    //return;
}
void vApplicationIdleHook (void)
{
    SysCtlSleep();
}

void vApplicationMallocFailedHook (void)
{
    while(1);
}


extern void vUARTTask( void *pvParameters );

void enable_lockedPins(void)
{
    /* Function to enable locked pins on TI EV  boards and microcontrollers.
     * In this case, only GPIO port D and pin 7 will be freed.
     * It's mandatory the inclusion of these libraries:
     *   #include "inc/hw_types.h"
     *   #include "inc/hw_gpio.h"
     *   #include "inc/hw_memmap.h"
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C2);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_I2C2)));
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);
    HWREG(GPIO_PORTD_BASE+GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTD_BASE+GPIO_O_CR) |= GPIO_PIN_7;
}

void main(void)
{
    //
    // Set the clocking to run at 40 MHz from the PLL.
    //
    MAP_SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
            SYSCTL_OSC_MAIN);   //Ponermos el reloj principal a 40 MHz (200 Mhz del Pll dividido por 5)


    // Get the system clock speed.
    g_ulSystemClock = SysCtlClockGet();
    MAP_SysCtlPeripheralClockGating(true);
    //Uses TIMER3
    CPUUsageInit(g_ulSystemClock, configTICK_RATE_HZ/10, 3);

    enable_lockedPins();

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0,115200,SysCtlClockGet());

    MAP_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UART0);
    MAP_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOA);

    create_IPCs();

    if((xTaskCreate(vUARTTask, (portCHAR *)"Uart", 512,NULL,tskIDLE_PRIORITY + 1, NULL) != pdTRUE))
    {
        while(1);
    }

    // tasks creation.
   if(xTaskCreate(KWPTask,"KWP Task",KWP_STACK_SIZE,NULL,KWP_TASK_PRIORITY,&kwpTaskHandler) != pdPASS) while(1);
   if(xTaskCreate(BTTask,"BT Task",BT_STACK_SIZE,NULL,BT_TASK_PRIORITY,NULL) != pdPASS ) while(1);
   if(xTaskCreate(RXHandlerTask,"RX Task",RX_STACK_SIZE,NULL,RX_TASK_PRIORITY,NULL) != pdPASS) while(1);

   //IntMasterEnable();

   vTaskStartScheduler();

   while(1);
}

