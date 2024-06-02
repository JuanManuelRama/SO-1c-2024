#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>
#include <utils/structs.h>

extern t_log* logger;
extern t_config* config;
typedef enum
{
	MENSAJE,
	PAQUETE,
	NUEVO_PROCESO,
	PROCESO,
	PCB,
	SLEEP,
	FETCH,
	NUEVA_IO,
	OPERACION_IO,
	FINALIZACION,
	FIN_DE_QUANTUM,
	RECURSO,
	PAGINA,
	MAS_PAGINA,
	MENOS_PAGINA,
	TAM_PROCESO,
	OOM,
	IO,
	IO_FAILURE,
	IO_SUCCESS
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

/**
* @fn      iniciar_servidor
* @brief   Inicializa el Servidor
* @param   puerto Puerto del Servidor
* @param   modulo que es inicializado como servidor
* @return  Socket del Servidor
*/
int iniciar_servidor(char*, char*);

/**
* @fn      esperar_cliente
* @brief   Espera al Cliente
* @param   cliente Cliente del Servidor
* @param   Socket del Servidor
* @return  Socket del Cliente
*/
int esperar_cliente(char*, int);

/**
* @fn     recibir_paquete
* @brief  Recibe el paquete
* @param  Socket del Cliente
* @return Paquete
*/
t_list* recibir_paquete(int);

/**
* @fn     recibir_mensaje
* @brief  Recibe el Mensaje
* @param  Socket del Cliente
* @return Nada
*/
void recibir_mensaje(int);

/**
* @fn    recibir_operacion
* @brief Recibe la Operación
* @param   Socket del Cliente
* @return Código de Operación, o -1 en caso de error
*/
int recibir_operacion(int);



/**
* @fn       crear_conexion
* @brief    Crea la conexión
* @param    ip
* @param    puerto
* @param	servidor Nombre del servidor
* @return   Socket del Cliente
*/
int crear_conexion(char*, char*, char*);

/**
* @fn       enviar_mensaje
* @brief    Envía un mensaje
* @param    mensaje
* @param    Socket del Cliente
* @return   nada
*/
void enviar_mensaje(char*, int);

/**
* @fn       enviar_string
* @brief    Envía la string a traves del socket con el codigo dado
* @param    string
* @param    Socket del Cliente
* @param    codigo de operacion a enviar
* @return   nada
*/
void enviar_string(char*, int, int);

/**
 * @fn       enviar_puntero
 * @brief    Envía un puntero a traves del socket con el codigo dado
 * @param    puntero
 * @param    Socket del Cliente
 * @param    codigo de operacion a enviar
 * @return   nada
 */
void enviar_puntero(void*, int, int);

/**
 * @fn       enviar_int
 * @brief    Envía un int a traves del socket con el codigo dado
 * @param    int
 * @param    Socket del Cliente
 * @param    codigo de operacion a enviar
 * @return   nada
 */
void enviar_int(int, int, int);


/**
* @fn       crear_paquete
* @brief    Crea un paquete
* @param    Ninguno
* @return   Paquete
*/
t_paquete* crear_paquete(void);

/**
* @fn       agregar_a_paquete
* @brief    Agrega al paquete
* @param    paquete
* @param    valor a agregar
* @param    tamaño del valor
* @return   nada
*/
void agregar_a_paquete(t_paquete*, void*, int);

/**
* @fn       enviar_paquete
* @brief    Envía el paquete
* @param    paquete
* @param    socket_cliente
* @return   nada
*/
void enviar_paquete(t_paquete*, int);

/**
* @fn      liberar_conexion
* @brief   Libera la conexión
* @param   socket_cliente
* @return  nada
*/
void liberar_conexion(int);

/**
* @fn      eliminar_paquete
* @brief   Elimina el Paquete
* @param   paquete Paquete a elminar
* @return  Socket del Servidor
*/
void eliminar_paquete(t_paquete*);

/**
	* @fn       buscar
	* @brief    Busca en config, loggea error de no encontrarlo
    * @param    key Palabra a buscar
    * @return   Resultado de la búsqueda
	*/
char* buscar (char*);

/**
	* @fn       interactuar
	* @brief    Interactua con el cliente
    * @param    socket_servidor Palabra a buscar
    * @return   Dios sabrá
	*/
void* interactuar (int);


/**
	* @fn       recibir_buffer
	* @brief    Recibe un buffer
    * @param    size Tamaño del buffer
	* @param	socket_cliente
    * @return   buffer
	*/
void* recibir_buffer(int*, int);


/**
	* @fn       serializar_paquete
	* @brief    Serializa un paquete
    * @param    paquete P
	* @param	bytes Cantidad de bytes del paquete
    * @return   magia (literalmente)!
	*/
void* serializar_paquete(t_paquete*, int);

void enviar_pcb(t_pcb pcb, int socket_cliente, int codigo_op);

/**
 * @fn enviar_codigo_op
 * @brief envia codigo de operacion
 * @param socket_cliente
 * @param codigo de operacion 
 */
void enviar_operacion(int, int);


t_pcb pcb_deserializar(int);

int recibir_int (int);

void* recibir_puntero(int);