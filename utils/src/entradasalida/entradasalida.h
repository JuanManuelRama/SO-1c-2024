#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <pthread.h>
#include <sockets/sockets.h>
#include <commons/string.h>

extern t_log* logger;
extern t_config* config;

void crear_interfaz_generica(char* nombre);