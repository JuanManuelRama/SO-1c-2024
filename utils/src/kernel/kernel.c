#include "kernel.h"


void inicializar_kernel(){
	logger = log_create("logKernel.log", "LOGS Kernel", 1, LOG_LEVEL_INFO);
	config = config_create("kernel.config");
	cNEW = queue_create();
	cREADY = queue_create();
	cREADY_PLUS = queue_create(); // vrr
	lBlocked = list_create();
	cEXIT = queue_create();
	multiprogramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
	quantum = config_get_int_value(config, "QUANTUM");
	idPCB = 1;
	pthread_mutex_init (&mNEW, NULL);
	pthread_mutex_init(&mREADY, NULL);
	pthread_mutex_init(&mREADY_PLUS, NULL); // vrr (global? o inicializamos solo cuando plani = vrr?)
	pthread_mutex_init(&mREADY, NULL);
	pthread_mutex_init(&mBLOCKED, NULL);
	pthread_mutex_init (&mEXIT, NULL);
	sem_init(&semPCP, 0, 0);
	sem_init(&semPLP, 0, 0);
	sem_init(&semEXIT, 0, 0);
	sem_init(&sMultiprogramacion, 0, multiprogramacion);
	planificacion_activa = true;
	pidRunning = -1;

	char** arrayRecursos = config_get_array_value(config, "RECURSOS");
	char** instanciasRecursos = config_get_array_value(config, "INSTANCIAS_RECURSOS");
	cantRecursos = string_array_size(arrayRecursos);
	recursos = malloc(cantRecursos*sizeof(t_recurso));

	for (int i = 0; i < cantRecursos; i++) {
		t_recurso recursoActual;
		recursoActual.nombre = arrayRecursos[i];
		recursoActual.instancias = atoi(instanciasRecursos[i]);
		recursoActual.cBloqueados = queue_create();
		pthread_mutex_init(&(recursoActual.mutex), NULL);
		recursos[i] = recursoActual;
	}

}

void finalizar_kernel(){
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion_memoria);
	liberar_conexion(conexion_cpu_dispatch);
	liberar_conexion(conexion_cpu_interrupt);
	queue_destroy(cNEW);
	queue_destroy(cREADY);
	queue_destroy(cREADY_PLUS);
	list_destroy(lBlocked);
	queue_destroy(cEXIT);
	list_destroy(lista_conexiones_IO);
	pthread_mutex_destroy(&mNEW);
	pthread_mutex_destroy(&mREADY);
	pthread_mutex_destroy(&mREADY_PLUS);	
	pthread_mutex_destroy(&mRUNNING);
	pthread_mutex_destroy(&mBLOCKED);
	pthread_mutex_destroy(&mEXIT);
	sem_destroy(&semPLP);
	sem_destroy(&semPCP);
	sem_destroy(&semEXIT);
	sem_destroy(&sMultiprogramacion);
	for (int i = 0; i < cantRecursos; i++) {
        queue_destroy(recursos[i].cBloqueados);
		pthread_mutex_destroy(&(recursos[i].mutex));
    }
	free(recursos);
	exit(0);
}
fConsola get_terminal(char* comando){
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
	if(!strcmp(comando, "MULTIPROGRAMACION"))
		return MULTIPROGRAMACION;
	if (!strcmp(comando, "0"))
		finalizar_kernel();
	return -1;
}

