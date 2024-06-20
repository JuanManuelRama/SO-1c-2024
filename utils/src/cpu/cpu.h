#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <sockets/sockets.h>
#include <pthread.h>
#include <readline/readline.h>
#include <commons/string.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <math.h>

typedef enum{
    SET,
    MOV_IN,
    MOV_OUT,
    SUM,
    SUB,
    JNZ,
    RESIZE,
    COPY_STRING,
    WAIT,
    SIGNAL,
    IO_GEN_SLEEP,
    IO_STDIN_READ,
    IO_STDOUT_WRITE,
    IO_FS_CREATE,
    IO_FS_DELETE,
    IO_FS_TRUNCATE,
    IO_FS_WRITE,
    IO_FS_READ,
    EXIT
}instrucciones;

typedef struct{
    int cod_instruccion;
    char** componentes;
}sInstruccion;

typedef struct{
  int pid;
  int motivo;
}sInterrupcion;

extern pthread_mutex_t mIntr;
extern t_pcb pcb;
extern int seVa;
extern int memoria;
extern int tam_pag;
extern int tam_memoria;
extern char* aEnviar;
extern int* vectorDirecciones;
extern int tamañoVector;
extern t_queue *cIntr;

void finalizar_cpu();
void interrupciones(int);

/**
 * @fn set_registro
 * @brief modifica el valor de un registro particular
 * @param pcb estructura que contiene el registro a modificar
 * @param registro nombre del registro a modificar
 * @param valor a asignar al registro
 * @return nuevo valor del registro, -1 si no se encuentra registro valido
 */
 int set_registro(char*, int);

 /**
 * @fn get_registro
 * @brief consulta el valor de un registro particular
 * @param pcb estructura que contiene el registro a consultar
 * @param registro nombre del registro a consultar
 * @return valor del registro, -1 si no se encuentra registro valido
 */
 int get_registro(char*);

 /**
 * @fn get_cod_instruccion
 * @brief Transofrma un string de instruccion en el enum que le corresponde
 * @param instruccion String de la intstruccion
 * @return ENUM de la instrucción
 */
 int get_cod_instruccion(char*);

 void interactuar_dispatch(int);

int MMU(int);
int cuanto_leo(char*);

//CICLO DE INSTRUCCIÓN
 /**
 * @fn fetch
 * @brief Consigue una instruccion
 * @return Puntero a la instruccion
 */
 char* fetch();

 /**
 * @fn decode
 * @brief Transofrma el string de la instruccion en un registro de instruccion
 * @param buffer Puntero al string de la instruccion
 * @return Registro de instruccion
 */
 sInstruccion decode(char*);

 /**
 * @fn execute
 * @brief Ejecuta la instrucción
 * @param instruccion Instrucción a ejecutar
 */
 void execute(sInstruccion);

 //INSTRUCCIONES DE LA CPU

 /**
 * @fn exe_SET
 * @brief Le asigna un valor a un registro
 * @param registro String con el nombre del registro
 * @param valor String con el valor
 */
 void exe_SET(char*, char*);

  /**
 * @fn exe_SUM
 * @brief Suma 2 registros, y lo almacena en el primero
 * @param reg_destino Registro en el que se almacenará el resultado
 * @param reg_origen Registro que sera sumado al primero
 */
void exe_SUM(char*, char*);

  /**
 * @fn exe_SUB
 * @brief Resta al Registro Destino el Registro Origen y deja el resultado en el Registro Destino.
 * @param reg_destino Registro en el que se almacenará el resultado
 * @param reg_origen Registro que sera restado del primero
 */
void exe_SUB(char*, char*);

  /**
 * @fn exe_JNZ
 * @brief Si el valor del registro es distinto de cero, actualiza el program counter al número de instrucción
 * @param registro Registro en el que se almacenará el resultado
 * @param numero_instruccion instruccion a la que se salta
 */
void exe_JNZ(char*, char*);

  /**
 * @fn exe_EXIT
 * @brief Finaliza el proceso
 */
void exe_EXIT();

  /**
 * @fn exe_WAIT
 * @brief Vuelve al Kernel a consumir el recurso
 * @param recurso Nombre del recurso
 */
void exe_WAIT(char*);

  /**
 * @fn exe_SIGNAL
 * @brief Vuelve al Kernel a liberar un recurso
 * @param recurso Nombre del recurso
 */
void exe_SIGNAL(char*);

  /**
 * @fn exe_IO_GEN_SLEEP
 * @brief Realiza una operación de I/O
 * @param nombre Nombre del dispositivo I/O
 * @param tiempo Tiempo de espera
 */ 
void exe_IO_GEN(char**);

void exe_IO_STD(char**);

void exe_MOV_IN(char*, char*);

void exe_MOV_OUT(char*, char*);

void exe_RESIZE(int);

void exe_COPY_STRING(int);

//LOGS OBLIGATORIOS
/**
 * @fn log_fetch
 * @brief Loguea la busqueda de una instruccion
 * @param pid Id del proceso
 * @param pc Numero de instrucion a buscar
 */
void log_fetch (int, int);

/**
*@fn 		log_execute
*@brief		Loguea la ejecucion de una instruccion
*@param     pid Id del proceso
*@param     instruccion Instruccion a ejecutar
*@param     parametros Parametros de la instruccion
*/
void log_execute(int, char*, char*);

void log_marco(int, int, int);

void log_rw(int, char*, int, int);

void log_rws(int, char*, int, char*);