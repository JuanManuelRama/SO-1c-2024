#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <sockets/cliente.c>
#include <utils/varias.h>

int main(int argc, char* argv[]) {

    int conexion;
	int socket_servidor;
	int socket_kernel;
	char* ip;
	char* puerto;
	char* valor;
	t_log* logger;
	t_config* config;

    logger = log_create("logCpu.log", "LOGS CPU", 1, LOG_LEVEL_INFO);
    log_info(logger, "TEST", "INFO");
    
    config = config_create("cpu.config");
	
	ip = buscar(logger, config, "IP_MEMORIA");

	puerto = buscar(logger, config, "PUERTO_MEMORIA");

    log_info(logger, ip, "INFO");
	log_info(logger, puerto, "INFO");

	conexion = crear_conexion(ip, puerto);
	socket_servidor = iniciar_servidor(logger, "8006");
	socket_kernel = esperar_cliente(logger, "Kernel", socket_servidor);

    liberar_conexion(conexion);
    log_destroy(logger);
    config_destroy(config);
    return 0;
}
