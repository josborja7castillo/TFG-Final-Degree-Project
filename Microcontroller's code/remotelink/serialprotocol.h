/* serialprotocol.h
 *
 * Este fichero define las funciones para implementar la comunicacion mediante
 * el protocolo de serializacion propuesto
 *
 * Los mensajes que se envian por el perfil USB constan de un byte de INICIO,
 * un byte de comando, un campo de datos opcional de longitud variable que depende del comando,
 * un CRC de 2 bytes y un byte de FIN.
 *
 */


#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include<stdlib.h>
#include<string.h>
#include<FreeRTOS.h>

//Caracteres especiales
#define START_FRAME_CHAR 0xFC
#define STOP_FRAME_CHAR 0xFD
#define ESCAPE_CHAR 0xFE
#define STUFFING_MASK 0x40

//Tipos de los campos
#define CHEKSUM_TYPE uint16_t
#define COMMAND_TYPE uint8_t


#define CHECKSUM_SIZE (sizeof(CHEKSUM_TYPE))
#define COMMAND_SIZE (sizeof(COMMAND_TYPE))
#define START_SIZE (1)
#define END_SIZE (1)

#define MINIMUN_FRAME_SIZE (START_SIZE+COMMAND_SIZE+CHECKSUM_SIZE+END_SIZE)

#define MAX_DATA_SIZE (32)
#define MAX_FRAME_SIZE (8*(MAX_DATA_SIZE))

#define VARIABLE 0xFF


//Codigos de Error del protocolo
#define PROT_ERROR_BAD_CHECKSUM (-1)
#define PROT_ERROR_RX_FRAME_TOO_LONG (-2)
#define PROT_ERROR_NOMEM (-3)
#define PROT_ERROR_STUFFED_FRAME_TOO_LONG (-4)
#define PROT_ERROR_COMMAND_TOO_LONG (-5)
#define PROT_ERROR_INCORRECT_PARAM_SIZE (-6)
#define PROT_ERROR_BAD_SIZE (-7)
#define PROT_ERROR_UNIMPLEMENTED_COMMAND (-7)


//********************Funciones de la libreria

//Funciones que obtienen campos del paquete
uint8_t decode_command_type(uint8_t * buffer);
int32_t check_and_extract_command_param(void *ptrtodata, int32_t data_size, void *ptrtoparam, uint32_t param_size);
int32_t get_command_param_pointer(uint8_t * buffer, int32_t frame_size, void **campo);

//Funciones para codificar y decodificar tramas
int32_t create_frame(uint8_t *frame, uint8_t command_type, void * param, int32_t param_size, int32_t max_size);
int32_t destuff_and_check_checksum(uint8_t *frame, int32_t max_size);


#endif
