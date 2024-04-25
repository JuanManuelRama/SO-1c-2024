#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sockets/sockets.h>
#include <commons/collections/list.h>
#include <pthread.h>

#define MAX_LINEA 15

typedef struct {
	bool ocupado;
	int espacio_libre;
} t_tablaMemoria;

typedef struct {
	short longitud;
	char instruccion [MAX_LINEA];
} t_instruccion;

void inicializar_tabla_de_memoria();
void inicializar();
void cargar_proceso(char *);
void finalizar();