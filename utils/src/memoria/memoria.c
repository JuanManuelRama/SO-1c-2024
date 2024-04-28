#include "memoria.h"

void recibir_proceso(int socket_cliente){
		int size;
		char* proceso = recibir_buffer (&size, socket_cliente);
		t_list* lProceso = cargar_proceso(proceso);
		t_instruccion *t_instruccion = list_get(lProceso, 4);
		log_info(logger, "%s", t_instruccion->instruccion); //Para procesos netamente de testeos, en realidad habrá que pasarle el puntero a lista el kernel
}

void interactuar_Kernel(int socket_cliente){
		while (1) {
		int cod_op = recibir_operacion(socket_cliente);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(socket_cliente);
			break;
		case NUEVO_PROCESO:
			recibir_proceso(socket_cliente);
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

t_list* cargar_proceso(char* nombreArchivo){
	// aca se podria agregar el uso de PATH_INSTRUCCIONES
	FILE* archivoProceso = fopen(nombreArchivo, "r");

	t_list* lProceso = list_create();
	t_instruccion *tInstruccion;
	char* buffer;
	buffer=malloc(MAX_LINEA);

	// recorre archivo cargando linea por linea en la lista
	while (!feof(archivoProceso)){
		tInstruccion=malloc(sizeof(t_instruccion));
		fgets(buffer, MAX_LINEA, archivoProceso);
		tInstruccion->longitud=strlen(buffer)-1;
		strncpy(tInstruccion->instruccion, buffer, tInstruccion->longitud);
		list_add(lProceso, tInstruccion);
	}

	free (buffer);
	fclose(archivoProceso);
	return lProceso;
}

void finalizar_memoria(){
	free(memoria_contigua);
	free(tablaMemoria);
	log_destroy(logger);
	config_destroy(config);
}
