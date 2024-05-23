#include "kernel.h"


void inicializar_kernel(){
	logger = log_create("logKernel.log", "LOGS Kernel", 1, LOG_LEVEL_INFO);
	config = config_create("kernel.config");
	cNEW = queue_create();
	cREADY = queue_create();
	lBlocked = list_create();
	cEXIT = queue_create();
	multiprogramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
	quantum = config_get_int_value(config, "QUANTUM");
	idPCB = 1;
	pthread_mutex_init (&mNEW, NULL);
	pthread_mutex_init(&mREADY, NULL);
	pthread_mutex_init(&mBLOCKED, NULL);
	pthread_mutex_init (&mEXIT, NULL);
	sem_init(&semPCP, 0, 0);
	sem_init(&semPLP, 0, 0);
	sem_init(&semEXIT, 0, 0);
	sem_init(&sMultiprogramacion, 0, multiprogramacion);
	planificacion_activa = true;
}

void finalizar_kernel(){
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion_memoria);
	liberar_conexion(conexion_cpu_dispatch);
	liberar_conexion(conexion_cpu_interrupt);
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
int get_terminal(char* comando){
	if(!strcmp(comando, "INICIAR_PROCESO"))
		return INICIAR_PROCESO;
	if(!strcmp(comando, "FINALIZAR_PROCESO"))
		return FINALIZAR_PROCESO;
	if(!strcmp(comando, "EJECUTAR_SCRIPT"))
		return EJECUTAR_SCRIPT;
	if(!strcmp(comando, "DETENER_PLANIFICACION"))
		return DETENER_PLANIFICACION;
	if(!strcmp(comando, "INICIAR_PLANIFICACION"))
		return INICIAR_PLANIFICACION;
	if(!strcmp(comando, "PROCESO_ESTADO"))	
		return PROCESO_ESTADO;
	return -1;
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

// FUNCIONES DE CONSOLA

void interactuar_consola(char* buffer){
	char** mensaje = string_split(buffer, " ");
	int consola = get_terminal(mensaje[0]);
	switch (consola){
		case INICIAR_PROCESO:
			crear_proceso (mensaje[1]);
			break;
		case FINALIZAR_PROCESO:
			log_info(logger, "Proceso finalizado");
			break;
		case EJECUTAR_SCRIPT:
			ejecutar_script(mensaje[1]);
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
	free (mensaje[0]);
	free(mensaje);
}

void crear_proceso (char* path){
	if(!planificacion_activa)
		return;
	sProceso* proceso = malloc(sizeof (sProceso));
	proceso->pcb.estado=NEW;
	proceso->pcb.pc=0;
	proceso->pcb.quantum=quantum;
	proceso->multifuncion=path;
	strcpy(proceso->multifuncion, path);
	proceso->pcb.pid=idPCB;
	idPCB++;
	log_nuevoProceso(proceso->pcb.pid);
	pthread_mutex_lock(&mNEW);
	queue_push(cNEW, proceso);
	pthread_mutex_unlock(&mNEW);
	sem_post(&semPLP);
}

void detener_planificacion(){
	if(planificacion_activa){
		pthread_mutex_lock(&mNEW);
		pthread_mutex_lock(&mREADY);
		pthread_mutex_lock(&mBLOCKED);
		pthread_mutex_lock(&mEXIT);
		planificacion_activa = false;
	}
}

void iniciar_planificacion(){
	if(!planificacion_activa){
		pthread_mutex_unlock(&mNEW);
		pthread_mutex_unlock(&mREADY);
		pthread_mutex_unlock(&mBLOCKED);
		pthread_mutex_unlock(&mEXIT);
		planificacion_activa = true;
	}
}

void ejecutar_script(char* path){
	FILE* script = fopen(path, "r");
	if(script == NULL){
		log_info(logger, "No se pudo abrir el archivo");
		return;
	}
	char* buffer = malloc(50);
	while(!feof(script)){
		buffer = fgets(buffer, 50, script);
		buffer[strcspn(buffer, "\n")]=0;
		interactuar_consola(buffer);
	}
	free (buffer);
	fclose(script);
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
		free (proceso->multifuncion);
		if(proceso->pcb.instrucciones == NULL){
			matadero(proceso, "La memoria no pudo abrir el archivo");
			continue;
		}
		proceso->pcb.estado=READY;
		log_cambioEstado(proceso->pcb.pid, NEW, READY);
		pthread_mutex_lock(&mREADY);
		queue_push(cREADY, proceso);
		pthread_mutex_unlock(&mREADY);
		sem_post(&semPCP);
	}
}

char** enviar_proceso(char* path){	
	enviar_string(path, conexion_memoria, NUEVO_PROCESO);
	if(recibir_operacion(conexion_memoria) != NUEVO_PROCESO){
		return NULL;
	}
	return recibir_puntero (conexion_memoria);
}

void matadero (sProceso* proceso, char* motivo){
	log_cambioEstado(proceso->pcb.pid, proceso->pcb.estado, FINISHED);
	proceso->pcb.estado=FINISHED;
	proceso->multifuncion = motivo;
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
	enviar_puntero(proceso->pcb.instrucciones, conexion_memoria, FINALIZACION);
	log_finalizacion(proceso->pcb.pid, proceso->multifuncion);
	free(proceso);
	sem_post(&sMultiprogramacion);
	}
}

