#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <pthread.h>
#include <sockets/sockets.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <unistd.h>

//extern t_log* logger;
//extern t_config* config;

void crear_interfaz_generica(char* nombre);
void crear_interfaz_stdin (char* nombre);
void crear_interfaz_stdout (char* nombre);

extern int tam_pagina;