char* get_estado(int estado){
	switch(estado){
		case NEW:
			return "NEW";
		case READY:
			return "READY";
		case READY_PLUS:
			return "READY PLUS";
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
	fConsola consola = get_terminal(mensaje[0]);
	switch (consola){
		case INICIAR_PROCESO:
			crear_proceso (mensaje[1]);
			free (mensaje[0]);
			free(mensaje);
			break;
		case FINALIZAR_PROCESO:
			finalizar_proceso(atoi(mensaje[1]));
			free (mensaje[0]);
			free(mensaje);
			break;
		case EJECUTAR_SCRIPT:
			ejecutar_script(mensaje[1]);
			string_array_destroy(mensaje);
			break;
		case DETENER_PLANIFICACION:
			detener_planificacion();
			string_array_destroy(mensaje);
			break;
		case INICIAR_PLANIFICACION:
			iniciar_planificacion();
			string_array_destroy(mensaje);
			break;
		case MULTIPROGRAMACION:
			cambiar_multiprogramacion(atoi(mensaje[1]));
			string_array_destroy(mensaje);
			break;
		case PROCESO_ESTADO:
			proceso_estado();
			string_array_destroy(mensaje);
			break;
		default:
			log_info(logger, "Código invalido");
			string_array_destroy(mensaje);
			break;
	}

}

void crear_proceso (char* path){
	if(!planificacion_activa)
		return;
	sProceso* proceso = malloc(sizeof (sProceso));
	proceso->pcb.estado=NEW;
	proceso->pcb.pc=0;
	proceso->pcb.registros.PC=0;
	proceso->pcb.quantum=quantum;
	proceso->recursos=list_create();
	proceso->multifuncion=path;
	proceso->pcb.pid=idPCB;
	idPCB++;
	log_nuevoProceso(proceso->pcb.pid);
	pthread_mutex_lock(&mNEW);
	queue_push(cNEW, proceso);
	pthread_mutex_unlock(&mNEW);
	sem_post(&semPLP);
}

void finalizar_proceso(int pid){
	if(pid==pidRunning){
		enviar_int(pid, conexion_cpu_interrupt, INTERRUPCION);
		return;
	}
	detener_planificacion();
	if(buscar_proceso_en_lista(cNEW->elements, pid))
		return;
	if(buscar_proceso_en_lista(cREADY->elements, pid))
		return;
	if(buscar_proceso_en_lista(cREADY_PLUS->elements, pid))
		return;
	if(buscar_proceso_en_lista(lBlocked, pid))
		return;
	for(int i=0; i<cantRecursos; i++){
		if(buscar_proceso_en_lista(recursos[i].cBloqueados->elements, pid))
			return;
	}
	iniciar_planificacion();
	log_warning(logger, "No se encontro el proceso");
}

sProceso* buscar_proceso_en_lista(t_list* lista, int pid){
	sProceso* proceso;

	bool existe_proceso(void* elem) {
		sProceso *proceso = (sProceso*)elem;
		return proceso->pcb.pid == pid;
	}

	proceso = list_find(lista, existe_proceso);
	if(!proceso)
		return NULL;
	list_remove_element(lista, proceso);
	if(proceso->pcb.estado!=READY && proceso->pcb.estado!=READY_PLUS)
		free(proceso->multifuncion);
	else
		sem_wait(&semPCP);
	iniciar_planificacion();
	matadero(proceso, "Interrumpido por Usuario");
	return 1;
}

void detener_planificacion(){
	if(planificacion_activa){
		pthread_mutex_lock(&mNEW);
		pthread_mutex_lock(&mREADY);
		pthread_mutex_lock(&mRUNNING);
		pthread_mutex_lock(&mBLOCKED);
		for(int i=0; i<cantRecursos; i++)
			pthread_mutex_lock(&(recursos[i].mutex));
		pthread_mutex_lock(&mEXIT);
		planificacion_activa = false;
	}
}

void iniciar_planificacion(){
	if(!planificacion_activa){
		pthread_mutex_unlock(&mNEW);
		pthread_mutex_unlock(&mREADY);
		pthread_mutex_unlock(&mRUNNING);
		pthread_mutex_unlock(&mBLOCKED);
		for(int i=0; i<cantRecursos; i++)
			pthread_mutex_unlock(&(recursos[i].mutex));
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
		buffer[strcspn(buffer, "\n")]=0; //saco el \n
		interactuar_consola(buffer);
	}
	free (buffer);
	fclose(script);
}

void cambiar_multiprogramacion(int grado){
	int cambio = grado - multiprogramacion;
	if (grado<0)
		return;
	if(cambio>0)
		for(int i=0; i<cambio; i++)
			sem_post(&sMultiprogramacion);
	else
		for(int i=0; i>cambio; i--)
			sem_wait(&sMultiprogramacion);
	multiprogramacion = grado;
}
void proceso_estado(){
	bool planiEstabaAcitba = planificacion_activa;
	detener_planificacion();
	printf("Proceso en estado:\n");
	listar_procesos(cNEW->elements, NEW);
	listar_procesos(cREADY->elements, READY);
	if(pidRunning != -1)
		printf("%s: %d\n", get_estado(RUNNING), pidRunning);
	listar_procesos(lBlocked, BLOCKED);
	for(int i=0; i<cantRecursos; i++)
		listar_procesos(recursos[i].cBloqueados->elements, BLOCKED);
	listar_procesos(cEXIT->elements, FINISHED);
	if(planiEstabaAcitba)
		iniciar_planificacion();
}
void listar_procesos(t_list* lista, estados estado){
	sProceso* proceso;
	int tamLista = list_size(lista);
	printf("%s: ", get_estado(estado));
	for(int i=0; i<tamLista-1; i++){
		proceso = list_get(lista, i);
		printf("%d, ", proceso->pcb.pid);
	}
	if(tamLista){
	proceso = list_get(lista, tamLista-1);
	printf("%d\n", proceso->pcb.pid);
	}
	else
		printf("\n");
}

void PLP(){
	sProceso* proceso;
	while(1){
		sem_wait(&semPLP);
		pthread_mutex_lock(&mNEW);
		proceso = queue_pop(cNEW);
		pthread_mutex_unlock(&mNEW);
		sem_wait(&sMultiprogramacion);
		proceso->pcb.instrucciones = enviar_proceso(*proceso);
		free (proceso->multifuncion);
		if(proceso->pcb.instrucciones == NULL){
			matadero(proceso, "La memoria no pudo abrir el archivo");
			continue;
		}
		proceso->pcb.estado=READY;
		log_cambioEstado(proceso->pcb.pid, NEW, READY);
		pthread_mutex_lock(&mREADY);
		queue_push(cREADY, proceso);
		log_ingresoReady(cREADY->elements, "Normal");
		pthread_mutex_unlock(&mREADY);
		sem_post(&semPCP);
	}
}

char** enviar_proceso(sProceso proceso){	
	enviar_string(proceso.multifuncion, conexion_memoria, NUEVO_PROCESO);
	enviar_operacion(conexion_memoria, proceso.pcb.pid);
	if(recibir_operacion(conexion_memoria) != NUEVO_PROCESO){
		return NULL;
	}
	return recibir_puntero (conexion_memoria);
}

void matadero (sProceso* proceso, char* motivo){
	log_cambioEstado(proceso->pcb.pid, proceso->pcb.estado, FINISHED);
	if(proceso->pcb.estado != NEW)
		sem_post(&sMultiprogramacion);
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
	for(int i=0; i<proceso->recursos->elements_count; i++)
		liberar_recurso(list_get(proceso->recursos, i));
	list_destroy(proceso->recursos);
	log_finalizacion(proceso->pcb.pid, proceso->multifuncion);
	free(proceso);
	}
}