void planificadorCP_FIFO(){
	sProceso* proceso;
	int motivo;
	int size;
	pthread_t hilo_IO; //usamos para crearle un hilo a cada instancia de IO
	while (1){
		sem_wait(&semPCP);

		despachar_a_running();

		//me quedo esperando que vuelva y veo porque volvio
		motivo = recibir_operacion(conexion_cpu_dispatch);
		proceso->pcb=pcb_deserializar(conexion_cpu_dispatch);

		switch(motivo){
			case FINALIZACION:
				matadero(proceso, "Finalizo");
				break;
			case IO:
				log_cambioEstado(proceso->pcb.pid, RUNNING, BLOCKED);
				proceso->pcb.estado=BLOCKED;

				if(recibir_operacion(conexion_cpu_dispatch) != IO)
					matadero(proceso, "No coinciden los códigos de salida");

				proceso->multifuncion = recibir_buffer(&size, conexion_cpu_dispatch);

				pthread_mutex_lock(&mBLOCKED);
				list_add(lBlocked, proceso);
				pthread_mutex_unlock(&mBLOCKED);

				pthread_create(&hilo_IO, NULL, atender_solicitud_IO, (void*)proceso);
				break;
			default:
				matadero(proceso, "Envio codigo de salida no valido");
				break;
		}
	}
}

void despachar_a_running() {
	//despacha a running al primero que este en la lista de ready
	sProceso* proceso;

	pthread_mutex_lock(&mREADY);
	proceso = queue_pop(cREADY); 
	pthread_mutex_unlock(&mREADY);

	log_cambioEstado(proceso->pcb.pid, READY, RUNNING);
	proceso->pcb.estado=RUNNING;
	enviar_pcb(proceso->pcb, conexion_cpu_dispatch, PCB);
}

void setear_timer(sProceso* proceso) {
	sleep(proceso->pcb.quantum / 1000); // divido para pasar de milisegs a segs (es lo q toma sleep)
	enviar_int (proceso->pcb.pid, conexion_cpu_interrupt, FIN_DE_QUANTUM);
}

