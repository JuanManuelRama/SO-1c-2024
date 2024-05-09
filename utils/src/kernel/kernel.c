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

void matadero (sProceso* proceso, char* motivo){
	log_cambioEstado(proceso->pcb.pid, RUNNING, FINISHED);
	strcpy(proceso->multifuncion, motivo);
	pthread_mutex_lock(&mEXIT);
	queue_push(cEXIT, proceso);
	pthread_mutex_unlock(&mEXIT);
	sem_post(&semEXIT);
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
	pthread_t hilo_IO; //usamos para crearle un hilo a cada instancia de IO
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
				matadero(proceso, "Finalizo");
				break;
			case IO:
				log_cambioEstado(proceso->pcb.pid, RUNNING, BLOCKED);
				int size;
				char* buffer = recibir_buffer(&size, conexion_cpu);
				strcpy(proceso->multifuncion, buffer);

				//mutex
				list_add(lBlocked, proceso);

				pthread_create(&hilo_IO, NULL, atender_solicitud_IO, (void*)proceso);
			default:
				matadero(proceso, "Envio codigo de salida no valido");
				break;
		}
	}
}
/*
Al recibir una petición de I/O de parte de la CPU 
primero se deberá validar que exista y esté conectada la interfaz solicitada,
 en caso contrario, se deberá enviar el proceso a EXIT.
En caso de que la interfaz exista y esté conectada, se deberá validar que la interfaz 
admite la operación solicitada, en caso de que no sea así, se deberá enviar el proceso a EXIT.
*/

void escuchar_conexiones_IO(int socket_IO) {
	lista_conexiones_IO = list_create();
	int socket;
	while(1){
		switch (recibir_operacion(socket)) {
			case NUEVA_IO:
				int size;
				char* nombre = recibir_buffer(&size, socket_IO);
				
				socket = esperar_cliente(nombre, kernel_servidor);

				t_conexion *conexion = malloc(sizeof(t_conexion));
				conexion->nombre = nombre;
				conexion->socket = socket;

				//FALTA MUTEX
				list_add(lista_conexiones_IO, conexion);
			// case STDIN, etc.
		}
	}
}

void atender_solicitud_IO(sProceso* proceso){
	char** nombre_y_operacion = string_n_split(proceso->multifuncion, 2, " ");

	// funcionalidad propia de gcc, inner function
	bool existe_conexion(void* elem) {
		t_conexion *conexion = (t_conexion*)elem;
		return !strcmp(conexion->nombre, nombre_y_operacion[0]);
	}

	t_conexion* IO_seleccionada = list_find(lista_conexiones_IO, existe_conexion);

	if (IO_seleccionada == NULL) {
		//mutex
		list_remove_element(lBlocked, proceso);
		matadero(proceso, "Se intento comunicar con una IO no conectada");
		return;
	}

	// le mandamos a la instancia encontrada la operacion
	enviar_string(nombre_y_operacion[1], IO_seleccionada->socket, OPERACION_IO);

	// nos quedamos escuchando la respuesta
	int op = recibir_operacion(IO_seleccionada->socket);

	if (op == -1) { // en caso de que la operacion no sea valida
		//mutex
		list_remove_element(lBlocked, proceso);

		strcpy(proceso->multifuncion, "Pidio una operacion no valida");
		
		pthread_mutex_lock(&mEXIT);
		queue_push(cEXIT, proceso);
		pthread_mutex_unlock(&mEXIT);
		sem_post(&semEXIT);

		return;
	}

	// por ahora solo tenemos IO generica, que devuelve mensaje
	// despues vemos que pueden devolver las otras y tratamos esos casos
	recibir_mensaje(IO_seleccionada->socket);

	
	// REPETIDO DE PLP: LO VOLVEMOS A READY DESPUES DE SU IO
	log_cambioEstado(proceso->pcb.pid, BLOCKED, READY);

	pthread_mutex_lock(&mREADY);
	queue_push(cREADY, proceso);
	pthread_mutex_unlock(&mREADY);

	sem_post(&semPCP); //avisamos al dispatcher q volvio a ready
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