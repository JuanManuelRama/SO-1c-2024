#include <stdint.h>
#include <commons/collections/queue.h>

typedef enum{
	INICIAR_PROCESO,
	FINALIZAR_PROCESO,
	EJECUTAR_SCRIPT,
	DETENER_PLANIFICACION,
	INICIAR_PLANIFICACION,
	MULTIPROGRAMACION,
	PROCESO_ESTADO
}fConsola;


typedef enum{
	NEW,
	READY,
	READY_PLUS,
	RUNNING,
	BLOCKED,
	FINISHED
}estados;

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
	estados estado;
	void* instrucciones;
} t_pcb;

typedef struct
{
	char* nombre;
	int instancias;
	t_queue* cBloqueados;
} t_recurso;

typedef struct
{
	int pid;
	int marco;
	int pagina;
} t_entradaTLB;
