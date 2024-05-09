#include "kernel.h"


void inicializar_kernel(){
	logger = log_create("logKernel.log", "LOGS Kernel", 1, LOG_LEVEL_INFO);
	config = config_create("kernel.config");
	cNEW = queue_create();
	cREADY = queue_create();
	cEXIT = queue_create();
	multiprogramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
	quantum = config_get_int_value(config, "QUANTUM");
	idPCB = 1;
	pthread_mutex_init (&mNEW, NULL);
	pthread_mutex_init(&mREADY, NULL);
	pthread_mutex_init (&mEXIT, NULL);
	sem_init(&semPCP, 0, 0);
	sem_init(&semPLP, 0, 0);
	sem_init(&semEXIT, 0, 0);
	sem_init(&sMultiprogramacion, 0, multiprogramacion);
}

void finalizar_kernel(){
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion_memoria);
	liberar_conexion(conexion_cpu);
	queue_destroy(cNEW);
	queue_destroy(cREADY);
	queue_destroy(cEXIT);
	pthread_mutex_destroy(&mNEW);
	pthread_mutex_destroy(&mREADY);
	pthread_mutex_destroy(&mEXIT);
	sem_destroy(&semPLP);
	sem_destroy(&semPCP);
	sem_destroy(&semEXIT);
	sem_destroy(&sMultiprogramacion);
}

char* get_estado(int estado){
	switch(estado){
		case NEW:
			return "NEW";
		case READY:
			return "READY";
		case RUNNING:
			return "RUNNING";
		case BLOCKED:
			return "BLOCKED";
		case FINISHED:
			return "FINISHED";
		default:
			return "error";
	}
}

void crear_proceso (char* path){
	sProceso* proceso = malloc(sizeof (sProceso));
	proceso->pcb.estado=NEW;
	proceso->pcb.pc=0;
	proceso->pcb.quantum=quantum;
	proceso->multifuncion=string_new();
	strcpy(proceso->multifuncion, path);
	proceso->pcb.pid=idPCB;
	idPCB++;
	log_nuevoProceso(proceso->pcb.pid);
	pthread_mutex_lock(&mNEW);
	queue_push(cNEW, proceso);
	pthread_mutex_unlock(&mNEW);
	sem_post(&semPLP);
}

void PLP(){
	sProceso* proceso;
	while(1){
		sem_wait(&semPLP);
		pthread_mutex_lock(&mNEW);
		proceso = queue_pop(cNEW);
		pthread_mutex_unlock(&mNEW);
		sem_wait(&sMultiprogramacion);
		proceso->pcb.instrucciones = enviar_proceso(proceso->multifuncion);
		proceso->pcb.estado=READY;
		log_cambioEstado(proceso->pcb.pid, NEW, READY);
		pthread_mutex_lock(&mREADY);
		queue_push(cREADY, proceso);
		pthread_mutex_unlock(&mREADY);
		sem_post(&semPCP);
	}
	
}

void carnicero(){
	sProceso* proceso;
	while(1){
	sem_wait(&semEXIT);
	pthread_mutex_lock(&mEXIT);
	proceso = queue_pop(cEXIT);
	pthread_mutex_unlock(&mEXIT);
	//liberar_memoria(proceso->pcb->instrucciones); FALTA PROGRAMAR
	log_finalizacion(proceso->pcb.pid, proceso->multifuncion);
	free(proceso->multifuncion);
	free(proceso);
	sem_post(&sMultiprogramacion);
	}
}

char** enviar_proceso(char* path){	
	enviar_string(path, conexion_memoria, NUEVO_PROCESO);
	int size;
	return NULL; //recibir_buffer (&size, conexion_memoria);
}

void syscall_IO_GEN_SLEEP(int socket, char* tiempo) {
	// aca se podria de ver como mandar el tiempo como int/float
	enviar_string(tiempo, socket, SLEEP);

	//se asume que tras sleepear el IO devuelve un mensaje de exito
	int operacion = recibir_operacion(socket);
	if (operacion == MENSAJE)
		recibir_mensaje(socket);
}

void planificadorCP(){
	sProceso* proceso;
	int motivo;
	while (1){
		sem_wait(&semPCP);
		pthread_mutex_lock(&mREADY);
		proceso = queue_pop(cREADY); 
		pthread_mutex_unlock(&mREADY);
		enviar_pcb(proceso->pcb, conexion_cpu, PCB); 
		log_cambioEstado(proceso->pcb.pid, READY, RUNNING);
		motivo = recibir_operacion(conexion_cpu);
		proceso->pcb=pcb_deserializar(conexion_cpu);
		switch(motivo){
			case FINALIZACION:
				log_cambioEstado(proceso->pcb.pid, RUNNING, FINISHED);
				strcpy(proceso->multifuncion, "Finalizó");
				pthread_mutex_lock(&mEXIT);
				queue_push(cEXIT, proceso);
				pthread_mutex_unlock(&mEXIT);
				sem_post(&semEXIT);
				break;
			//case IO:
			default:
				log_error(logger, "Motivo inválido para salir");
				break;
		}
	}
}


//LOGS OBLIGATORIOS
void log_nuevoProceso (int pid){
	log_info(logger, "Se creo el proceso %d en NEW", pid);
}

void log_cambioEstado (int pid, int eAnterior, int eActual){
	log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pid, get_estado(eAnterior), get_estado(eActual));
}
void log_finalizacion(int pid, char* motivo){
	log_info(logger, "Finaliza el proceso %d - Motivo: %s", pid, motivo);
}