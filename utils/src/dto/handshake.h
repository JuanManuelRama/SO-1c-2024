#include<stdlib.h>
#include <stdio.h>

/**
 * @fn handshake_cliente
 * @brief Envia el handshake desde el socket de cliente
 * @param socket_cliente socket del cliente desde el que se envia el handshake
 */
 int32_t handshake_cliente (int socket_cliente);

/**
 * @fn handshake_servidor
 * @brief Recibe el handshake enviado desde el cliente
 * @param socket_servidor socket del servidor desde el que se recibe el handshake
 */
 void handshake_servidor (int socket_servidor);
