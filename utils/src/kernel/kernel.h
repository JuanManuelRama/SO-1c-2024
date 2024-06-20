#include <stdlib.h>
#include <stdio.h>
#include <sockets/sockets.h>
#include <pthread.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <commons/collections/queue.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>

extern t_log* logger;
extern t_config* config;
extern t_queue* cNEW;
extern t_queue* cREADY;
extern t_queue* cREADY_PLUS; // usado para VRR
extern t_queue* cEXIT;
extern t_list* lBlocked;
extern t_list* lista_conexiones_IO;
extern t_recurso* recursos;
extern pthread_mutex_t mNEW;
extern pthread_mutex_t mREADY;
extern pthread_mutex_t mREADY_PLUS; // usado para VRR
extern pthread_mutex_t mRUNNING;
extern pthread_mutex_t mBLOCKED;
extern pthread_mutex_t mEXIT;
extern pthread_mutex_t mCONEXIONES;
extern sem_t semPCP;
extern sem_t semPLP;
extern sem_t semEXIT;
extern sem_t sMultiprogramacion;
extern int conexion_memoria;
extern int conexion_cpu_dispatch;
extern int conexion_cpu_interrupt;
extern int idPCB;
extern int multiprogramacion;
extern int quantum;
extern int kernel_servidor;
extern int pidRunning;
extern int tam_pagina;
extern int cantRecursos;
extern int instanciasUtilizadas;
extern bool planificacion_activa;
extern bool planiEsVrr; // vrr (se usa para ver a donde mandas a un proceso q vuelve de IO, ready/ready+)
extern char* recursoPedido;

typedef struct{
    t_pcb pcb;
    char* multifuncion;
}sProceso;

typedef struct{
    char *nombre;
    int socket;
}t_conexion;


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
*@param 	proceso Proceso a enviar
*@return	puntero a la lista de instrucciones (aun no está programado)	
*/


char** enviar_proceso(sProceso);



//FUNCIONES DE CONSOLA

/**
 * @fn 		interactuar_consola
 * @brief	Interactua con la consola
 * @param 	buffer Comando a evaluar 
 */
void interactuar_consola(char*);

/**
 * @fn 		get_terminal
 * @brief	Devuelve el valor de la terminal
 * @param 	comando Comando a evaluar 
 * @return 	Valor de la terminal
 */
int get_terminal(char*);
/**
*@fn 		crear_proceso
*@brief		Crea un proceso
*/
void crear_proceso(char*);

void iniciar_planificacion();

void detener_planificacion();

void ejecutar_script(char*);

void cambiar_multiprogramacion(int);

void proceso_estado();

//PLANIFICADORES
/**
*@fn 		planificadorCP_FIFO
*@brief		Envía y recibe procesos de la CPU segun fifo
*/
void planificadorCP_FIFO();

/**
*@fn 		planificadorCP_RR
*@brief		Envía y recibe procesos de la CPU segun RR
*/
void planificadorCP_RR();

void planificadorCP_VRR();

/**
*@fn 		PLP
*@brief		Pasa a procesos de NEW a READY
*/
void PLP();

/**
*@fn 		matadero
*@brief		Prepara los procesos para ser eliminados
*@param     proceso Proceso a elminiar
*@param     motivo Motivo para eliminarlo
*/
void matadero(sProceso*, char*);

/**
*@fn 		carnicero
*@brief		Finaliza los procesos
*/
void carnicero();



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

void log_bloqueo(int, char*);

/**
*@fn 		log_cambioEstado
*@brief		Loguea la finalizacion de un proceso
*@param     pid Id del proceso finalizado
*@param     motivo Motivo de la finalización
*/
void log_finalizacion(int, char*);

void log_ingresoReady(t_list*, char*);

void log_finDeQuantum(int);

void listar_procesos(t_list*, int);


//falta agregar documentacion
void atender_solicitud_IO(sProceso*);
void escuchar_conexiones_IO(int socket);

/**
*@fn 		despachar_a_running
*@brief		toma el primer elemento de cola READY, loggea, cambia su estado y lo manda a cpu
*/
void despachar_a_running();

/**
*@fn 		setear_timer
*@brief		recibe un quantum, espera ese tiempo y al terminar manda interrupcion a cpu
*/
void setear_timer(sProceso*);

int buscar_recurso(char*);

void liberar_recursos(int);