void planificadorCP_FIFO(){
	sProceso* proceso;
	t_paquete* paquete;
	int indiceRecurso;
	int motivo;
	int size;
	pthread_t hilo_IO; //usamos para crearle un hilo a cada instancia de IO
	while (1){
		sem_wait(&semPCP);

		//despachar_a_running(proceso);

		pthread_mutex_lock(&mREADY);
		proceso = queue_pop(cREADY); 
		pthread_mutex_unlock(&mREADY);

		log_cambioEstado(proceso->pcb.pid, READY, RUNNING);
		proceso->pcb.estado=RUNNING;
		pthread_mutex_lock(&mRUNNING);
		pidRunning = proceso->pcb.pid;
		pthread_mutex_unlock(&mRUNNING);
		enviar_pcb(proceso->pcb, conexion_cpu_dispatch, PCB);


		//me quedo esperando que vuelva y veo porque volvio
		motivo = recibir_operacion(conexion_cpu_dispatch);
		pthread_mutex_lock(&mRUNNING);
		pidRunning = -1;
		pthread_mutex_unlock(&mRUNNING);
		proceso->pcb=pcb_deserializar(conexion_cpu_dispatch);

		switch(motivo){
			case FINALIZACION:
				matadero(proceso, "Éxito");
				break;
			case IO:
			case IO_VECTOR:
				log_cambioEstado(proceso->pcb.pid, RUNNING, BLOCKED);
				proceso->pcb.estado = BLOCKED;				
				if(recibir_operacion(conexion_cpu_dispatch) != IO)
					matadero(proceso, "No coinciden los códigos de salida");

				proceso->multifuncion = recibir_buffer(&size, conexion_cpu_dispatch);

				pthread_mutex_lock(&mBLOCKED);
				list_add(lBlocked, proceso);
				
				pthread_create(&hilo_IO, NULL, atender_solicitud_IO, (void*)proceso);
				pthread_mutex_lock(&mBLOCKED);
				pthread_mutex_unlock(&mBLOCKED);
				break;
			case PEDIRRECURSO:
				if(recibir_operacion(conexion_cpu_dispatch) != PEDIRRECURSO){
					matadero(proceso, "No coinciden los códigos de salida");
					break;
				}
				proceso->multifuncion = recibir_buffer(&size, conexion_cpu_dispatch);
				pedir_recurso(proceso);
				break;
			case DARRECURSO:
				if(recibir_operacion(conexion_cpu_dispatch) != DARRECURSO){
					matadero(proceso, "No coinciden los códigos de salida");
					break;
				}
				proceso->multifuncion = recibir_buffer(&size, conexion_cpu_dispatch);
				int recurso = buscar_recurso(proceso->multifuncion);
				free(proceso->multifuncion);
				if(recurso == -1){
					matadero(proceso, "Pidio un recurso no existente");
					break;
				}
				list_remove_element(proceso->recursos, recursos[recurso].nombre);
				liberar_recurso(recursos[recurso].nombre);
				log_ingresoReady(cREADY->elements, "Normal");
				pthread_mutex_lock(&mREADY);
				queue_push(cREADY, proceso);
				pthread_mutex_unlock(&mREADY);
				sem_post(&semPCP);
				break;
			case INTERRUPCION:
				matadero(proceso, "Interrumpido por Usuario");
				break;
			case OOM:
				matadero(proceso, "Out of Memory");
				break;
			default:
				matadero(proceso, "Envio codigo de salida no valido");
				break;
		}		
	}
}

