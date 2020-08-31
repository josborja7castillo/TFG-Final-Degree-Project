/*
 * remotelink.c
 *
 * Implementa la funcionalidad RPC entre la TIVA y el interfaz de usuario
 */

#include<stdbool.h>
#include<stdint.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include "driverlib/adc.h"
#include "driverlib/timer.h"
#include "utils/uartstdio.h"
#include "usb_dev_serial.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "serialprotocol.h"
#include "remotelink.h"
#include "remotelink_messages.h"


#define VARIABLE 0xFF

////Defino a un tipo que es un puntero a funcion con el prototipo que tienen que tener las funciones que definamos
//typedef int32_t (*rpc_function_prototype)(uint32_t param_size, void *param);

static uint8_t Rxframe[MAX_FRAME_SIZE];	//Usar una global permite ahorrar pila en la tarea de RX.
static uint8_t Txframe[MAX_FRAME_SIZE]; //Usar una global permite ahorrar pila en las tareas, pero hay que tener cuidado al transmitir desde varias tareas!!!!
static uint32_t gRemoteProtocolErrors=0;

extern QueueHandle_t TXQueue;

// TODO
//Ojo!! TxFrame es global (para ahorrar memoria de pila en las tareas) --> Se deben tomar precauciones al usar esta funcion varias tareas
//IDEM en lo que respecta al envio por la conexion USB serie desde varias tareas....
//Estas precauciones no se han tomado en este codigo de parti1da, pero al realizar la practica se deberian tener en cuenta....
// "TODO" (por hacer)
int32_t remotelink_sendMessage(uint8_t message_type,void *parameter,int32_t paramsize)
{
    int32_t numdatos;

    numdatos=create_frame(Txframe,message_type,parameter,paramsize,MAX_FRAME_SIZE);
    if (numdatos>=0)
    {
        USBSerialWrite(Txframe,numdatos,portMAX_DELAY);
    }

    return numdatos;
}

//Funciones internas

/* Recibe una trama (sin los caracteres de inicio y fin */
/* Utiliza la funcion bloqueante xSerialGetChar ---> Esta funcion es bloqueante y no se puede llamar desde una ISR !!!*/
// Esta funcion es INTERNA de la biblioteca y solo se utiliza en la rarea TivaRPC_ServerTask
static int32_t remotelink_waitForMessage(uint8_t *frame, int32_t maxFrameSize)
{
    int32_t i;
    uint8_t rxByte;

    do
    {
        //Elimino bytes de la cola de recepcion hasta llegar a comienzo de nueva trama
        USBSerialGetChar( ( char *)&rxByte, portMAX_DELAY);
    } while (rxByte!=START_FRAME_CHAR);

    i=0;
    do
    {
        USBSerialGetChar( ( char *)frame, portMAX_DELAY);
        i++;
    } while ((*(frame++)!=STOP_FRAME_CHAR)&&(i<=maxFrameSize));

    if (i>maxFrameSize)
    {
        return PROT_ERROR_RX_FRAME_TOO_LONG;    //Numero Negativo indicando error
    }
    else
    {

        if (i<(MINIMUN_FRAME_SIZE-START_SIZE))
        {
            return PROT_ERROR_BAD_SIZE; //La trama no tiene el tamanio minimo
        }
        else
        {
            return (i-END_SIZE); //Tamanio de la trama sin los bytes de inicio y fin
        }
    }
}

//Funcion que procesa los mensajes recibidos desde el PC (ejecuta las acciones correspondientes a las ordenes recibidas)
static int32_t remotelink_messageReceived(uint8_t message_type, void *parameters, int32_t parameterSize)
{
    int32_t status=0;   //Estado de la ejecucion (positivo, sin errores, negativo si error)

    //Comprueba el tipo de mensaje
    switch (message_type)
    {       
       case MESSAGE_PING:
        {
            remotelink_sendMessage(MESSAGE_PING,NULL,0);
        }
        break;

       default:
           //mensaje desconocido/no implementado
           status=PROT_ERROR_UNIMPLEMENTED_COMMAND; //Devuelve error.
    }


    return status;   //Devuelve status
}


