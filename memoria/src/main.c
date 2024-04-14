#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <sockets/servidor.c>


int main(void){
	t_log* logger;
	t_config* config;
	char* puerto;
	int socket_servidor;
	int socket_cpu;

	logger = log_create ("logM.log", "Crear log",1, LOG_LEVEL_INFO);
	config=config_create("memoria.config");
		if(config_has_property(config, "PUERTO_ESCUCHA"))
		puerto = config_get_string_value(config, "PUERTO_ESCUCHA");
	log_info(logger, "Memoria inicializada");
	socket_servidor = iniciar_servidor(logger, puerto);
	socket_cpu = esperar_cliente(logger, "CPU", socket_servidor);
	log_destroy(logger);
    return 0;
}
