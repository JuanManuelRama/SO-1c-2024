#include "kernel.h"


void inicializar_kernel(){
	logger = log_create("logKernel.log", "LOGS Kernel", 1, LOG_LEVEL_INFO);
	config = config_create("kernel.config");
	cNEW = queue_create();
	cREADY = queue_create();
	multiprogramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
	idPCB = 1;
	pthread_mutex_init(&mREADY, NULL);
	pthread_mutex_init (&mNEW, NULL);
	sem_init(&semPCP, 0, 0);
	sem_init(&semPLP, 0, 0);
	sem_init(&sMultiprogramacion, 0, multiprogramacion);
}

void finalizar_kernel(){
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion_memoria);
	liberar_conexion(conexion_cpu);
	pthread_mutex_destroy(&mNEW);
	pthread_mutex_destroy(&mREADY);
	sem_destroy(&semPLP);
	sem_destroy(&semPCP);
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
	sPLP* proceso = malloc(sizeof (semPLP));
	proceso->pcb = malloc (sizeof(t_pcb));
	proceso->pcb->estado=NEW;
	proceso->pcb->pc =0;
	proceso->pcb->pid=idPCB;
	idPCB++;
	proceso->path=path;
	log_nuevoProceso(proceso->pcb->pid);
	
	//proceso->quantum=quantum AÚN NO ESTÁ DEFINIDO
	pthread_mutex_lock(&mNEW);
	queue_push(cNEW, proceso);
	pthread_mutex_unlock(&mNEW);
	sem_post(&semPLP);
}

void PLP(){
	sPLP* proceso;
	while(1){
		sem_wait(&semPLP);
		pthread_mutex_lock(&mNEW);
		proceso = queue_pop(cNEW);
		pthread_mutex_unlock(&mNEW);
		sem_wait(&sMultiprogramacion);
		proceso->pcb->instrucciones = enviar_proceso(proceso->path);
		proceso->pcb->estado=READY;
		log_cambioEstado(proceso->pcb->pid, NEW, READY);
		pthread_mutex_lock(&mREADY);
		queue_push(cREADY, proceso->pcb);
		pthread_mutex_unlock(&mREADY);
		sem_post(&semPCP);
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

void planificadorCP(int cpu){
	while (1){
		sem_wait(&semPCP);
		pthread_mutex_lock(&mREADY);
		t_pcb* proceso = queue_pop(cREADY); 
		pthread_mutex_unlock(&mREADY);
		enviar_pcb(*proceso, conexion_cpu, PCB); 
	}
}


//LOGS OBLIGATORIOS
void log_nuevoProceso (int pid){
	log_info(logger, "Se creo el proceso %d en NEW", pid);
}

void log_cambioEstado (int pid, int eAnterior, int eActual){
	log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pid, get_estado(eAnterior), get_estado(eActual));
}

t_pcb crear_pcb (int pid, int pc, int quantum, t_registros registros, int estado, char** instrucciones){
	t_pcb pcb;
	pcb.pid = pid;
	pcb.pc = pc;
	pcb.quantum = quantum;
	pcb.registros = registros;
	pcb.estado = estado;
	pcb.instrucciones = instrucciones;
	return pcb;
}