#include <cpu/cpu.h>


t_log* logger;
t_config* config;
t_pcb pcb;

int main() {
	logger = log_create("logCpu.log", "LOGS CPU", 1, LOG_LEVEL_INFO);
	config = config_create("cpu.config");
    /*int conexion;
	int socket_servidor;
	int socket_kernel;
	char* ip;
	char* puerto;
	pthread_t hilo_kernel;

	logger = log_create("logCpu.log", "LOGS CPU", 1, LOG_LEVEL_INFO);
	config = config_create("cpu.config");
	
	// buscamos datos en config y conectamos con memoria
	ip = buscar("IP_MEMORIA");
	puerto = buscar("PUERTO_MEMORIA");
	conexion = crear_conexion(ip, puerto, "Memoria"); 
	enviar_mensaje("Saludos desde la cpu", conexion);

	//tambien sera servidor, con el kernel como cliente
	puerto = buscar("PUERTO_ESCUCHA_DISPATCH");
	socket_servidor = iniciar_servidor(puerto, "CPU");
	socket_kernel = esperar_cliente("Kernel", socket_servidor);
	pthread_create(&hilo_kernel, NULL, interactuar, (void*)socket_kernel);
	
	pthread_join(hilo_kernel, NULL);

    liberar_conexion(conexion);
    log_destroy(logger);
    config_destroy(config);*/

	char* buffer;
	sInstruccion instruccion;
	buffer=string_new();
	pcb.pid=1;
	while(1){
		buffer = fetch();
		instruccion = decode(buffer);
		execute(instruccion);

	}
    return 0;

}
