#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <sockets/servidor.h>
#include <utils/varias.h>

t_log*  logger; 
t_config* config; 

int main(void){
	char* puerto;
	int socket_servidor;
	int socket_cpu;
	int socket_kernel;

    logger = log_create ("logM.log", "LOGS MEMORIA",1, LOG_LEVEL_INFO); 
    config = config_create("memoria.config"); 

    // buscamos dato en config e inicializamos servidor para cpu y kernel como clientes
	puerto = buscar(logger, config, "PUERTO_ESCUCHA");
	socket_servidor = iniciar_servidor(logger, puerto, "Memoria");
	socket_cpu = esperar_cliente(logger, "CPU", socket_servidor);
	socket_kernel = esperar_cliente(logger, "Kernel", socket_servidor);
	while (1){
		int cod_op = recibir_operacion(socket_cpu);
		if(cod_op==MENSAJE){
			recibir_mensaje(socket_cpu, logger);
			break;
		}
	}
	log_destroy(logger);
	config_destroy(config);
    return 0;
}
