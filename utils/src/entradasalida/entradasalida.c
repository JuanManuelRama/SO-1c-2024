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
		switch (cod_op) {
			case SLEEP:
				int size;
				char* buffer = recibir_buffer(&size, socket_kernel);
				double tiempo = atof(buffer);
				log_info(logger, "Me mandaron a dormir");
				sleep(tiempo * unidad_trabajo);
				enviar_mensaje("Sleep terminado.", socket_kernel);
				log_info(logger, "Sleep ejecutado exitosamente");
				free(buffer);
				break;
			case -1:
				log_error(logger, "el cliente se desconecto");
				liberar_conexion(socket_kernel);
				return;
			default:
				log_warning(logger,"Operacion desconocida. No quieras meter la pata");
				break;
		}
	}

	liberar_conexion(socket_kernel);
}
