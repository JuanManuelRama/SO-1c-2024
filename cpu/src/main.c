#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <sockets/cliente.h>
#include <utils/varias.h>

t_log* logger;
t_config* config;

int main(int argc, char* argv[]) {
    int conexion;
	int socket_servidor;
	int socket_kernel;
	char* ip;
	char* puerto;
	char* valor;

	logger = log_create("logCpu.log", "LOGS CPU", 1, LOG_LEVEL_INFO);
	config = config_create("cpu.config");
	
	// buscamos datos en config y conectamos con memoria
	ip = buscar(logger, config, "IP_MEMORIA");
	puerto = buscar(logger, config, "PUERTO_MEMORIA");
	conexion = crear_conexion(ip, puerto); 
	enviar_mensaje("Saludos desde la cpu",conexion);

	//tambien sera servidor, con el kernel como cliente
	puerto = buscar(logger, config, "PUERTO_ESCUCHA_DISPATCH");
	socket_servidor = iniciar_servidor(logger, puerto, "CPU");
	socket_kernel = esperar_cliente(logger, "Kernel", socket_servidor);
	enviar_mensaje("Saludos desde la cpu",conexion);
	while (1){
		int cod_op = recibir_operacion(socket_kernel);
		if(cod_op==MENSAJE){
			recibir_mensaje(socket_kernel, logger);
			break;
		}
	}
    liberar_conexion(conexion);
    log_destroy(logger);
    config_destroy(config);
    return 0;
}
