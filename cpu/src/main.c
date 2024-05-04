#include <cpu/cpu.h>


t_log* logger;
t_config* config;
t_pcb pcb;
pthread_mutex_t mProceso;

int main() {
	logger = log_create("logCpu.log", "LOGS CPU", 1, LOG_LEVEL_INFO);
	config = config_create("cpu.config");
	pthread_mutex_init(&mProceso, NULL);
	pthread_mutex_lock(&mProceso);
    int conexion;
	int socket_servidor;
	int socket_cliente;
	char* ip;
	char* puerto;
	pthread_t hilo_kernel;

	logger = log_create("logCpu.log", "LOGS CPU", 1, LOG_LEVEL_INFO);
	config = config_create("cpu.config");
	
	// buscamos datos en config y conectamos con memoria
	ip = buscar("IP_MEMORIA");
	puerto = buscar("PUERTO_MEMORIA");
	//conexion = crear_conexion(ip, puerto, "Memoria"); 
	//enviar_mensaje("Saludos desde la cpu", conexion);

	//tambien sera servidor, con el kernel como cliente
	puerto = buscar("PUERTO_ESCUCHA_DISPATCH");
	socket_servidor = iniciar_servidor(puerto, "CPU");
	socket_cliente = esperar_cliente("Kernel", socket_servidor);
	pthread_create(&hilo_kernel, NULL, interactuar_dispatch, (void*)socket_cliente);
	



	char* buffer;
	sInstruccion instruccion;
	while(1){
		pthread_mutex_lock(&mProceso);
		buffer = fetch();
		instruccion = decode(buffer);
		execute(instruccion);
		pthread_mutex_unlock(&mProceso);

	}
	//liberar_conexion(conexion);
    log_destroy(logger);
    config_destroy(config);
	pthread_join(hilo_kernel, NULL);
    return 0;

}