void despachar_a_running(sProceso* proceso) {
	//despacha a running al primero que este en la lista de ready
	//sProceso* proceso;

	pthread_mutex_lock(&mREADY);
	proceso = queue_pop(cREADY); 
	pthread_mutex_unlock(&mREADY);

	log_cambioEstado(proceso->pcb.pid, READY, RUNNING);
	proceso->pcb.estado=RUNNING;
	enviar_pcb(proceso->pcb, conexion_cpu_dispatch, PCB);
}

void setear_timer(sProceso* proceso) {
	usleep(proceso->pcb.quantum * 1000); // multiplicamos por mil para ir de milisecs a microsecs (lo q toma usleep)
	enviar_int (proceso->pcb.pid, conexion_cpu_interrupt, FIN_DE_QUANTUM);
}

void planificadorCP_RR(){
	sProceso* proceso;
	t_paquete* paquete;
	int indiceRecurso;
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
		pthread_mutex_lock(&mRUNNING);
		pidRunning = proceso->pcb.pid;
		pthread_mutex_unlock(&mRUNNING);
		enviar_pcb(proceso->pcb, conexion_cpu_dispatch, PCB);

		pthread_create(&hilo_timer, NULL, setear_timer, (void *) proceso);

		motivo = recibir_operacion(conexion_cpu_dispatch);
		proceso->pcb=pcb_deserializar(conexion_cpu_dispatch);
		pthread_mutex_lock(&mRUNNING);
		pidRunning = -1;
		pthread_mutex_unlock(&mRUNNING);
		switch(motivo){
			case FINALIZACION:
				pthread_cancel(hilo_timer); //cancelamos el hilo de timer pq volvimos por otro motivo
				matadero(proceso, "Éxito");
				break;
			case IO:
			case IO_VECTOR:
				pthread_cancel(hilo_timer); // cancelo el hilo de timer pq volvio antes de tiempo

				log_cambioEstado(proceso->pcb.pid, RUNNING, BLOCKED);
				proceso->pcb.estado = BLOCKED;
				
				if(recibir_operacion(conexion_cpu_dispatch) != IO)
					matadero(proceso, "No coinciden los códigos de salida");

				proceso->multifuncion = recibir_buffer(&size, conexion_cpu_dispatch);

				pthread_mutex_lock(&mBLOCKED);
				list_add(lBlocked, proceso);
				
				pthread_create(&hilo_IO, NULL, atender_solicitud_IO, (void*)proceso);
				pthread_mutex_lock(&mBLOCKED);
				pthread_mutex_unlock(&mBLOCKED);
				break;
			case PEDIRRECURSO:
				pthread_cancel(hilo_timer); // cancelo el hilo de timer pq volvio antes de tiempo
				if(recibir_operacion(conexion_cpu_dispatch) != PEDIRRECURSO){
					matadero(proceso, "No coinciden los códigos de salida");
					break;
				}
				proceso->multifuncion = recibir_buffer(&size, conexion_cpu_dispatch);
				pedir_recurso(proceso);
				break;
			case DARRECURSO:
				pthread_cancel(hilo_timer); // cancelo el hilo de timer pq volvio antes de tiempo
				if(recibir_operacion(conexion_cpu_dispatch) != DARRECURSO){
					matadero(proceso, "No coinciden los códigos de salida");
					break;
				}
				proceso->multifuncion = recibir_buffer(&size, conexion_cpu_dispatch);
				int recurso = buscar_recurso(proceso->multifuncion);
				free(proceso->multifuncion);
				if(recurso == -1){
					matadero(proceso, "Pidio un recurso no existente");
					break;
				}
				list_remove_element(proceso->recursos, recursos[recurso].nombre);
				liberar_recurso(recursos[recurso].nombre);
				log_ingresoReady(cREADY->elements, "Normal");
				pthread_mutex_lock(&mREADY);
				queue_push(cREADY, proceso);
				pthread_mutex_unlock(&mREADY);
				sem_post(&semPCP);
				break;
			case FIN_DE_QUANTUM:
				log_finDeQuantum(proceso->pcb.pid);
				log_cambioEstado(proceso->pcb.pid, RUNNING, READY);
				proceso->pcb.estado=READY;

				pthread_mutex_lock(&mREADY);
				queue_push(cREADY, proceso);
				log_ingresoReady(cREADY->elements, "Normal");
				pthread_mutex_unlock(&mREADY);

				sem_post(&semPCP); // pasa a estar esperando, aviso al dispatcher
				break;
			case INTERRUPCION:
				pthread_cancel(hilo_timer); //cancelamos el hilo de timer pq volvimos por otro motivo
				matadero(proceso, "Interrumpido por Usuario");
				break;
			case OOM:
				pthread_cancel(hilo_timer); //cancelamos el hilo de timer pq volvimos por otro motivo
				matadero(proceso, "Out of Memory");
				break;
			default:
				matadero(proceso, "Envio codigo de salida no valido");
				break;
		}		
	}
}

