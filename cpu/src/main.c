#include <cpu/cpu.h>


t_log* logger;
t_config* config;
t_pcb pcb;
int seVa;

int main() {
	logger = log_create("logCpu.log", "LOGS CPU", 1, LOG_LEVEL_INFO);
	config = config_create("cpu.config");
    int conexion;
	int socket_servidor;
	int socket_dispatch;
	char* ip;
	char* puerto;
	char* buffer;
	pthread_t hilo_kernel;
	sInstruccion instruccion;

	logger = log_create("logCpu.log", "LOGS CPU", 1, LOG_LEVEL_INFO);
	config = config_create("cpu.config");
	seVa = false;
	// buscamos datos en config y conectamos con memoria
	ip = buscar("IP_MEMORIA");
	puerto = buscar("PUERTO_MEMORIA");
	//conexion = crear_conexion(ip, puerto, "Memoria"); 
	//enviar_mensaje("Saludos desde la cpu", conexion);

	//tambien sera servidor, con el kernel como cliente
	puerto = buscar("PUERTO_ESCUCHA_DISPATCH");
	socket_servidor = iniciar_servidor(puerto, "CPU");
	socket_dispatch = esperar_cliente("Kernel", socket_servidor);
	




	while(1){
		recibir_operacion(socket_dispatch);
		pcb=pcb_deserializar(socket_dispatch);
		while(!seVa){
			buffer = fetch();
			instruccion = decode(buffer);
			execute(instruccion);
			pcb.pc++;
		}
		enviar_pcb(pcb, socket_dispatch, seVa);
		seVa=false;
	}
	//liberar_conexion(conexion);
    log_destroy(logger);
    config_destroy(config);
	pthread_join(hilo_kernel, NULL);
    return 0;

}

