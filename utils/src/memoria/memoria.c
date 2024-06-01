#include "memoria.h"

void recibir_proceso(int socket_cliente){
		int size;
		t_proceso* proceso = malloc(sizeof(t_proceso));
		char* path = recibir_buffer (&size, socket_cliente);
		proceso->pid = recibir_operacion(socket_cliente);
		proceso->instrucciones = cargar_proceso(path);
		free (path);
		if(proceso->instrucciones){
		proceso->paginas = malloc(sizeof(t_pag)*CANT_PAG);
		log_TdP(proceso->pid);
		}
		else
			proceso=NULL;
		enviar_puntero(proceso, socket_cliente, NUEVO_PROCESO);
}

void interactuar_Kernel(int kernel){
		while (1) {
		int cod_op = recibir_operacion(kernel);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(kernel);
			break;
		case NUEVO_PROCESO:
			recibir_proceso(kernel);
			break;
		case FINALIZACION:
			liberar_proceso(kernel);
			break;
		case -1:
			log_error(logger, "el cliente se desconecto");
			return EXIT_FAILURE;
		default:
			log_warning(logger,"Operacion no esperada por parte del Kernel");
			break;
		}
	}
}

void interactuar_cpu(int cpu){
	while (1) {
		int cod_op = recibir_operacion(cpu);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(cpu);
			break;
		case PROCESO:
			proceso = recibir_puntero(cpu);
			break;
		case FETCH:
			buscar_instruccion(cpu);
			break;
		case -1:
			log_error(logger, "el cliente se desconecto");
			return EXIT_FAILURE;
		default:
			log_warning(logger,"Operacion no esperada por parte de este cliente");
			break;
		}
	}
}

void interactuar_IO (int IO){
	while (1) {

		int cod_op = recibir_operacion(IO);
		switch (cod_op) {

		//cases de solicitudes de las interfaces 

		case -1:
			log_error(logger, "el cliente se desconecto");
			return EXIT_FAILURE;
		default:
			log_warning(logger,"Operacion no esperada por parte de este cliente");
			break;
		}
	}
}

void escuchar_nuevas_IO (int socket_server){
	int socket;
	pthread_t hilo_IO;

	while(1){
		socket = accept(socket_server, NULL, NULL);
		if (recibir_operacion(socket) == NUEVA_IO) {
			pthread_create(&hilo_IO, NULL, interactuar_IO, (void*)socket);
			log_info(logger, "Se conecto nueva IO");
		}
	}
}

void buscar_instruccion(int socket_cliente){
	int instruccion = recibir_int(socket_cliente);
	sleep(RETARDO);
	enviar_string(proceso->instrucciones[instruccion], socket_cliente, FETCH);
}

void inicializar_tabla_de_memoria(){
    for(short i = 0; i < CANT_PAG; i++){
    	tablaMemoria[i].ocupado=false;
		tablaMemoria[i].espacio_libre=TAM_PAG;
	}
}

void inicializar_memoria(){
	logger = log_create ("logM.log", "LOGS MEMORIA",1, LOG_LEVEL_INFO); 
	config = config_create("memoria.config"); 

	TAM_PAG = config_get_int_value(config,"TAM_PAGINA");
	TAM_MEMORIA = config_get_int_value(config,"TAM_MEMORIA");
	RETARDO = config_get_int_value(config,"RETARDO_RESPUESTA")/1000;

	//PATH_INSTRUCCIONES = buscar("PATH_INSTRUCCIONES");

	if (TAM_MEMORIA % TAM_PAG) {
		log_error(logger, "Tamaño de memoria no multiplo de cantidad de paginas");
		exit(-1);
	}

	CANT_PAG = TAM_MEMORIA/TAM_PAG;
	memoria_contigua = malloc(TAM_MEMORIA);
	tablaMemoria = malloc(CANT_PAG*sizeof(t_tablaMemoria));
	inicializar_tabla_de_memoria();
}

char** cargar_proceso(char* nombreArchivo){
	// aca se deberá agregar el uso de PATH_INSTRUCCIONES
	FILE* archivoProceso = fopen(nombreArchivo, "r");
	if(archivoProceso == NULL){
		log_error(logger, "No se pudo abrir el archivo: %s", nombreArchivo);
		return NULL;
	}
	t_queue* qProceso = queue_create();
	char* instruccion;
	char* buffer;
	buffer=malloc(MAX_LINEA);

	// recorre archivo cargando linea por linea en la lista
	while (!feof(archivoProceso)){
		fgets(buffer, MAX_LINEA, archivoProceso);
		instruccion = malloc(strlen (buffer) +1);
		strcpy(instruccion, buffer);
		instruccion[strcspn(instruccion, "\n")]=0;
		queue_push(qProceso, instruccion);
	}
	free (buffer);
	fclose(archivoProceso);
	return queue_a_array(qProceso);;
}


char** queue_a_array(t_queue* cola){
	short elem = queue_size(cola);
	char** array = malloc(sizeof (char*)*(elem+1));
	short i=0;
	while(cola->elements->elements_count){
		array[i] = queue_pop(cola);
		i++;
	}
	array[i]=NULL;
	queue_destroy(cola);
	return array;
}



void finalizar_memoria(){
	free(memoria_contigua);
	free(tablaMemoria);
	log_destroy(logger);
	config_destroy(config);
}

void liberar_proceso(int socket_cliente){
	t_proceso* proceso=recibir_puntero(socket_cliente);
	if(proceso){
	string_array_destroy(proceso->instrucciones);
	free(proceso->paginas);
	log_TdP(proceso->pid);
	}

	free(proceso);
}

// LOGS OBLIGATORIOS
log_TdP(int pid){
	log_info(logger, "PID: %d - Tamaño: %d", pid, CANT_PAG);
}