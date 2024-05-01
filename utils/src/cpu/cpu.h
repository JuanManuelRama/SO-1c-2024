#include <string.h>
#include <utils/structs.h>

/**
 * @fn set_registro
 * @brief modifica el valor de un registro particular
 * @param pcb estructura que contiene el registro a modificar
 * @param registro nombre del registro a modificar
 * @param valor a asignar al registro
 * @return nuevo valor del registro, -1 si no se encuentra registro valido
 */
 int set_registro(t_pcb, char*, int);

 /**
 * @fn get_registro
 * @brief consulta el valor de un registro particular
 * @param pcb estructura que contiene el registro a consultar
 * @param registro nombre del registro a consultar
 * @return valor del registro, -1 si no se encuentra registro valido
 */
 int get_registro(t_pcb, char*);