void planificadorCP_VRR() {
	sProceso* proceso;
	t_paquete* paquete;
	int motivo;
	int size;
	int indiceRecurso;
	pthread_t hilo_IO; //usamos para crearle un hilo a cada instancia de IO
	pthread_t hilo_timer;
	struct timespec tiempoInicio;
	struct timespec tiempoVuelta;

	while (1) {
		sem_wait(&semPCP);

		if (!queue_is_empty(cREADY_PLUS)) {
			// prioridad a los de ready plus
			pthread_mutex_lock(&mREADY_PLUS);
			proceso = queue_pop(cREADY_PLUS); 
			pthread_mutex_unlock(&mREADY_PLUS);

			log_cambioEstado(proceso->pcb.pid, READY_PLUS, RUNNING);
		} else {
			pthread_mutex_lock(&mREADY);
			proceso = queue_pop(cREADY); 
			pthread_mutex_unlock(&mREADY);

			log_cambioEstado(proceso->pcb.pid, READY, RUNNING);
		}


		proceso->pcb.estado=RUNNING;

		pthread_mutex_lock(&mRUNNING);
		pidRunning = proceso->pcb.pid;
		pthread_mutex_unlock(&mRUNNING);

		enviar_pcb(proceso->pcb, conexion_cpu_dispatch, PCB); // lo mando a correr
		clock_gettime(CLOCK_MONOTONIC_RAW, &tiempoInicio); // marco la hora q lo mande a correr

		pthread_create(&hilo_timer, NULL, setear_timer, (void *) proceso); // seteo timer (lo que le quede esta en su pcb)

		// me quedo esperando a que vuelva
		motivo = recibir_operacion(conexion_cpu_dispatch);
		clock_gettime(CLOCK_MONOTONIC_RAW, &tiempoVuelta); // marco la hora q volvio
		proceso->pcb=pcb_deserializar(conexion_cpu_dispatch);

		pthread_mutex_lock(&mRUNNING);
		pidRunning = -1;
		pthread_mutex_unlock(&mRUNNING);

		switch(motivo){
			case FINALIZACION:
				pthread_cancel(hilo_timer); //cancelamos el hilo de timer pq volvimos por otro motivo
				matadero(proceso, "Éxito");
				break;
			case IO:
			case IO_VECTOR:
				pthread_cancel(hilo_timer); // cancelo el hilo de timer pq volvio antes de tiempo
				get_quantum(proceso, tiempoInicio, tiempoVuelta); // actualizo el quantum restante

				log_cambioEstado(proceso->pcb.pid, RUNNING, BLOCKED);
				proceso->pcb.estado = BLOCKED;
				
				if(recibir_operacion(conexion_cpu_dispatch) != IO)
					matadero(proceso, "No coinciden los códigos de salida");

				proceso->multifuncion = recibir_buffer(&size, conexion_cpu_dispatch);

				pthread_mutex_lock(&mBLOCKED);
				list_add(lBlocked, proceso);
				
				pthread_create(&hilo_IO, NULL, atender_solicitud_IO, (void*)proceso);
				pthread_mutex_lock(&mBLOCKED);
				pthread_mutex_unlock(&mBLOCKED);
				break;
			case PEDIRRECURSO:
				pthread_cancel(hilo_timer); // cancelo el hilo de timer pq volvio antes de tiempo
				get_quantum(proceso, tiempoInicio, tiempoVuelta); // actualizo el quantum restante
				if(recibir_operacion(conexion_cpu_dispatch) != PEDIRRECURSO){
					matadero(proceso, "No coinciden los códigos de salida");
					break;
				}
				proceso->multifuncion = recibir_buffer(&size, conexion_cpu_dispatch);
				pedir_recurso(proceso);
				break;
			case DARRECURSO:
				pthread_cancel(hilo_timer); // cancelo el hilo de timer pq volvio antes de tiempo
				get_quantum(proceso, tiempoInicio, tiempoVuelta); // actualizo el quantum restante
				if(recibir_operacion(conexion_cpu_dispatch) != DARRECURSO){
					matadero(proceso, "No coinciden los códigos de salida");
					break;
				}
				proceso->multifuncion = recibir_buffer(&size, conexion_cpu_dispatch);
				int recurso = buscar_recurso(proceso->multifuncion);
				free(proceso->multifuncion);
				if(recurso == -1){
					matadero(proceso, "Pidio un recurso no existente");
					break;
				}
				list_remove_element(proceso->recursos, recursos[recurso].nombre);
				liberar_recurso(recursos[recurso].nombre);
				log_ingresoReady(cREADY_PLUS->elements, "Prioridad");
				pthread_mutex_lock(&mREADY_PLUS);
				queue_push(cREADY_PLUS, proceso);
				pthread_mutex_unlock(&mREADY_PLUS);
				sem_post(&semPCP);
				break;
			case FIN_DE_QUANTUM:
				log_finDeQuantum(proceso->pcb.pid);
				log_cambioEstado(proceso->pcb.pid, RUNNING, READY);
				proceso->pcb.estado=READY;
				proceso->pcb.quantum=quantum; // le "recargo" el quantum para la prox vuelta

				pthread_mutex_lock(&mREADY);
				queue_push(cREADY, proceso);
				log_ingresoReady(cREADY->elements, "Normal");
				pthread_mutex_unlock(&mREADY);

				sem_post(&semPCP); // pasa a estar esperando, aviso al dispatcher
				break;
			case INTERRUPCION:
				pthread_cancel(hilo_timer); //cancelamos el hilo de timer pq volvimos por otro motivo
				matadero(proceso, "Interrumpido por Usuario");
				break;
			case OOM:
				pthread_cancel(hilo_timer); //cancelamos el hilo de timer pq volvimos por otro motivo
				matadero(proceso, "Out of Memory");
				break;
			default:
				matadero(proceso, "Envio codigo de salida no valido");
				break;
		}		
	}
}


