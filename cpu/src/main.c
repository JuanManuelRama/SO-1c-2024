#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <sockets/servidor.h>
#include <pthread.h>

t_log* logger;
t_config* config;

int main(int argc, char* argv[]) {
    int conexion;
	int socket_servidor;
	int socket_kernel;
	char* ip;
	char* puerto;
	char* valor;
	pthread_t hilo_kernel;

	logger = log_create("logCpu.log", "LOGS CPU", 1, LOG_LEVEL_INFO);
	config = config_create("cpu.config");
	
	// buscamos datos en config y conectamos con memoria
	ip = buscar("IP_MEMORIA");
	puerto = buscar("PUERTO_MEMORIA");
	conexion = crear_conexion(ip, puerto, "Memoria"); 

	//tambien sera servidor, con el kernel como cliente
	puerto = buscar("PUERTO_ESCUCHA_DISPATCH");
	socket_servidor = iniciar_servidor(puerto, "CPU");
	socket_kernel = esperar_cliente("Kernel", socket_servidor);
	enviar_mensaje("Saludos desde la cpu", conexion);
	pthread_create(&hilo_kernel, NULL, interactuar, (void*)socket_kernel);
	pthread_join(hilo_kernel, NULL);
    liberar_conexion(conexion);
    log_destroy(logger);
    config_destroy(config);
    return 0;
}
