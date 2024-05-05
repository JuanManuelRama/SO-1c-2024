#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <sockets/sockets.h>
#include <pthread.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <commons/collections/queue.h>
#include <semaphore.h>

extern t_log* logger;
extern t_config* config;
extern t_queue* cNEW;
extern t_queue* cREADY;
extern pthread_mutex_t mNEW;
extern pthread_mutex_t mREADY;
extern sem_t semPCP;
extern sem_t semPLP;
extern sem_t sMultiprogramacion;
extern int conexion_memoria;
extern int conexion_cpu;
extern int idPCB;
extern int multiprogramacion;

typedef struct{
    t_pcb* pcb;
    char* path;
}sPLP;

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
*@fn 		get_estado
*@brief		Transofrma un enum de estado en string
*@param 	int Enum del estado
*@return	String de estado
*/

char* get_estado(int);

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
*/

void syscall_IO_GEN_SLEEP(int, char*);

/**
*@fn 		planificadorCP
*@brief		Envía y recibe procesos de la CPU
*/
void planificadorCP();

/**
*@fn 		crear_proceso
*@brief		Crea un proceso
*/
void crear_proceso(char* path);


//LOGS OBLIGATORIOS
/**
*@fn 		log_nuevoProceso
*@brief		Loguea la id del proceso creado
*@param     pid Id del proceso creado
*/
void log_nuevoProceso (int);

/**
*@fn 		log_cambioEstado
*@brief		Loguea la un cambio de estado de un proceso
*@param     pid Id del proceso creado
*@param     eAnterior Estado Anterior
*@param     eActual Estado actual
*/
void log_cambioEstado(int, int, int);

/**
*@fn 		crear_pcb
*@brief		Crea una PCB en base a los parametros que le pasamos
*@param     pid que es el process id del proceso
*@param     pc program counter
*@param     quantum del proceso 
*@param     registros asociados al proceso
*@param     estado del proceso
*@param     instrucciones del proceso
*@return	t_pcb
*/
t_pcb crear_pcb (int, int, int, t_registros, int, char**);

void PLP();
