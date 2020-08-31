/*
 * bluetooth.h
 *
 *  Created on: 29 mar. 2020
 *      Author: borja
 */

#ifndef LIBRARIES_BLUETOOTH_H_
#define LIBRARIES_BLUETOOTH_H_

#define BT_INT_CLEAR                UART1_BASE
#define BT_NT_TESTING                  1

void BTTask (void *pvParameters);
static void translate(char *msg);

#endif /* LIBRARIES_BLUETOOTH_H_ */
