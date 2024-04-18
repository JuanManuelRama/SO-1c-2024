#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <sockets/sockets.h>
#include <pthread.h>

t_log*  logger; 
t_config* config; 

int main(void){
	char* puerto;
	int socket_servidor;
	int socket_cpu;
	int socket_kernel;
	int socket_IO;
	pthread_t hilo_cpu, hilo_kernel, hilo_IO;

    logger = log_create ("logM.log", "LOGS MEMORIA",1, LOG_LEVEL_INFO); 
    config = config_create("memoria.config"); 

    // buscamos dato en config e inicializamos servidor para cpu y kernel como clientes
	puerto = buscar("PUERTO_ESCUCHA");
	socket_servidor = iniciar_servidor(puerto, "Memoria");

	socket_cpu = esperar_cliente("CPU", socket_servidor);
	pthread_create(&hilo_cpu, NULL, interactuar, (void*)socket_cpu);

	socket_kernel = esperar_cliente("Kernel", socket_servidor);
	pthread_create(&hilo_kernel, NULL, interactuar, (void*)socket_kernel);

	socket_IO = esperar_cliente("IO", socket_servidor);
	pthread_create(&hilo_IO, NULL, interactuar, (void*)socket_IO);
	
	pthread_join(hilo_cpu, NULL);
	pthread_join(hilo_kernel, NULL);
	pthread_join(hilo_IO, NULL);

	log_destroy(logger);
	config_destroy(config);
    return 0;
}
