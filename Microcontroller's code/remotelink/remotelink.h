/*
 * tivarpc.h
 *
 *  Fichero de cabecera. Implementa la funcionalidad RPC entre la TIVA y el interfaz de usuario
 */

#ifndef TIVARPC_H__
#define TIVARPC_H__


#include "remotelink_messages.h"
#include<stdbool.h>
#include<stdint.h>

#include "serialprotocol.h"

//parametros de funcionamiento de la tarea
#define REMOTELINK_TASK_STACK (512)
#define REMOTELINK_TASK_PRIORITY (tskIDLE_PRIORITY+2)

void remotelink_init(void);
int32_t remotelink_sendMessage(uint8_t comando,void *parameter,int32_t paramsize);

#endif /*TIVARPC_H__ */
