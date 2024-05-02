#include <string.h>
#include <utils/structs.h>
#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <sockets/sockets.h>
#include <pthread.h>
#include <readline/readline.h>
#include <commons/string.h>

typedef enum{
    SET,
    MOV_IN,
    MOV_OUT,
    SUM,
    SUB,
    JNZ //faltan casi todas, otro día lo llenaré
}instrucciones;

typedef struct{
    int cod_instruccion;
    char** componentes;
}sInstruccion;

extern t_pcb pcb;

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
 * @fn set
 * @brief Le asigna un valor a un registro
 * @param registro String con el nombre del registro
 * @param valor String con el valor
 */
 void set(char*, char*);

  /**
 * @fn sum
 * @brief Suma 2 registros, y lo almacena en el primero
 * @param reg1 Registro en el que se almacenará el resultado
 * @param reg2 Registro al que se le sumará el primero
 */
void sum(char*, char*);