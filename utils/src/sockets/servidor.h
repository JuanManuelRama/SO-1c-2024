#ifndef UTILS_SERVER_H_
#define UTILS_SERVER_H_
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>


typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

extern t_log* logger;


/**
* @fn    iniciar_servidor
* @brief Inicializa el servidor
* @param   Ninguno
* @return  Socket del Servidor
*/
int iniciar_servidor(void);

/**
* @fn    esperar_cliente
* @brief Espera al cliente
* @param   Socket del Servidor
* @return Socket del Cliente
*/
int esperar_cliente(int);

/**
* @fn    recibir_paquete
* @brief Recibe el paquete
* @param   Socket del Cliente
* @return Paquete
*/
t_list* recibir_paquete(int);

/**
* @fn    recibir_mensaje
* @brief Recibe el Mensaje
* @param   Socket del Cliente
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