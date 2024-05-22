#include "entradasalida.h"

void crear_interfaz_generica(char* nombre) {
    int socket_kernel;
    char* ip;
	char* puerto;
	int unidad_trabajo;

    // buscamos datos en config y conectamos con Kernel
	ip = config_get_string_value(config, "IP_KERNEL");
	puerto = config_get_string_value(config, "PUERTO_KERNEL");
	unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");

	socket_kernel = crear_conexion(ip, puerto, "Kernel");
	enviar_string(nombre, socket_kernel, NUEVA_IO);

	// una vez conectado y avisado que me conecte, me quedo esperando solicitudes
	while(1) {
		int cod_op = recibir_operacion(socket_kernel);
		if (cod_op == IO) {
			int size;
			char* buffer = recibir_buffer(&size, socket_kernel);
			// aca habria que splitear lo que recibimos y analizar si puedo o no hacerla (siendo generica, solo puedo hacer sleep)

			free(buffer);
		} else {
			log_error(logger, "Soy una IO, no puedo hacer otras cosas!!")
		}
	}

	liberar_conexion(socket_kernel);
}