void get_quantum(sProceso* proceso, struct timespec tiempoInicio, struct timespec tiempoVuelta){
	int tiempoTranscurrido = (tiempoVuelta.tv_sec - tiempoInicio.tv_sec) * 1000 + (tiempoVuelta.tv_nsec - tiempoInicio.tv_nsec) / 1000000;
	if (proceso->pcb.quantum >= tiempoTranscurrido) 
		proceso->pcb.quantum -= tiempoTranscurrido;
	else
		proceso->pcb.quantum = 0; // caso borde, evitamos quantums negativos
}


//RECURSOS
int buscar_recurso(char* nombre){
	for (int i = 0; i < cantRecursos; i++) {
    	if (!strcmp(nombre, recursos[i].nombre)) {
			return i;
		}
	}
	return -1;
}

void pedir_recurso(sProceso* proceso){
	int recurso = buscar_recurso(proceso->multifuncion);
	if(recurso == -1){
		matadero(proceso, "Pidio un recurso no existente");
		return;
	}
	pthread_mutex_lock((&(recursos[recurso].mutex)));
	recursos[recurso].instancias--;
	pthread_mutex_unlock((&(recursos[recurso].mutex)));
	if (recursos[recurso].instancias < 0) {
		log_cambioEstado(proceso->pcb.pid, RUNNING, BLOCKED);
		proceso->pcb.estado = BLOCKED;
		pthread_mutex_lock(&(recursos[recurso].mutex));
		queue_push(recursos[recurso].cBloqueados, proceso);
		pthread_mutex_unlock(&(recursos[recurso].mutex));            			
		log_bloqueo(proceso->pcb.pid, recursos[recurso].nombre);
    }
	else{
		free(proceso->multifuncion);
		list_add(proceso->recursos, recursos[recurso].nombre);
		if(planiEsVrr){
			log_cambioEstado(proceso->pcb.pid, RUNNING, READY_PLUS);
			proceso->pcb.estado=READY_PLUS;
			pthread_mutex_lock(&mREADY_PLUS);
			queue_push(cREADY, proceso);
			log_ingresoReady(cREADY_PLUS->elements, "Prioridad");
			pthread_mutex_unlock(&mREADY_PLUS);
		}
		else{
			log_cambioEstado(proceso->pcb.pid, RUNNING, READY);
			proceso->pcb.estado=READY;
			pthread_mutex_lock(&mREADY);
			queue_push(cREADY, proceso);
			log_ingresoReady(cREADY->elements, "Normal");
			pthread_mutex_unlock(&mREADY);
		}		
		sem_post(&semPCP);
	}
}