// Codigo para procesar los comandos recibidos a traves del canal USB del micro ("conector lateral")
//Esta tarea decodifica los comandos y ejecuta la función que los procesa
//También gestiona posibles errores en la comunicacion
static portTASK_FUNCTION( remotelink_Task, pvParameters ){

    int32_t numdatos;
    uint8_t command;
    void *ptrtoreceivedparam;
    int32_t error_status=0;

    /* The parameters are not used. (elimina el warning)*/
    ( void ) pvParameters;

    for(;;)
    {
        //Espera hasta que se reciba una trama con datos serializados por el interfaz USB
        numdatos=remotelink_waitForMessage(Rxframe,MAX_FRAME_SIZE); //Esta funcion es bloqueante

        if (numdatos>0)
        {
            //Si no ha habido errores recibiendo la trama, la intenta decodificar y procesar
            //primero se "desestufa" y se comprueba el checksum
            numdatos=destuff_and_check_checksum(Rxframe,numdatos);
            if (numdatos<0)
            {
                //Error de checksum (PROT_ERROR_BAD_CHECKSUM), ignorar el paquete
                gRemoteProtocolErrors++;
                // Procesamiento del error (TODO, POR HACER!!)
            }
            else
            {
                //El paquete esta bien, luego procedo a tratarlo.
                //Obtiene el valor del campo comando
                command=decode_command_type(Rxframe);
                //Obtiene un puntero al campo de parametros y su tamanio.
                numdatos=get_command_param_pointer(Rxframe,numdatos,&ptrtoreceivedparam);

                //Llamo a la funcion que procesa la orden que nos ha llegado desde el PC
                error_status=remotelink_messageReceived(command,ptrtoreceivedparam,numdatos);

                //Una vez ejecutado, se comprueba si ha habido errores.
                switch(error_status)
                {
                    //Se procesarían a continuación
                    case PROT_ERROR_NOMEM:
                    {
                        // Procesamiento del error NO MEMORY
                        UARTprintf("Remotelink Error: not enough memory\n");
                    }
                    break;
                    case PROT_ERROR_STUFFED_FRAME_TOO_LONG:
                    {
                        // Procesamiento del error STUFFED_FRAME_TOO_LONG
                        UARTprintf("Remotelink Error: Frame too long. Cannot be created\n");
                    }
                    break;
                    case PROT_ERROR_COMMAND_TOO_LONG:
                    {
                        // Procesamiento del error COMMAND TOO LONG
                        UARTprintf("Remotelink Error: Packet too long. Cannot be allocated\n");
                    }
                    break;
                    case PROT_ERROR_INCORRECT_PARAM_SIZE:
                    {
                        // Procesamiento del error INCORRECT PARAM SIZE
                        UARTprintf("Remotelink Error: Incorrect parameter size\n");
                    }
                    break;
                    case PROT_ERROR_UNIMPLEMENTED_COMMAND:
                    {
                        MESSAGE_REJECTED_PARAMETER parametro;
                        parametro.command=command;
                        numdatos=remotelink_sendMessage(MESSAGE_REJECTED,&parametro,sizeof(parametro));
                        UARTprintf("Remotelink Error: Unexpected command: %x\n",(uint32_t)command);
                        gRemoteProtocolErrors++;
                        //Aqui se podria, ademas, comprobar numdatos....
                    }
                    break;
                    //AÑadir casos de error adicionales aqui...
                    default:
                    /* No hacer nada */
                    break;
                }
            }
        }
        else
        { // if (numdatos >0)
            //Error de recepcion de trama(PROT_ERROR_RX_FRAME_TOO_LONG), ignorar el paquete
            gRemoteProtocolErrors++;
            // Procesamiento del error (TODO)
        }
    }
}


//Inicializa la tarea que recibe comandos (se debe llamar desde main())
void remotelink_init(void)
{
    USBSerialInit(32,32);   //Inicializo el  sistema USB-serie

    //
    // Crea la tarea que gestiona los comandos USB (definidos en CommandProcessingTask)
    //
    if(xTaskCreate(remotelink_Task, (portCHAR *)"remotelink",REMOTELINK_TASK_STACK, NULL, REMOTELINK_TASK_PRIORITY, NULL) != pdTRUE)
    {
        while(1);
    }

}





