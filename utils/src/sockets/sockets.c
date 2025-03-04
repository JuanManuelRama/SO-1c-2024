#include <sockets/sockets.h>

int iniciar_servidor(char* puerto, char* modulo)
{
    int socket_servidor;

	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL,puerto, &hints, &servinfo);

	// Creamos el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family,
							servinfo->ai_socktype,
							servinfo->ai_protocol);

	// Asociamos el socket a un puerto
	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes
	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);

	printf("%s inicializado como servidor \n", modulo);

	return socket_servidor;
}

int esperar_cliente(char* cliente, int socket_servidor)
{
    // Aceptamos un nuevo cliente
	printf("Esperando a %s \n", cliente);
	int socket_cliente;
	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	socket_cliente = accept(socket_servidor, NULL, NULL);
	printf("Se conecto %s \n", cliente);

	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	printf("Me llego el mensaje: %s \n", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}

t_paquete* recibir_recurso(int socket_cliente) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    
    recv(socket_cliente, &(paquete->codigo_operacion), sizeof(int), MSG_WAITALL);
    paquete->buffer = malloc(sizeof(t_buffer));
    recv(socket_cliente, &(paquete->buffer->size), sizeof(int), MSG_WAITALL);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    recv(socket_cliente, paquete->buffer->stream, paquete->buffer->size, MSG_WAITALL);

    return paquete;
}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

int crear_conexion(char *ip, char* puerto, char* servidor)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	// Ahora vamos a crear el socket.
	int socket_cliente =  socket(server_info->ai_family,
                         server_info->ai_socktype,
                         server_info->ai_protocol);

	// Ahora que tenemos el socket, vamos a conectarlo
	setsockopt(socket_cliente, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);
	printf("Conexión establecida con %s \n", servidor);

	return socket_cliente;
}

void enviar_operacion(int socket_cliente, int codigo_op){
	void* codigo = malloc(sizeof(op_code));

	memcpy(codigo, &codigo_op, sizeof(int));

	send(socket_cliente, codigo, sizeof(op_code), MSG_NOSIGNAL);
	free(codigo);
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	enviar_string(mensaje, socket_cliente, MENSAJE);
}


void enviar_string(char* string, int socket_cliente, int codigo_op)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = codigo_op;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(string) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, string, paquete->buffer->size);

	// aca se le suma 2 veces el tamaño de un int, entiendo que es por el op_code y el int size del paquete
	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, MSG_NOSIGNAL);
	free(a_enviar);
	eliminar_paquete(paquete);
}


void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

char* buscar(char* clave){
    if(config_has_property(config, clave))
		return config_get_string_value(config, clave);
	else
	    log_error(logger, "No se ha encontrado %s", clave);
		return NULL;
}

void* interactuar(int socket_cliente){
		while (1) {
		int cod_op = recibir_operacion(socket_cliente);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(socket_cliente);
			break;
		case -1:
			log_error(logger, "el cliente se desconecto");
			return EXIT_FAILURE;
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
}

void enviar_int(int valor, int socket_cliente, int codigo_op)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = codigo_op;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = sizeof(int);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, &valor, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void enviar_puntero(void* puntero, int socket_cliente, int codigo_op)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = codigo_op;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = sizeof(void*);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, &puntero, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}


void enviar_pcb(t_pcb pcb, int socket_cliente, int codigo_op)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	int corrimiento = 0;

	paquete->codigo_operacion = codigo_op; // Codigo de operacion a enviar
	paquete->buffer = malloc(sizeof(t_buffer)); 
	paquete->buffer->size = sizeof(int) * 5 + sizeof(t_registros) + sizeof(void *); // Tañano del stream de datos a enviar
	corrimiento = 0; // Seteamos cuanto nos moveremos
	paquete->buffer->stream = malloc(paquete->buffer->size);
	
	memcpy(paquete->buffer->stream + corrimiento, &pcb.pid, sizeof(int)); // Guardado el PID
	corrimiento += sizeof(int);

	memcpy(paquete->buffer->stream + corrimiento, &pcb.pc, sizeof(int)); // Guardado el PC
	corrimiento += sizeof(int);

	memcpy(paquete->buffer->stream + corrimiento, &pcb.quantum, sizeof(int)); // Guardado el Quantum
	corrimiento += sizeof(int);

	memcpy(paquete->buffer->stream + corrimiento, &pcb.registros, sizeof(t_registros)); // Guardados los Registros
	corrimiento += sizeof(t_registros);

	memcpy(paquete->buffer->stream + corrimiento, &pcb.estado, sizeof(int)); // Guardado el Estado
	corrimiento += sizeof(int);

	memcpy(paquete->buffer->stream + corrimiento, &pcb.instrucciones, sizeof(void*)); // Guardada la instruccion

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free (a_enviar);

	eliminar_paquete(paquete);
}

void enviar_vector(int* vector, int tamaño, int socket_cliente){
	for (int i=0; i < tamaño; i++)
        enviar_operacion(socket_cliente, vector[i]);
}

int* recibir_vector(int socket_cliente, int tamañoVector){
	int* vector = calloc(tamañoVector, sizeof(int));
	for(int i=0; i<tamañoVector; i++)
		vector[i]=recibir_operacion(socket_cliente);
	return vector;
}

int recibir_int(int socket_cliente){
	int valor;
	void* stream=recibir_buffer(&valor, socket_cliente);
	memcpy(&(valor),stream, sizeof(int));
	free(stream);
	return valor;
}

void* recibir_puntero(int socket_cliente){
	void* puntero;
	int size;
	void* stream=recibir_buffer(&size, socket_cliente);
	memcpy(&(puntero),stream, sizeof(void*));
	free(stream);
	return puntero;
}

t_pcb pcb_deserializar(int socket_cliente){
    t_pcb pcb;
	int size;
	void* stream=recibir_buffer(&size, socket_cliente);
    memcpy(&(pcb),stream, sizeof(t_pcb));
    free(stream);
    return pcb;
}
