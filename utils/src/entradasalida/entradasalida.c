#include "entradasalida.h"

void destruir_args_IO(args_instancia_IO *args) {
    free(args->nombre);
    free(args->path_config);
    free(args);
}

void* crear_args_IO(char* nombre, char* path_config) {
	args_instancia_IO *args = malloc(sizeof(args_instancia_IO));
	args->nombre = string_new();
	args->path_config = string_new();
	strcpy(args->nombre, nombre);
	strcpy(args->path_config, path_config);

	return args;
}

void* crear_interfaz_generica(void* args) {
	args_instancia_IO *nombre_y_path = (args_instancia_IO*)args;

    int socket_kernel;
    char* ip;
	char* puerto;

	t_config* config = config_create(nombre_y_path->path_config);

    // buscamos datos en config y conectamos con Kernel
	ip = config_get_string_value(config, "IP_KERNEL");
	puerto = config_get_string_value(config, "PUERTO_KERNEL");

	socket_kernel = crear_conexion(ip, puerto, "Kernel");
	enviar_string(nombre_y_path->nombre, socket_kernel, NUEVA_IO);

	// una vez conectado y avisado que me conecte, me quedo esperando solicitudes
	while(1) {
		int cod_op = recibir_operacion(socket_kernel);
		switch (cod_op) {
			case SLEEP:
				int size;
				char* buffer = recibir_buffer(&size, socket_kernel);
				double tiempo = atof(buffer);
				log_info(logger, "Me mandaron a dormir");
				sleep(tiempo);
				enviar_mensaje("Sleep terminado.", socket_kernel);
				log_info(logger, "Sleep ejecutado exitosamente");
				free(buffer);
				break;
			case -1:
				log_error(logger, "el cliente se desconecto");
				config_destroy(config);
				liberar_conexion(socket_kernel);
				destruir_args_IO(args);
				return EXIT_FAILURE;
			default:
				log_warning(logger,"Operacion desconocida. No quieras meter la pata");
				break;
		}
	}
	config_destroy(config);
	liberar_conexion(socket_kernel);
	destruir_args_IO(args);
}