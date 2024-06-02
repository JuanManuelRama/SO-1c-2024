#include <memoria/memoria.h>

int TAM_MEMORIA;
int RETARDO;
short CANT_PAG;
short TAM_PAG;
char* PATH_INSTRUCCIONES;
t_proceso* proceso;
t_bitarray* bitmap;

void* memoria_contigua;

t_log* logger;
t_config* config;

t_tablaMemoria (*tablaMemoria);

int main (){
	char* puerto;
	int socket_servidor;
	int socket_cliente;
	int socket_IO;
	pthread_t hilo_cpu, hilo_kernel, hilo_IO;

	inicializar_memoria();

	// buscamos dato en config e inicializamos servidor para cpu y kernel como clientes
	puerto = buscar("PUERTO_ESCUCHA");
	socket_servidor = iniciar_servidor(puerto, "Memoria");

	socket_cliente = esperar_cliente("CPU", socket_servidor);
	pthread_create(&hilo_cpu, NULL, interactuar_cpu, (void*)socket_cliente);

	socket_cliente = esperar_cliente("Kernel", socket_servidor);
	pthread_create(&hilo_kernel, NULL, interactuar_Kernel, (void*)socket_cliente);

	//socket_IO = esperar_cliente("IO", socket_servidor);
	//pthread_create(&hilo_IO, NULL, interactuar, (void*)socket_IO);

	// me quedo escuchando nuevas interfaces que se puedan conectar
	pthread_create(&hilo_IO, NULL, escuchar_nuevas_IO, (void*)socket_servidor);


	pthread_join(hilo_kernel, NULL);
	pthread_join(hilo_cpu, NULL);

	//pthread_join(hilo_IO, NULL);

	finalizar_memoria();
	return 0;
}

