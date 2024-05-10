#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <pthread.h>
#include <sockets/sockets.h>
#include <commons/string.h>

extern t_log* logger;
extern t_config* config;

typedef struct {
	char* nombre;
	char* path_config;
}args_instancia_IO;

void destruir_args_IO(args_instancia_IO *args);

void* crear_args_IO(char* nombre, char* path_config);

void* crear_interfaz_generica(void* args);