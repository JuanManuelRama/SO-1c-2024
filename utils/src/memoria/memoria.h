#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sockets/sockets.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <pthread.h>

#define MAX_LINEA 50



typedef struct {
	bool ocupado;
	int espacio_libre;
} t_tablaMemoria;

typedef struct {
	short longitud;
	char instruccion [MAX_LINEA];
} t_instruccion;

extern int TAM_MEMORIA;
extern short CANT_PAG;
extern short TAM_PAG;
extern char* PATH_INSTRUCCIONES;
extern void* memoria_contigua;
extern t_log* logger;
extern t_config* config;
extern t_tablaMemoria (*tablaMemoria);





void inicializar_tabla_de_memoria();
void inicializar_memoria();


void interactuar_Kernel (int socket_cliente);


/**
*@fn 		cargar_proceso
*@brief		Vuelca un proceso en un array
*@param 	nombreArchivo Direccion al archivo
*@return	Puntero al array de instrucciones 
*/
char** cargar_proceso(char *);


/**
*@fn 		recibir_proceso
*@brief		Carga un proceso
*@param 	socket_cliente
*/
void recibir_proceso(int);

/**
*@fn 		queue_a_array
*@brief		Vuelca la cola en un array
*@param 	cola
*@returns 	Puntero al array 
*/
char** queue_a_array(t_queue*);

void finalizar_memoria();