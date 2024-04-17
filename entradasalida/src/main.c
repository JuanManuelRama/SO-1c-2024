#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <sockets/sockets.h>

t_log* logger;
t_config* config;

int main(int argc, char* argv[]) {
    int conexion_memoria;
    int conexion_kernel;
    char* ip;
	char* puerto;

    logger = log_create("logIO.log", "LOGS CPU", 1, LOG_LEVEL_INFO);
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
	enviar_mensaje("Saludos desde la interfaz IO", conexion_kernel);

    log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion_memoria);
	liberar_conexion(conexion_kernel);
    return 0;
}