void planificadorCP_RR(){
	sProceso* proceso;
	int motivo;
	int size;
	pthread_t hilo_IO; //usamos para crearle un hilo a cada instancia de IO
	pthread_t hilo_timer;

	while (1) {
		sem_wait(&semPCP);

		pthread_mutex_lock(&mREADY);
		proceso = queue_pop(cREADY); 
		pthread_mutex_unlock(&mREADY);

		log_cambioEstado(proceso->pcb.pid, READY, RUNNING);
		proceso->pcb.estado=RUNNING;
		enviar_pcb(proceso->pcb, conexion_cpu_dispatch, PCB);

		pthread_create(&hilo_timer, NULL, setear_timer, (void *) proceso);

		motivo = recibir_operacion(conexion_cpu_dispatch);
		proceso->pcb=pcb_deserializar(conexion_cpu_dispatch);

		switch(motivo){
			case FINALIZACION:
				pthread_cancel(hilo_timer); //cancelamos el hilo de timer pq volvimos por otro motivo
				matadero(proceso, "Finalizo");
				break;
			case IO:
				pthread_cancel(hilo_timer);
				log_cambioEstado(proceso->pcb.pid, RUNNING, BLOCKED);
				proceso->pcb.estado=BLOCKED;

				if(recibir_operacion(conexion_cpu_dispatch) != IO)
					matadero(proceso, "No coinciden los códigos de salida");

				proceso->multifuncion = recibir_buffer(&size, conexion_cpu_dispatch);

				pthread_mutex_lock(&mBLOCKED);
				list_add(lBlocked, proceso);
				pthread_mutex_unlock(&mBLOCKED);
				
				pthread_create(&hilo_IO, NULL, atender_solicitud_IO, (void*)proceso);
				break;
			case FIN_DE_QUANTUM:
				log_cambioEstado(proceso->pcb.pid, RUNNING, READY);
				proceso->pcb.estado=READY;

				pthread_mutex_lock(&mREADY);
				queue_push(cREADY, proceso);
				pthread_mutex_unlock(&mREADY);

				sem_post(&semPCP); // aviso que ya se puede despachar otro
				break;
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

void escuchar_conexiones_IO(int socket_server) {
	lista_conexiones_IO = list_create();
	int socket;
	while(1){
		socket = accept(socket_server, NULL, NULL); // aca uso accept en vez de esperar_cliente pq todavia no se el nombre
		switch (recibir_operacion(socket)) {
			case NUEVA_IO:
				int size;
				char* nombre = recibir_buffer(&size, socket);

				t_conexion *conexion = malloc(sizeof(t_conexion));
				conexion->nombre = string_new();
				strcpy(conexion->nombre, nombre);
				conexion->socket = socket;

				pthread_mutex_lock(&mCONEXIONES);
				list_add(lista_conexiones_IO, conexion);
				pthread_mutex_unlock(&mCONEXIONES);
				
				log_info(logger, "Se conecto la IO: %s", nombre);
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
		pthread_mutex_lock(&mBLOCKED);
		list_remove_element(lBlocked, proceso);
		pthread_mutex_unlock(&mBLOCKED);
		free(proceso->multifuncion);
		matadero(proceso, "Se intento comunicar con una IO no conectada");
		string_array_destroy(nombre_y_operacion);
		return;
	}

	// le mandamos a la instancia encontrada la operacion
	enviar_string(nombre_y_operacion[1], IO_seleccionada->socket, OPERACION_IO);

	// nos quedamos escuchando la respuesta
	int op = recibir_operacion(IO_seleccionada->socket);

	if (op == -1) { // en caso de que la operacion no sea valida
		pthread_mutex_lock(&mBLOCKED);
		list_remove_element(lBlocked, proceso);
		pthread_mutex_unlock(&mBLOCKED);
		free(proceso->multifuncion);
		matadero(proceso, "La IO no admite la operacion solicitada");
		string_array_destroy(nombre_y_operacion);
		return;
	}

	// por ahora solo tenemos IO generica, que devuelve mensaje
	// despues vemos que pueden devolver las otras y tratamos esos casos
	recibir_mensaje(IO_seleccionada->socket);

	
	// REPETIDO DE PLP: LO VOLVEMOS A READY DESPUES DE SU IO
	pthread_mutex_lock(&mBLOCKED);
	list_remove_element(lBlocked, proceso);
	pthread_mutex_unlock(&mBLOCKED);
	log_cambioEstado(proceso->pcb.pid, BLOCKED, READY);
	proceso->pcb.estado=READY;
	free(proceso->multifuncion);
	pthread_mutex_lock(&mREADY);
	queue_push(cREADY, proceso);
	pthread_mutex_unlock(&mREADY);
	sem_post(&semPCP); //avisamos al dispatcher q volvio a ready
	string_array_destroy(nombre_y_operacion);
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