#include <cpu/cpu.h>


t_log* logger;
t_config* config;
t_pcb pcb;

void interactuar_Kernel(int socket_cliente){
		while (1) {
		int cod_op = recibir_operacion(socket_cliente);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(socket_cliente);
			break;
		case PCB:
			log_info(logger, "llego pcb, ahora deserealizo:");
			t_pcb generica = pcb_deserializar(socket_cliente);
			log_info(logger, "Proces ID: %d", generica.pid);
			log_info(logger, "Program Counter: %d", generica.pc);
			log_info(logger, "Quantum: %d", generica.quantum);
			log_info(logger, "Estado: %d", generica.estado);
			break;
		case -1:
			log_error(logger, "el cliente se desconecto");
			return EXIT_FAILURE;
		default:
			log_warning(logger,"Operacion no esperada por parte de este cliente");
			break;
		}
	}
}

int main() {
	logger = log_create("logCpu.log", "LOGS CPU", 1, LOG_LEVEL_INFO);
	config = config_create("cpu.config");
    int conexion;
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
	//conexion = crear_conexion(ip, puerto, "Memoria"); 
	//enviar_mensaje("Saludos desde la cpu", conexion);

	//tambien sera servidor, con el kernel como cliente
	puerto = buscar("PUERTO_ESCUCHA_DISPATCH");
	socket_servidor = iniciar_servidor(puerto, "CPU");
	socket_kernel = esperar_cliente("Kernel", socket_servidor);
	pthread_create(&hilo_kernel, NULL, interactuar_Kernel, (void*)socket_kernel);
	
	pthread_join(hilo_kernel, NULL);

    //liberar_conexion(conexion);
    log_destroy(logger);
    config_destroy(config);

	char* buffer;
	sInstruccion instruccion;
	pcb.pid=1;
	while(1){
		buffer = fetch();
		instruccion = decode(buffer);
		execute(instruccion);

	}
    return 0;

}

