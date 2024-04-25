#include "main.h"

int TAM_MEMORIA;
short CANT_PAG;
short TAM_PAG;
char* PATH_INSTRUCCIONES;

void* memoria_contigua;

t_log* logger;
t_config* config;

t_list* lProcesos;
t_tablaMemoria (*tablaMemoria);

int main (){
	char* puerto;
	int socket_servidor;
	int socket_cpu;
	int socket_kernel;
	int socket_IO;
	pthread_t hilo_cpu, hilo_kernel, hilo_IO;

	inicializar();

	// buscamos dato en config e inicializamos servidor para cpu y kernel como clientes
	puerto = buscar("PUERTO_ESCUCHA");
	socket_servidor = iniciar_servidor(puerto, "Memoria");

	socket_cpu = esperar_cliente("CPU", socket_servidor);
	pthread_create(&hilo_cpu, NULL, interactuar, (void*)socket_cpu);

	socket_kernel = esperar_cliente("Kernel", socket_servidor);
	pthread_create(&hilo_kernel, NULL, interactuar, (void*)socket_kernel);

	socket_IO = esperar_cliente("IO", socket_servidor);
	pthread_create(&hilo_IO, NULL, interactuar, (void*)socket_IO);


	cargar_proceso("instru.txt");
	//Enviar la lProcesos al Kernel para que la añada al PCB
	t_instruccion *t_instruccion = list_get(lProcesos, 0);
	log_info(logger, "%s", t_instruccion->instruccion);



	pthread_join(hilo_cpu, NULL);
	pthread_join(hilo_kernel, NULL);
	pthread_join(hilo_IO, NULL);

	finalizar();
	return 0;
}

void inicializar_tabla_de_memoria(){
    for(short i = 0; i < CANT_PAG; i++){
    	tablaMemoria[i].ocupado=false;
		tablaMemoria[i].espacio_libre=TAM_PAG;
	}
}

void inicializar(){
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

void cargar_proceso(char* nombreArchivo){
	// aca se podria agregar el uso de PATH_INSTRUCCIONES
	FILE* archivoProceso = fopen(nombreArchivo, "r");

	lProcesos = list_create();
	t_instruccion *tInstruccion;
	char* buffer;
	buffer=malloc(MAX_LINEA);

	// recorre archivo cargando linea por linea en la lista
	while (!feof(archivoProceso)){
		tInstruccion=malloc(sizeof(t_instruccion));
		fgets(buffer, MAX_LINEA, archivoProceso);
		tInstruccion->longitud=strlen(buffer)-1;
		strncpy(tInstruccion->instruccion, buffer, tInstruccion->longitud);
		list_add(lProcesos, tInstruccion);
	}

	free (buffer);
	fclose(archivoProceso);
}

void finalizar(){
	free(memoria_contigua);
	free(tablaMemoria);
	list_destroy(lProcesos);
	log_destroy(logger);
	config_destroy(config);
}
