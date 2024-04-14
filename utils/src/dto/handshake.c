#include "handshake.h"

int32_t handshake_cliente (int socket_cliente){
    size_t bytes;

    int32_t handshake = 1;
    int32_t resultado;

    bytes = send(socket_cliente, &handshake, sizeof(int32_t), 0);
    bytes = recv(socket_cliente, &resultado, sizeof(int32_t), MSG_WAITALL);

    if (resultado == 0) {
    // Handshake OK
    return 0;
    } else {
    // Handshake ERROR
    return -1;
    }
}

void handshake_servidor (int socket_servidor){
    size_t bytes;

    int32_t handshake;
    int32_t resultOk = 0;
    int32_t resultError = -1;

    bytes = recv(socket_servidor, &handshake, sizeof(int32_t), MSG_WAITALL);
    if (handshake == 1) {
        bytes = send(socket_servidor, &resultOk, sizeof(int32_t), 0);
    } else {
        bytes = send(socket_servidor, &resultError, sizeof(int32_t), 0);
    }

}