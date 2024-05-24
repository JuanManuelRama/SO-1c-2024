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
		if (cod_op == OPERACION_IO) {
			int size;
			char* buffer = recibir_buffer(&size, socket_kernel);
			char** instruccion = string_n_split(buffer, 3, " ");

			log_info(logger, "Operacion: %s", instruccion[0]);

			if (!strcmp(instruccion[0], "IO_GEN_SLEEP")){
				
				int cant_unidades_trabajo = atoi(instruccion[2]);
				sleep(cant_unidades_trabajo*unidad_trabajo/1000);

				log_info(logger, "Resultado de %s: io_success", nombre);
				enviar_operacion(socket_kernel, IO_SUCCESS);

			} else {
				log_info(logger, "Resultado de %s: io_failure", nombre);
				enviar_operacion(socket_kernel, IO_FAILURE);
			}

			free(buffer);
			string_array_destroy(instruccion);
		} else {
			log_error(logger, "Soy una IO, no puedo hacer otras cosas!!");
		}
	}

	liberar_conexion(socket_kernel);
}
