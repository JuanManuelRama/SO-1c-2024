#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <sockets/sockets.h>

t_log* logger;
t_config* config;

int main(int argc, char* argv[]) {
    int conexion_memoria;
    int conexion_cpu;
	char* ip;
	char* puerto;

	// inicializamos logger y config
	logger = log_create("logKernel.log", "LOGS Kernel", 1, LOG_LEVEL_INFO);
	config = config_create("kernel.config");
    
	// buscamos datos en config y conectamos a memoria
	ip = buscar("IP_MEMORIA");
	puerto = buscar ("PUERTO_MEMORIA");
	conexion_memoria = crear_conexion(ip, puerto, "Memoria"); 

	// buscamos datos en config y conectamos a cpu
	ip = buscar("IP_CPU");
	puerto = buscar("PUERTO_CPU_DISPATCH");
	conexion_cpu = crear_conexion(ip, puerto, "CPU");
	
	enviar_mensaje("Saludos desde el Kernel",conexion_cpu);
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion_memoria);
	liberar_conexion(conexion_cpu);
    return 0;
}
