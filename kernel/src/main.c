#include <kernel/kernel.h>

//Variables globales
t_log* logger;
t_config* config;
t_queue* cNEW;
t_queue* cREADY;
t_queue* cEXIT;
t_list* lBlocked;
pthread_mutex_t mNEW;
pthread_mutex_t mREADY;
pthread_mutex_t mBLOCKED;
pthread_mutex_t mEXIT;
sem_t semPCP;
sem_t semPLP;
sem_t semEXIT;
sem_t sMultiprogramacion;
t_pcb generica;
int conexion_memoria;
int conexion_cpu;
int idPCB;
int multiprogramacion;
int quantum;
int kernel_servidor;
bool planificacion_activa;
t_list *lista_conexiones_IO;

// de prueba, despues se borra.


int main() {

	int socket_IO;
	char* ip;
	char* puerto;
	pthread_t hilo_IO;
	pthread_t hilo_pcp;
	pthread_t hilo_carnicero;

	inicializar_kernel();
    
	// buscamos datos en config y conectamos a memoria
	ip = buscar("IP_MEMORIA");
	puerto = buscar ("PUERTO_MEMORIA");
	conexion_memoria = crear_conexion(ip, puerto, "Memoria"); 
	enviar_mensaje("Saludos desde el Kernel",conexion_memoria);

	// buscamos datos en config y conectamos a cpu
	ip = buscar("IP_CPU");
	puerto = buscar("PUERTO_CPU_DISPATCH");
	conexion_cpu = crear_conexion(ip, puerto, "CPU");

	//tambien sera servidor, con el I/O como cliente
	puerto = buscar("PUERTO_ESCUCHA");
	kernel_servidor = iniciar_servidor(puerto, "Kernel");
	pthread_create(&hilo_IO, NULL, escuchar_conexiones_IO, (void*)kernel_servidor);

	pthread_create(&hilo_pcp, NULL, PLP, NULL);
	pthread_create(&hilo_carnicero, NULL, carnicero, NULL);
	pthread_create(&hilo_pcp, NULL, planificadorCP, NULL);

	char* buffer;
	char** mensaje;
	int consola;

		while(1){
		buffer  = readline(">");
		mensaje = string_split(buffer, " ");
		consola = get_terminal(mensaje[0]);
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
				detener_planificacion();
				break;
			case INICIAR_PLANIFICACION:
				iniciar_planificacion();
				break;
			case PROCESO_ESTADO:
				log_info(logger, "El estado del proceso es:");
				break;
			default:
				log_info(logger, "Código invalido");
				break;
		}
		free (buffer);
		free (mensaje[0]);
		free(mensaje);
	}
	pthread_join(hilo_IO, NULL);
	finalizar_kernel();

	
    return 0;
}

