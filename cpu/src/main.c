#include <cpu/cpu.h>


t_log* logger;
t_config* config;
t_pcb pcb;
pthread_mutex_t mCdI;
int seVa;
int memoria;
char* aEnviar;



int main() {
	int socket_servidor;
	int socket_dispatch;
	char* ip;
	char* puerto;
	char* buffer;
	pthread_t hilo_kernel;
	sInstruccion instruccion;


	logger = log_create("logCpu.log", "LOGS CPU", 1, LOG_LEVEL_INFO);
	config = config_create("cpu.config");
	aEnviar = string_new();
	seVa = false;
	pthread_mutex_init(&mCdI, NULL);
	// buscamos datos en config y conectamos con memoria
	ip = buscar("IP_MEMORIA");
	puerto = buscar("PUERTO_MEMORIA");
	memoria = crear_conexion(ip, puerto, "Memoria"); 
	enviar_mensaje("Saludos desde la cpu", memoria);

	//tambien sera servidor, con el kernel como cliente
	puerto = buscar("PUERTO_ESCUCHA_DISPATCH");
	socket_servidor = iniciar_servidor(puerto, "CPU");
	socket_dispatch = esperar_cliente("Kernel", socket_servidor);
	

	while(1){
		if(recibir_operacion(socket_dispatch)!=PCB)
			finalizar_cpu();
		pcb=pcb_deserializar(socket_dispatch);
		enviar_puntero(pcb.instrucciones, memoria, PROCESO);
		while(!seVa){
			pthread_mutex_lock(&mCdI);
    		buffer = fetch(memoria);
			instruccion = decode(buffer);
			execute(instruccion);
			pcb.registros.PC++;
			free (buffer);
			string_array_destroy(instruccion.componentes);
			pthread_mutex_unlock(&mCdI);
		}
		enviar_pcb(pcb, socket_dispatch, seVa);
		if(seVa == IO)
			enviar_string(aEnviar, socket_dispatch, seVa);
		seVa=false;
	}
    return 0;

}

