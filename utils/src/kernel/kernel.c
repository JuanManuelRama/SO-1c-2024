#include "kernel.h"


void inicializar_kernel(){
	logger = log_create("logKernel.log", "LOGS Kernel", 1, LOG_LEVEL_INFO);
	config = config_create("kernel.config");
	cProcesos = queue_create();
	multiprogramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
	idPCB = 1;
	pthread_mutex_init(&scProceso, NULL);
	pthread_mutex_lock(&scProceso);
	sem_init(&sMultiprogramacion, 0, multiprogramacion);
}

void finalizar_kernel(){
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion_memoria);
	liberar_conexion(conexion_cpu);
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
	t_pcb* proceso = malloc(sizeof (t_pcb));
	proceso->estado=NEW;
	proceso->pc=0;
	proceso->pid=idPCB;
	idPCB++;
	log_nuevoProceso(proceso->pid);
	//proceso->quantum=quantum AÚN NO ESTÁ DEFINIDO
	sem_wait(&sMultiprogramacion);
	//proceso->instrucciones = enviar_proceso(path);
	proceso->estado=READY;
	log_cambioEstado(proceso->pid, NEW, READY);
	queue_push(cProcesos, "hOLA");
	if(cProcesos->elements->elements_count==1)
		pthread_mutex_unlock(&scProceso);
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

void planificadorCP(int cpu){
	while (1){
		pthread_mutex_lock(&scProceso);
		t_pcb* proceso = queue_pop(cProcesos); 
		//enviar_pcb(conexion_cpu, *proceso) AUN NO ESTÁ PROGRAMADA
		if(!queue_is_empty(cProcesos))
			pthread_mutex_unlock(&scProceso);	//Si no está vacia, desbloquea para seguir
	}
}


//LOGS OBLIGATORIOS
void log_nuevoProceso (int pid){
	log_info(logger, "Se creo el proceso %d en NEW", pid);
}

void log_cambioEstado (int pid, int eAnterior, int eActual){
	log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pid, get_estado(eAnterior), get_estado(eActual));
}