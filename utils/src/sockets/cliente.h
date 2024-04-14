#ifndef CLIENTE_H_
#define CLIENTE_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<commons/config.h>

typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

/**
* @fn       crear_conexion
* @brief    Crea la conexión
* @param    ip
* @param    puerto
* @return   Socket del Cliente
*/
int crear_conexion(char* ip, char* puerto);

/**
* @fn       enviar_mensaje
* @brief    Envía un mensaje
* @param    mensaje
* @param    Socket del Cliente
* @return   Socket del Servidor
*/
void enviar_mensaje(char* mensaje, int socket_cliente);

/**
* @fn       crear_paquete
* @brief    Crea un paquete
* @param    Ninguno
* @return   Paquete
*/
t_paquete* crear_paquete(void);

/**
* @fn       agregar_a_paquete
* @brief    Agrega al paquete
* @param    paquete
* @param    valor a agregar
* @param    tamaño del valor
* @return   nada
*/
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);

/**
* @fn       enviar_paquete
* @brief    Envía el paquete
* @param    paquete
* @param    socket_cliente
* @return   nada
*/
void enviar_paquete(t_paquete* paquete, int socket_cliente);

/**
* @fn    liberar_conexion
* @brief Libera la conexión
* @param   socket_cliente
* @return  nada
*/
void liberar_conexion(int socket_cliente);

/**
* @fn    eliminar_paquete
* @brief Elimina el Paquete
* @param   Ninguno
* @return  Socket del Servidor
*/
void eliminar_paquete(t_paquete* paquete);

#endif
