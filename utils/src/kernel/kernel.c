#include "kernel.h"


void inicializar(){
	logger = log_create("logKernel.log", "LOGS Kernel", 1, LOG_LEVEL_INFO);
	config = config_create("kernel.config");
}

void finalizar(){
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion_memoria);
	liberar_conexion(conexion_cpu);
}

void enviar_proceso(char* path)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = NUEVO_PROCESO;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(path) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, path, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(conexion_memoria, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}