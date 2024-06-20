#include <kernel/kernel.h>

//Variables globales
t_log* logger;
t_config* config;
t_queue* cNEW;
t_queue* cREADY;
t_queue* cEXIT;
t_queue* cREADY_PLUS; // usado para VRR
t_list* lBlocked;
pthread_mutex_t mNEW;
pthread_mutex_t mREADY;
pthread_mutex_t mREADY_PLUS; // usado para VRR
pthread_mutex_t mRUNNING;
pthread_mutex_t mBLOCKED;
pthread_mutex_t mEXIT;
pthread_mutex_t mCONEXIONES;
sem_t semPCP;
sem_t semPLP;
sem_t semEXIT;
sem_t sMultiprogramacion;
t_pcb generica;
int conexion_memoria;
int conexion_cpu_dispatch;
int conexion_cpu_interrupt;
int idPCB;
int multiprogramacion;
int quantum;
int tam_pagina;
int kernel_servidor;
bool planificacion_activa;
bool planiEsVrr; // medio tosco el flag global, se puede pensar otra forma (se usa para vrr)
int pidRunning;
t_list *lista_conexiones_IO;

t_recurso* recursos;
int cantRecursos;
int instanciasUtilizadas;
char* recursoPedido;

int main() {

	int socket_IO;
	char* ip;
	char* puerto;
	char* algoritmoCortoPlazo;
	pthread_t hilo_IO;
	pthread_t hilo_pcp;
	pthread_t hilo_plp;
	pthread_t hilo_carnicero;

	inicializar_kernel();
    
	// buscamos datos en config y conectamos a memoria
	ip = buscar("IP_MEMORIA");
	puerto = buscar ("PUERTO_MEMORIA");
	conexion_memoria = crear_conexion(ip, puerto, "Memoria"); 
	tam_pagina = recibir_operacion(conexion_memoria);
	log_info(logger, "tam_pag = %i", tam_pagina);
	enviar_mensaje("Saludos desde el Kernel",conexion_memoria);

	// buscamos datos en config y conectamos a cpu
	ip = buscar("IP_CPU");
	puerto = buscar("PUERTO_CPU_DISPATCH");
	conexion_cpu_dispatch = crear_conexion(ip, puerto, "CPU dispatch");

	ip = buscar("IP_CPU");
	puerto = buscar("PUERTO_CPU_INTERRUPT");
	conexion_cpu_interrupt = crear_conexion(ip, puerto, "CPU interrupt");

	//tambien sera servidor, con el I/O como cliente
	puerto = buscar("PUERTO_ESCUCHA");
	kernel_servidor = iniciar_servidor(puerto, "Kernel");
	pthread_create(&hilo_IO, NULL, escuchar_conexiones_IO, (void*)kernel_servidor);

	pthread_create(&hilo_plp, NULL, PLP, NULL);
	pthread_create(&hilo_carnicero, NULL, carnicero, NULL);

	algoritmoCortoPlazo = buscar("ALGORITMO_PLANIFICACION");
	planiEsVrr = false;
	if (!strcmp(algoritmoCortoPlazo, "FIFO")) {
		pthread_create(&hilo_pcp, NULL, planificadorCP_FIFO, NULL);
	} else if (!strcmp(algoritmoCortoPlazo, "RR")) {
		pthread_create(&hilo_pcp, NULL, planificadorCP_RR, NULL);
	} else {
		planiEsVrr = true;
		pthread_create(&hilo_pcp, NULL, planificadorCP_VRR, NULL);
	}


	char* buffer;
	while(1){
	buffer=readline(">");
	interactuar_consola(buffer);
	free (buffer);
	}
		
	
	
	pthread_join(hilo_IO, NULL);
	finalizar_kernel();

	
    return 0;
}

