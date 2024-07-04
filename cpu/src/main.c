#include <cpu/cpu.h>


t_log* logger;
t_config* config;
t_pcb pcb;
t_queue *cIntr;
t_queue *tlb;
pthread_mutex_t mIntr;
int seVa;
int memoria;
int tam_pag;
int tam_memoria;
int* vectorDirecciones;
int tamañoVector;
char* aEnviar;
char* algoritmo_TLB;
int cant_entradas_TLB;
int entradas_actuales_tlb;
t_entradaTLB *entrada_TLB;

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
	aEnviar = malloc(100);
	seVa = false;
	cIntr = queue_create();
	tlb = queue_create();
	pthread_mutex_init(&mIntr, NULL);
	// buscamos datos en config y conectamos con memoria
	ip = buscar("IP_MEMORIA");
	puerto = buscar("PUERTO_MEMORIA");
	memoria = crear_conexion(ip, puerto, "Memoria"); 
	tam_pag = recibir_operacion(memoria);
	log_info(logger, "tam_pag = %i", tam_pag);
	tam_memoria = recibir_operacion(memoria);
	enviar_mensaje("Saludos desde la cpu", memoria);

	// Antes de conectarse con el kernel, preparo la TLB
	algoritmo_TLB = buscar("ALGORITMO_TLB");
	cant_entradas_TLB = atoi(buscar("CANTIDAD_ENTRADAS_TLB"));
	entradas_actuales_tlb = cant_entradas_TLB;

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
					case INTERRUPCION:
						if(pcb.pid == interrupcion->pid)
							seVa = INTERRUPCION;
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
		if(seVa == DARRECURSO || seVa == PEDIRRECURSO){
			enviar_string(aEnviar, socket_dispatch, seVa);
		}else if(seVa == IO){
			enviar_string(aEnviar, socket_dispatch, IO);
		}else if(seVa == IO_VECTOR){
			enviar_string(aEnviar, socket_dispatch, IO);
			enviar_vector(vectorDirecciones, tamañoVector, socket_dispatch);
			free(vectorDirecciones);
		}

		seVa=false;
	}
    return 0;

}

