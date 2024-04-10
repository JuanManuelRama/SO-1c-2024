#ifndef UTILS_HELLO_H_
#define UTILS_HELLO_H_
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>

/**
* @fn    iniciar_servidor
* @brief Inicializa el servidor
* @par   Ninguno
*/
int iniciar_servidor(void);

/**
* @fn    esperar_cliente
* @brief Espera al cliente
* @par   Socket del Servidor
*/
int esperar_cliente(int)

/**
* @fn    recibir_paquete
* @brief Recibe el paquete
* @par   Socket del Cliente
*/
t_list* recibir_paquete(int);

/**
* @fn    recibir_mensaje
* @brief Recibe el Mensaje
* @par   Socket del Cliente
*/
void recibir_mensaje(int);

/**
* @fn    recibir_operacion
* @brief Recibe la Operación
* @par   Socket del Cliente
*/
int recibir_operacion(int);

#endif