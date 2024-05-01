#include <string.h>
#include <stdint.h>

// TODO: Repetido con kernel.h, generalizar en una sola biblioteca para todos los modulos 
typedef struct
{
	uint32_t PC;
	uint8_t AX;
	uint8_t BX;
	uint8_t CX;
	uint8_t DX;
	uint32_t EAX;
	uint32_t EBX;
	uint32_t ECX;
	uint32_t EDX;
	uint32_t SI;
	uint32_t DI;
} t_registros;

typedef struct 
{
	int pid;
	int pc;
	int quantum;
	t_registros registros;
	char* estado;
	char** instrucciones;
} t_pcb;

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