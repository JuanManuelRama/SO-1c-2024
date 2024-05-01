#include "kernel.h"


void inicializar_kernel(){
	logger = log_create("logKernel.log", "LOGS Kernel", 1, LOG_LEVEL_INFO);
	config = config_create("kernel.config");
	cProcesos = queue_create();
	idPCB = 1;
}

void finalizar_kernel(){
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion_memoria);
	liberar_conexion(conexion_cpu);
}

void crear_proceso (char* path){
	t_pcb* proceso = malloc(sizeof (t_pcb));
	proceso->estado=NEW;
	proceso->pc=0;
	proceso->pid=idPCB;
	idPCB++;
	//proceso->quantum=quantum AÚN NO ESTÁ DEFINIDO
	//VERIFICAR GRADO DE MULTIPROGRAMACION
	proceso->instrucciones = enviar_proceso(path);
}

char** enviar_proceso(char* path){	
	enviar_string(path, conexion_memoria, NUEVO_PROCESO);
	int size;
	return recibir_buffer (&size, conexion_memoria);
}

void syscall_IO_GEN_SLEEP(int socket, char* tiempo) {
	// aca se podria de ver como mandar el tiempo como int/float
	enviar_string(tiempo, socket, IO_GEN_SLEEP);

	//se asume que tras sleepear el IO devuelve un mensaje de exito
	int operacion = recibir_operacion(socket);
	if (operacion == MENSAJE)
		recibir_mensaje(socket);
}

void planificadorCP(int cpu){
	while (1){

	}
}