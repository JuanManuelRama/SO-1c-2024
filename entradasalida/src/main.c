#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <sockets/sockets.h>

t_log* logger;
t_config* config;

void escuchar_kernel(int);

int main(int argc, char* argv[]) {
    int conexion_memoria;
    int conexion_kernel;
    char* ip;
	char* puerto;

    logger = log_create("logIO.log", "LOGS IO", 1, LOG_LEVEL_INFO);
	config = config_create("interfaz.config");
    
    // buscamos datos en config y conectamos con memoria
	ip = buscar("IP_MEMORIA");
	puerto = buscar("PUERTO_MEMORIA");
	conexion_memoria = crear_conexion(ip, puerto, "Memoria"); 
	enviar_mensaje("Saludos desde la interfaz IO", conexion_memoria);

    // buscamos datos en config y conectamos con Kernel
	ip = buscar("IP_KERNEL");
	puerto = buscar("PUERTO_KERNEL");
	conexion_kernel = crear_conexion(ip, puerto, "Kernel"); 

	// esto se puede meter en un thread cuando queramos que haga mas cosas a la vez
	escuchar_kernel(conexion_kernel);

    log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion_memoria);
	liberar_conexion(conexion_kernel);
    return 0;
}


void escuchar_kernel(int socket_kernel) {
	while(1) {
		int cod_op = recibir_operacion(socket_kernel);
		switch (cod_op) {
			case IO_GEN_SLEEP:
				int size;
				char* buffer = recibir_buffer(&size, socket_kernel);
				double tiempo = atof(buffer);
				log_info(logger, "Me mandaron a dormir");
				sleep(tiempo);
				enviar_mensaje("Sleep terminado.", socket_kernel);
				log_info(logger, "Sleep ejecutado exitosamente");
				free(buffer);
				break;
			case -1:
				log_error(logger, "el cliente se desconecto");
				return EXIT_FAILURE;
			default:
				log_warning(logger,"Operacion desconocida. No quieras meter la pata");
				break;
		}
	}
}