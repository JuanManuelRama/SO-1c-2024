#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sockets/sockets.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <pthread.h>
#include <commons/string.h>
#include <commons/bitarray.h>

#define MAX_LINEA 50



typedef struct {
	bool ocupado;
	int espacio_libre;
} t_tablaMemoria;

typedef struct {
	short longitud;
	char instruccion [MAX_LINEA];
} t_instruccion;

typedef struct {
	int marco;
	bool estado;
}t_pag;

typedef struct {
	int pid;
	char** instrucciones;
	t_pag* paginas;
}t_proceso;

extern int TAM_MEMORIA;
extern int RETARDO;
extern short CANT_PAG;
extern short TAM_PAG;
extern char* PATH_INSTRUCCIONES;
extern t_proceso* proceso;
extern void* memoria_contigua;
extern t_log* logger;
extern t_config* config;
extern t_tablaMemoria (*tablaMemoria);
extern t_bitarray* bitmap;





void inicializar_memoria();

void interactuar_cpu (int socket_cliente);
void interactuar_Kernel (int socket_cliente);
void interactuar_IO (int socket_IO);

void escuchar_nuevas_IO (int socket_server);


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
 * @fn 		buscar_instruccion
 * @brief	Busca una instruccion
 * @param 	socket_cliente
 */
void buscar_instruccion(int);
/**
*@fn 		queue_a_array
*@brief		Vuelca la cola en un array
*@param 	cola
*@returns 	Puntero al array 
*/
char** queue_a_array(t_queue*);


t_pag* nuevaTablaPaginas (int);


/**
 * @fn 		aniadir_paginas
 * @brief	Añade paginas
 * @param 	cpu Socket cpu
*/
void aniadir_paginas (int);

/**
 * @fn 		buscar_marco
 * @brief	Busca un marco vacío
 * @returns Indice del marco o -1 en Out of Memory
 */
int buscar_marco();

/**
 * @fn 		sacar_paginas
 * @brief	Saca paginas
 * @param 	paginas Cantidad de paginas a sacar
 
*/
void sacar_paginas(int);

/**
 * @fn 		traducir_pagina
 * @brief	Traduce una pagina
 * @param 	cpu Socket de la CPU
 */

/**
 * @fn 		tamanio_proceso
 * @brief	Devuelve el tamaño del proceso
 * @param 	socket_cliente
*/
void tamanio_proceso(int);

void traducir_pagina(int);

void liberar_proceso(int);

void finalizar_memoria();


//LOGS OBLIGATORIOS
void log_TdP(int);
void log_pagina(int, int, int);