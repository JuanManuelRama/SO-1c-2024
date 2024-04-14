#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <sockets/servidor.h>
#include <sockets/servidor.c>

int conectCliente(t_log* logger, int socket_servidor, char* cliente)
{
	int socket_cliente;
	log_info(logger, "Esperando a %s\n", cliente);
	socket_cliente = esperar_cliente(socket_servidor);
	return socket_cliente;
}

int main(void){
	t_log* logger;
	int socket_servidor;
	int socket_cpu;

	logger = log_create ("logM.log", "Crear log",1, LOG_LEVEL_INFO);
	socket_servidor = iniciar_servidor();
	log_info(logger, "Memoria inicializada");
	socket_cpu = conectCliente(logger, socket_servidor, "CPU");
	log_destroy(logger);
    return 0;
}
