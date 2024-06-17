#include <cpu/cpu.h>


t_log* logger;
t_config* config;
t_pcb pcb;
t_queue *cIntr;
pthread_mutex_t mIntr;
int seVa;
int memoria;
int tam_pag;
int tam_memoria;
int* vectorDirecciones;
int tamañoVector;
char* aEnviar;



int main() {
	int socket_servidor;
	int socket_dispatch;
	int socket_interrupcion;
	char* ip;
	char* puerto;
	char* buffer;
	pthread_t hilo_kernel;
	sInstruccion instruccion;
	sInterrupcion* interrupcion;


	logger = log_create("logCpu.log", "LOGS CPU", 1, LOG_LEVEL_INFO);
	config = config_create("cpu.config");
	aEnviar = string_new();
	seVa = false;
	cIntr = queue_create();
	pthread_mutex_init(&mIntr, NULL);
	// buscamos datos en config y conectamos con memoria
	ip = buscar("IP_MEMORIA");
	puerto = buscar("PUERTO_MEMORIA");
	memoria = crear_conexion(ip, puerto, "Memoria"); 
	tam_pag = recibir_operacion(memoria);
	log_info(logger, "tam_pag = %i", tam_pag);
	tam_memoria = recibir_operacion(memoria);
	enviar_mensaje("Saludos desde la cpu", memoria);

	//tambien sera servidor, con el kernel como cliente
	puerto = buscar("PUERTO_ESCUCHA_DISPATCH");
	socket_servidor = iniciar_servidor(puerto, "CPU");
	socket_dispatch = esperar_cliente("Kernel", socket_servidor);
	puerto = buscar("PUERTO_ESCUCHA_INTERRUPT");
	socket_servidor = iniciar_servidor(puerto, "CPU");
	socket_interrupcion = esperar_cliente("Interrupciones", socket_servidor);
	pthread_create(&hilo_kernel, NULL, interrupciones, (void*)socket_interrupcion);
	

	while(1){
		if(recibir_operacion(socket_dispatch)!=PCB)
			finalizar_cpu();
		pcb=pcb_deserializar(socket_dispatch);
		enviar_puntero(pcb.instrucciones, memoria, PROCESO);
		while(!seVa){
    		buffer = fetch(memoria);
			instruccion = decode(buffer);
			execute(instruccion);
			pcb.registros.PC++;
			free (buffer);
			string_array_destroy(instruccion.componentes);
			pthread_mutex_lock(&mIntr);
			while(!queue_is_empty(cIntr)){
				interrupcion = queue_pop(cIntr);
				pthread_mutex_unlock(&mIntr);
				switch(interrupcion->motivo){
					case FINALIZACION:
						if(pcb.pid == interrupcion->pid)
							seVa = FINALIZACION;
						break;
					case FIN_DE_QUANTUM:
						if(!seVa && pcb.pid == interrupcion->pid)
							seVa = FIN_DE_QUANTUM;
						break;
					default:
						log_error(logger, "Interrupcion desconocida");
						break;
				}
				free(interrupcion);
				pthread_mutex_lock(&mIntr);
			}
			pthread_mutex_unlock(&mIntr);
		}
		enviar_pcb(pcb, socket_dispatch, seVa);
		if(seVa == IO_GEN || seVa == DARRECURSO || seVa == PEDIRRECURSO){
			enviar_string(aEnviar, socket_dispatch, IO);
		}else if(seVa == IO_STD){
			enviar_string(aEnviar, socket_dispatch, IO);
			enviar_vector(vectorDirecciones, tamañoVector, socket_dispatch);
		}

		seVa=false;
	}
    return 0;

}

