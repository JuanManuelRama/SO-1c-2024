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
extern t_queue* cEXIT;
extern t_list* lBlocked;
extern t_list* lista_conexiones_IO;
extern pthread_mutex_t mNEW;
extern pthread_mutex_t mREADY;
extern pthread_mutex_t mBLOCKED;
extern pthread_mutex_t mEXIT;
extern sem_t semPCP;
extern sem_t semPLP;
extern sem_t semEXIT;
extern sem_t sMultiprogramacion;
extern int conexion_memoria;
extern int conexion_cpu;
extern int idPCB;
extern int multiprogramacion;
extern int quantum;
extern int kernel_servidor;
extern bool planificacion_activa;

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
void crear_proceso(char* path);

void iniciar_planificacion();

void detener_planificacion();

void ejecutar_script(char* path);

//PLANIFICADORES
/**
*@fn 		planificadorCP
*@brief		Envía y recibe procesos de la CPU
*/
void planificadorCP();

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

/**
*@fn 		log_cambioEstado
*@brief		Loguea la finalizacion de un proceso
*@param     pid Id del proceso finalizado
*@param     motivo Motivo de la finalización
*/
void log_finalizacion(int, char*);

//falta agregar documentacion
void atender_solicitud_IO(sProceso*);
void escuchar_conexiones_IO(int socket);