#ifndef UTILS_SERVIDOR_H_
#define UTILS_SERVIDOR_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>


typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;



/**
* @fn      iniciar_servidor
* @brief   Inicializa el Servidor
* @param   logger Log del Servidor
* @param   puerto Puerto del Servidor
* @return  Socket del Servidor
*/
int iniciar_servidor(t_log*, char*);

/**
* @fn      esperar_cliente
* @brief   Espera al Cliente
* @param   logger Log del Servidor
* @param   cliente Cliente del Servidor
* @param   Socket del Servidor
* @return  Socket del Cliente
*/
int esperar_cliente(t_log*,char*, int);

/**
* @fn     recibir_paquete
* @brief  Recibe el paquete
* @param  Socket del Cliente
* @return Paquete
*/
t_list* recibir_paquete(int);

/**
* @fn     recibir_mensaje
* @brief  Recibe el Mensaje
* @param  Socket del Cliente
* @return Nada
*/
void recibir_mensaje(int);

/**
* @fn    recibir_operacion
* @brief Recibe la Operación
* @param   Socket del Cliente
* @return Código de Operación, o -1 en caso de error
*/
int recibir_operacion(int);

#endif