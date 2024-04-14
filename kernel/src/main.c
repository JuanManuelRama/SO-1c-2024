#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <sockets/cliente.c>
#include <utils/varias.h>

int main(int argc, char* argv[]) {

    int conexion_memoria;
    int conexion_cpu;
	char* ip;
	char* puerto;
	char* valor;
	int i;
	t_log* logger;
	t_config* config;

    logger = log_create("logKernel.log", "LOGS Kernel", 1, LOG_LEVEL_INFO);
    log_info(logger, "TEST", "INFO");
    
    config = config_create("kernel.config");
	ip = buscar(logger, config, "IP_MEMORIA");
	puerto = buscar (logger, config, "PUERTO_MEMORIA");

    log_info(logger, ip, "INFO");
	log_info(logger, puerto, "INFO");

	conexion_memoria = crear_conexion(ip, puerto);

	ip = buscar(logger, config, "IP_CPU");
	puerto = buscar(logger, config, "PUERTO_CPU_DISPATCH");
	conexion_cpu = crear_conexion(ip, puerto);    


    liberar_conexion(conexion_memoria);
    liberar_conexion(conexion_cpu);
    log_destroy(logger);
    config_destroy(config);
    return 0;
}
