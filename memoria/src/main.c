#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <sockets/servidor.h>
#include <sockets/servidor.c>


int main(void){
	t_log* logger;
	int socket_servidor;
	int socket_cpu;

	logger = log_create ("logM.log", "Crear log",1, LOG_LEVEL_INFO);
	log_info(logger, "Memoria inicializada");
	socket_servidor = iniciar_servidor(logger);
	socket_cpu = esperar_cliente(logger, "CPU", socket_servidor);
	log_destroy(logger);
    return 0;
}
