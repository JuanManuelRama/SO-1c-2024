#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <utils/hello.h>
#include <pthread.h>
#include <sockets/sockets.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <unistd.h>
#include <commons/bitarray.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <dirent.h> 

//extern t_log* logger;
//extern t_config* config;

//INTERFACES
void crear_interfaz_generica(char* nombre);
void crear_interfaz_stdin (char* nombre);
void crear_interfaz_stdout (char* nombre);
void crear_interfaz_fs (char* nombre);


extern int tam_pagina;

//OPERACIONES
bool crear_fs(char* nombre);
void eliminar_fs(char* nombre);
void truncar_fs(char* nombre, int tamaño);
bool escribir_fs(char* archivo, char* cadena, int offset);
char* leer_fs(char *nombre, int offset, int cantALeer);

// UTILS
char* armarPathMetadata (char* nombre);
void iniciar_fs();
void compactar();
void liberarEntradaFat(void *elem);

//LOGS OBLIGATORIOS
void log_operacion(int, char*);
void log_creacion(int, char*);
void log_eliminacion(int, char*);
void log_truncamiento(int, char*, int);
