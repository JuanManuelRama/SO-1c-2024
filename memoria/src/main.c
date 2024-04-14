#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <sockets/servidor.h>
#include <sockets/servidor.c>

void conectCliente(t_log* logger, int socket_servidor, char* cliente, int socket_cliente)
{
	log_info(logger, "Esperando a %s\n", cliente); //No se como concatenar el mensaje al nombre del cliente 
	socket_cliente = esperar_cliente(socket_servidor); //No se como pasar el socket del cliente por referencia
}

int main(void){
	t_log* logger;
	int socket_servidor;
	int socket_cpu=0;

	logger = log_create ("logM.log", "Crear log",1, LOG_LEVEL_INFO);
	socket_servidor = iniciar_servidor();
	log_info(logger, "Memoria inicializada");
	conectCliente(logger, socket_servidor, "CPU", socket_cpu);
	log_destroy(logger);
    return 0;
}
