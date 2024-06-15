#include <entradasalida/entradasalida.h>

t_log* logger;
t_config* config;

int tam_pagina;

int main(int argc, char *argv[]) {
	// nos aseguramos que nos pasen 3 argumentos por linea de comando (el exe, el nombre y el path al config)
	if (argc != 3) {
		logger = log_create("logIO.log", "LOGS IO", 1, LOG_LEVEL_INFO);
		log_error(logger, "Faltan parametros para inicializar IO");
		return EXIT_FAILURE;
	}
	
	char* nombre = string_new();
	char* path_config = string_new();
	char* tipo;

	// tomamos el nombre y path config de los parametros pasados por linea de comando
	strcpy(nombre, argv[1]);
	strcpy(path_config, argv[2]);

	logger = log_create("logIO.log", "LOGS IO", 1, LOG_LEVEL_INFO);
	config = config_create(path_config);
	tipo = buscar("TIPO_INTERFAZ");

	if (!strcmp(tipo, "GENERICA")) {
		crear_interfaz_generica(nombre);
	} else if (!strcmp(tipo, "STDIN")) {
		crear_interfaz_stdin(nombre);
	} else if (!strcmp(tipo, "STDOUT")) {
		crear_interfaz_stdout(nombre);
	} else if (!strcmp(tipo, "DIALFS")) {
		;
	} else {
		log_error(logger, "TIPO DE INTERFAZ NO RECONOCIDO");
	}

    log_destroy(logger);
	config_destroy(config);
    return 0;
}