#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <sockets/servidor.c>
#include <utils/varias.h>


int main(void){
	t_log* logger;
	t_config* config;
	char* puerto;
	int socket_servidor;
	int socket_cpu;
	int socket_kernel;

	logger = log_create ("logM.log", "Crear log",1, LOG_LEVEL_INFO);
	config=config_create("memoria.config");
	puerto = buscar(logger, config, "PUERTO_ESCUCHA");
	log_info(logger, "Memoria inicializada");
	socket_servidor = iniciar_servidor(logger, puerto);
	socket_cpu = esperar_cliente(logger, "CPU", socket_servidor);
	socket_kernel = esperar_cliente(logger, "Kernel", socket_servidor);

	log_destroy(logger);
	config_destroy(config);
    return 0;
}
