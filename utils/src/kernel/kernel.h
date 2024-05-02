#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/structs.h>
#include <sockets/sockets.h>
#include <pthread.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <commons/collections/queue.h>
#include <semaphore.h>

extern t_log* logger;
extern t_config* config;
extern t_queue* cProcesos;
extern pthread_mutex_t scProceso;
extern sem_t sMultiprogramacion;
extern int conexion_memoria;
extern int conexion_cpu;
extern int idPCB;
extern int multiprogramacion;


/**
*@fn 		inicializar
*@brief		Inicializa logger, config, (posiblemente agreguemos más)	
*/
void inicializar_kernel();

/**
*@fn 		finalizar
*@brief		Finaliza logger, config, y conexiones
*/
void finalizar_kernel();

/**
*@fn 		enviar_proceso
*@brief		Envia la dirreccion del de proceso a la memoria
*@param 	path path del proceso relativo a la memoria
*@return	puntero a la lista de instrucciones (aun no está programado)	
*/
char** enviar_proceso(char*);


/**
*@fn 		syscall_IO_GEN
*@brief		realiza llamado a IO generica, la hace dormir y espera un mensaje de vuelta
*@param 	socket de IO
*@param     tiempo que queremos que duerma
*@return	nada
*/

void syscall_IO_GEN_SLEEP(int, char*);

/**
*@fn 		planificadorCP
*@brief		Envía y recibe procesos de la CPU
*@return	nada
*/
void planificadorCP();

/**
*@fn 		crear_proceso
*@brief		Crea un proceso
*@return	nada
*/
void crear_proceso(char* path);