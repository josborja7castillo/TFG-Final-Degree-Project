/*
 * kwp2000.h
 *
 *  Created on: 20 feb. 2020
 *      Author: borja
 */

#ifndef KWP2000_KWP2000_H_
#define KWP2000_KWP2000_H_

#include <stdint.h>
#include <FreeRTOS.h>
#include "queue.h"
#include "semphr.h"

#define SLOW_INIT_ADDR 0x33
#define SLOW_INIT_STCPR 0x55
#define FAST_INIT_STC   0x81
#define FAST_INIT_STCPR 0xC1

#define MY_ADDR     0xF1
#define BUFFER_SIZE 64
#define OBD2_MODE   0x01
#define MSG_LEN     6

#define FAST            0
#define SLOW            1
#define READY           2

#define KWP_INT_CLEAR               UART3_BASE

void KWPTask(void *pvParameters);
void KWPHandler(void);
static uint8_t expectedDataLength(uint8_t *param);
static void char5BaudPut(uint8_t var);
static void tIdle(void);
static void tWup(void);
static void noResponseHandler(uint8_t *state);
static void generate_OBDII_Message(void);
static void generate_SiemensRHY_Message(void);
void RXHandlerTask(void *pvParameters);



static const uint8_t pids_tx_buffer[BUFFER_SIZE] = {
                                                    0x05,
                                                    0x10,
                                                    0x23,
                                                    0x0B,
                                                    0x0C,
                                                    0x0D,
                                                    0x0C,
                                                    0x0C,
                                                    0x0C,
                                                    0x0D,
                                                    0x0C,
                                                    0x0C,
                                                    0x0C,
                                                    0x0C,
                                                    0x0D,
                                                    0x0C,
                                                    0x0C,
                                                    0x0D,
                                                    0x0C,
                                                    0x0C,
                                                    0x0C,
                                                    0x0D,
                                                    0x0C,
                                                    0x0C,
                                                    0x0C,
                                                    0x10,
                                                    0x23,
                                                    0x0B,
                                                    0x0D,
                                                    0x0C,
                                                    0x0C,
                                                    0x0C,
                                                    0x0D,
                                                    0x0C,
                                                    0x0C,
                                                    0x0C,
                                                    0x0D,
                                                    0x0C,
                                                    0x0C,
                                                    0x0C,
                                                    0x0D,
                                                    0x0C,
                                                    0x0C,
                                                    0x0C,
                                                    0x0D,
                                                    0x0C,
                                                    0x0C,
                                                    0x0C,
                                                    0x0D,
                                                    0x0C,
                                                    0x0C,
                                                    0x0C,
                                                    0x0D,
                                                    0x0C,
                                                    0x0C,
                                                    0x10,
                                                    0x23,
                                                    0x0B,
                                                    0x0C,
                                                    0x0D,
                                                    0x0B,
                                                    0x23,
                                                    0x0C,
                                                    0x0D
};

static uint8_t OBDII_Message[MSG_LEN] = {
                                    0xC2,
                                    0x33,
                                    MY_ADDR,
                                    OBD2_MODE,
                                    0x00,
                                    0x00};

static uint8_t nonOBD_Message[MSG_LEN] = {
                                    0x82,
                                    0x10,
                                    MY_ADDR,
                                    0x21,
                                    0xC4,
                                    0x68
};

static uint8_t clearDTC_Message[] = {
                                     0xC1,
                                     0x33,
                                     0xF1,
                                     0x04,
                                     0xE9};
static uint8_t readDTC_Message[] = {
                                    0xC1,
                                    0X33,
                                    0xF1,
                                    0x03,
                                    0xE8};

static uint8_t STC_message[5]={0x81,0x10,0xF1,0x81,0x03};


#endif /* KWP2000_KWP2000_H_ */
