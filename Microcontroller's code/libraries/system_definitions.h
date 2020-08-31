/*
 * system_definitions.h
 *
 *  Created on: 21 abr. 2020
 *      Author: borja
 */

#ifndef SYSTEM_DEFINITIONS_H_
#define SYSTEM_DEFINITIONS_H_

#define BT_TASK_PRIORITY            RX_TASK_PRIORITY+1
#define KWP_TASK_PRIORITY           tskIDLE_PRIORITY+2
#define RX_TASK_PRIORITY            KWP_TASK_PRIORITY-1
#define BT_STACK_SIZE               128 //stack size of 128 words (256 bytes)
#define KWP_STACK_SIZE              512 //stack size of 256 words (512 bytes)
#define RX_STACK_SIZE               configMINIMAL_STACK_SIZE //Minimum stack size as defined in FreeRTOSConfig.h


#define USE_MALLOC 0


typedef struct{
    uint8_t dataLength;
    uint8_t *ptrToData;
}DATA_MESSAGE;

#endif /* SYSTEM_DEFINITIONS_H_ */
