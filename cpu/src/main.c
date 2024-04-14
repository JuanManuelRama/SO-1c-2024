#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <sockets/cliente.c>

int main(int argc, char* argv[]) {

    int conexion;
	char* ip;
	char* puerto;
	char* valor;

	t_log* logger;
	t_config* config;

    logger = log_create("logCpu.log", "LOGS CPU", 1, LOG_LEVEL_INFO);
    log_info(logger, "TEST", "INFO");
    
    config = config_create("cpu.config");
	
	if(config_has_property(config, "IP_MEMORIA"))
		ip = config_get_string_value(config, "IP_MEMORIA");

	if(config_has_property(config, "PUERTO_MEMORIA"))
		puerto = config_get_string_value(config, "PUERTO_MEMORIA");

    log_info(logger, ip, "INFO");
	log_info(logger, puerto, "INFO");

	conexion = crear_conexion(ip, puerto);

    liberar_conexion(conexion);
    log_destroy(logger);
    config_destroy(config);
    return 0;
}
