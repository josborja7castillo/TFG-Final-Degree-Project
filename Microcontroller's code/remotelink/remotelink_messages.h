/*
 * remotelink_messages.h
 *
 *  Created on: March. 2019
 *
 */

#ifndef RL_MESSAGES_H
#define RL_MESSAGES_H


typedef enum {
    MESSAGE_PING,
    MESSAGE_REJECTED
    //etc, etc...
} commandTypes;


#pragma pack(1) //Changes memory alignment to 1 byte.

typedef struct {
    uint8_t command;
} MESSAGE_REJECTED_PARAMETER;

#pragma pack()  //Back to normal memory alignment.

#endif // RPCCOMMANDS_H

