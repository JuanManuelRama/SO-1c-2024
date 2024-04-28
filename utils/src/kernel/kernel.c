#include "kernel.h"


void inicializar_kernel(){
	logger = log_create("logKernel.log", "LOGS Kernel", 1, LOG_LEVEL_INFO);
	config = config_create("kernel.config");
}

void finalizar_kernel(){
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion_memoria);
	liberar_conexion(conexion_cpu);
}

void enviar_proceso(char* path)
{	//CREAR PCB
	//VERIFICAR GRADO DE MULTIPROGRAMACION
	enviar_string(path, conexion_memoria, NUEVO_PROCESO);
	//RECIBIR PUNTERO A LA LISTA
	//AGREGARL A LA COLA
}

void syscall_IO_GEN_SLEEP(int socket, char* tiempo) {
	// aca se podria de ver como mandar el tiempo como int/float
	enviar_string(tiempo, socket, IO_GEN_SLEEP);

	//se asume que tras sleepear el IO devuelve un mensaje de exito
	int operacion = recibir_operacion(socket);
	if (operacion == MENSAJE)
		recibir_mensaje(socket);
}