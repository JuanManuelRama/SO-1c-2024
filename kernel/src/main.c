#include <kernel/kernel.h>

//Variables globales
t_log* logger;
t_config* config;
t_queue* cProcesos;
pthread_mutex_t scProceso;
sem_t sMultiprogramacion;
t_pcb generica;
int conexion_memoria;
int conexion_cpu;
int idPCB;
int multiprogramacion;

// de prueba, despues se borra.
void prueba_IO_GEN(int socket);

int main() {
	int kernel_servidor;
	int socket_IO;
	char* ip;
	char* puerto;
	pthread_t hilo_IO;
	pthread_t hilo_pcp;

	inicializar_kernel();
    
	// buscamos datos en config y conectamos a memoria
	ip = buscar("IP_MEMORIA");
	puerto = buscar ("PUERTO_MEMORIA");
	//conexion_memoria = crear_conexion(ip, puerto, "Memoria"); 
	//enviar_mensaje("Saludos desde el Kernel",conexion_memoria);

	// buscamos datos en config y conectamos a cpu
	ip = buscar("IP_CPU");
	puerto = buscar("PUERTO_CPU_DISPATCH");
	conexion_cpu = crear_conexion(ip, puerto, "CPU");
	t_registros registro;
	generica = crear_pcb(1, 10, 2, registro, 1, NULL);

	log_info(logger, "Proces ID: %d", generica.pid);
	log_info(logger, "Program Counter: %d", generica.pc);
	log_info(logger, "Quantum: %d", generica.quantum);
	log_info(logger, "Estado: %d", generica.estado);

	enviar_pcb(generica, conexion_cpu, PCB);
	enviar_mensaje("Saludos desde el Kernel",conexion_cpu);

	//tambien sera servidor, con el I/O como cliente
	//puerto = buscar("PUERTO_ESCUCHA");
	//kernel_servidor = iniciar_servidor(puerto, "Kernel");

	//socket_IO = esperar_cliente("I/O", kernel_servidor);
	//pthread_create(&hilo_IO, NULL, prueba_IO_GEN, (void*)socket_IO);

	pthread_create(&hilo_pcp, NULL, planificadorCP, NULL);

	char* buffer;
	char** mensaje;
	int consola;
	while(1){
		buffer  = readline(">");
		mensaje = string_split(buffer, " ");
		consola = strcmp ("INICIAR_PROCESO", mensaje[0]);
		switch (consola){
			case INICIAR_PROCESO:
				crear_proceso (mensaje[1]);
				break;
			case FINALIZAR_PROCESO:
				log_info(logger, "Proceso finalizado");
				break;
			case EJECUTAR_SCRIPT:
				log_info(logger, "Script ejecutado");
				break;
			case DETENER_PLANIFICACION:
				log_info(logger, "Planificacion detenida");
				break;
			case INICIAR_PLANIFICACION:
				log_info(logger, "Planificacion iniciada");
				break;
			case PROCESO_ESTADO:
				log_info(logger, "El estado del proceso es:");
				break;
			default:
				log_info(logger, "Código invalido");
				break;
		}
	}
	pthread_join(hilo_IO, NULL);
	finalizar_kernel();

	
    return 0;
}

void prueba_IO_GEN(int socket) {
	for (int i = 0; i<5; i++) {
		syscall_IO_GEN_SLEEP(socket, "10");
		sleep(15);
	}
}