void liberar_recurso(char* recurso) {
    int indiceRecurso = buscar_recurso(recurso);
        
    pthread_mutex_lock(&(recursos[indiceRecurso].mutex));
    recursos[indiceRecurso].instancias++;
    pthread_mutex_unlock(&(recursos[indiceRecurso].mutex));
        
    if(recursos[indiceRecurso].instancias <= 0 && !queue_is_empty(recursos[indiceRecurso].cBloqueados)) {
        pthread_mutex_lock(&(recursos[indiceRecurso].mutex));
        sProceso* procesoDesbloqueado = queue_pop(recursos[indiceRecurso].cBloqueados);
		free(procesoDesbloqueado->multifuncion);
        pthread_mutex_unlock(&(recursos[indiceRecurso].mutex));
		list_add(procesoDesbloqueado->recursos, recursos[indiceRecurso].nombre);

		if(!planiEsVrr){
			log_cambioEstado(procesoDesbloqueado->pcb.pid, BLOCKED, READY);
			procesoDesbloqueado->pcb.estado = READY;

			pthread_mutex_lock(&mREADY);
			queue_push(cREADY, procesoDesbloqueado);
			log_ingresoReady(cREADY->elements, "Normal");
			pthread_mutex_unlock(&mREADY);
		}
		else{
			log_cambioEstado(procesoDesbloqueado->pcb.pid, BLOCKED, READY_PLUS);
			procesoDesbloqueado->pcb.estado = READY_PLUS;
			pthread_mutex_lock(&mREADY_PLUS);
			queue_push(cREADY_PLUS, procesoDesbloqueado);
			log_ingresoReady(cREADY->elements, "Prioridad");
			pthread_mutex_unlock(&mREADY_PLUS);
    	}
		sem_post(&semPCP);
	}
}



//IO

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
				conexion->nombre = nombre;
				conexion->socket = socket;
				pthread_mutex_init(&(conexion->mutex), NULL);

				pthread_mutex_lock(&mCONEXIONES);
				list_add(lista_conexiones_IO, conexion);
				pthread_mutex_unlock(&mCONEXIONES);
				
				log_info(logger, "Se conecto la IO: %s", nombre);
			// case STDIN, etc.
		}
	}
}

void atender_solicitud_IO(sProceso* proceso){
	char** instruccionIO = string_split(proceso->multifuncion, " ");
	int tamañoVector;
	int* vectorDirecciones;

	if(!strcmp(instruccionIO[0], "IO_STDIN_READ") || !strcmp(instruccionIO[0], "IO_STDOUT_WRITE")){
	tamañoVector = atoi(instruccionIO[3]);
	vectorDirecciones = recibir_vector(conexion_cpu_dispatch, tamañoVector);
	}

	if(!strcmp(instruccionIO[0], "IO_FS_WRITE") || !strcmp(instruccionIO[0], "IO_FS_READ")){
		tamañoVector = atoi(instruccionIO[4]);
		vectorDirecciones = recibir_vector(conexion_cpu_dispatch, tamañoVector);
	}
	pthread_mutex_unlock(&mBLOCKED);

	// funcionalidad propia de gcc, inner function
	bool existe_conexion(void* elem) {
		t_conexion *conexion = (t_conexion*)elem;
		return !strcmp(conexion->nombre, instruccionIO[1]);
	}

	t_conexion* IO_seleccionada = list_find(lista_conexiones_IO, existe_conexion);

	if (IO_seleccionada == NULL) {
		pthread_mutex_lock(&mBLOCKED);
		list_remove_element(lBlocked, proceso);
		pthread_mutex_unlock(&mBLOCKED);
		free(proceso->multifuncion);
		matadero(proceso, "Se intento comunicar con una IO no conectada");
		string_array_destroy(instruccionIO);
		return;
	}
	log_bloqueo(proceso->pcb.pid, IO_seleccionada->nombre);
	pthread_mutex_lock(&(IO_seleccionada->mutex));
	if(list_remove_element(lBlocked, proceso)==false){
		pthread_mutex_unlock(&(IO_seleccionada->mutex));
		if(!strcmp(instruccionIO[0], "IO_STDIN_READ") || !strcmp(instruccionIO[0], "IO_STDOUT_WRITE") || !strcmp(instruccionIO[0], "IO_FS_WRITE") || !strcmp(instruccionIO[0], "IO_FS_READ"))
			free(vectorDirecciones);
		string_array_destroy(instruccionIO);
		return;
	}
	list_add(lBlocked, proceso);
	// le mandamos a la instancia encontrada la operacion
	enviar_string(proceso->multifuncion, IO_seleccionada->socket, OPERACION_IO);
	enviar_operacion(IO_seleccionada->socket, proceso->pcb.pid);

	if(!strcmp(instruccionIO[0], "IO_STDIN_READ") || !strcmp(instruccionIO[0], "IO_STDOUT_WRITE") || !strcmp(instruccionIO[0], "IO_FS_WRITE") || !strcmp(instruccionIO[0], "IO_FS_READ")){
		enviar_vector(vectorDirecciones, tamañoVector, IO_seleccionada->socket);
		free(vectorDirecciones);
	}
		

	// nos quedamos escuchando la respuesta
	int codOp = recibir_operacion (IO_seleccionada->socket);
	pthread_mutex_unlock(&(IO_seleccionada->mutex));

	if (codOp == IO_FAILURE) { // en caso de que la operacion no sea valida
		pthread_mutex_lock(&mBLOCKED);
		list_remove_element(lBlocked, proceso);
		pthread_mutex_unlock(&mBLOCKED);
		free(proceso->multifuncion);
		matadero(proceso, "La IO no admite la operacion solicitada");
		string_array_destroy(instruccionIO);
		return;
	}

	if (codOp == 0 || codOp == -1) { // en caso de que el socket de la interfaz se haya desconectado
		pthread_mutex_lock(&mBLOCKED);
		list_remove_element(lBlocked, proceso);
		pthread_mutex_unlock(&mBLOCKED);
		free(proceso->multifuncion);
		matadero(proceso, "Se intento comunicar con una IO no conectada");
		string_array_destroy(instruccionIO);
		free(IO_seleccionada->nombre);
		pthread_mutex_destroy(&(IO_seleccionada->mutex));
		pthread_mutex_lock(&mCONEXIONES);
		list_remove_element(lista_conexiones_IO, IO_seleccionada);
		pthread_mutex_unlock(&mCONEXIONES);

		return;
	}
	
	// LO VOLVEMOS A READY DESPUES DE SU IO

	// lo sacamos de blocked
	string_array_destroy(instruccionIO);
	pthread_mutex_lock(&mBLOCKED);
	if(list_remove_element(lBlocked, proceso) == false){
		pthread_mutex_unlock(&mBLOCKED);
		return;
	}
	pthread_mutex_unlock(&mBLOCKED);

	free(proceso->multifuncion);

	if (!planiEsVrr) {
		log_cambioEstado(proceso->pcb.pid, BLOCKED, READY);
		proceso->pcb.estado=READY;

		pthread_mutex_lock(&mREADY);
		queue_push(cREADY, proceso);
		log_ingresoReady(cREADY->elements, "Normal");
		pthread_mutex_unlock(&mREADY);
	} else {
		log_cambioEstado(proceso->pcb.pid, BLOCKED, READY_PLUS);
		proceso->pcb.estado=READY_PLUS;

		pthread_mutex_lock(&mREADY_PLUS);
		queue_push(cREADY_PLUS, proceso);
		log_ingresoReady(cREADY_PLUS->elements, "Prioridad");
		pthread_mutex_unlock(&mREADY_PLUS);
	}

	sem_post(&semPCP); //avisamos al dispatcher q hay proceso listo
}


//LOGS OBLIGATORIOS
void log_nuevoProceso (int pid){
	log_info(logger, "Se creo el proceso %d en NEW", pid);
}

void log_cambioEstado (int pid, estados eAnterior, estados eActual){
	log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pid, get_estado(eAnterior), get_estado(eActual));
}
void log_finalizacion(int pid, char* motivo){
	log_info(logger, "Finaliza el proceso %d - Motivo: %s", pid, motivo);
}

void log_finDeQuantum(int pid){
	log_info(logger, "PID: %d - Desalojado por fin de Quantum", pid);
}

void log_ingresoReady(t_list* lista, char* cola){
	sProceso* proceso;
	char* listado = string_new();
	for(int i=0; i<list_size(lista); i++){
		proceso = list_get(lista, i);
		string_append_with_format(&listado, "PID: %d ", proceso->pcb.pid);
	}
	log_info(logger, "Cola Ready %s: %s", cola, listado);
	free(listado);
}

void log_bloqueo(int pid, char* motivo){
	log_info(logger, "PID: %d - Bloqueado por: %s", pid, motivo);
}



