#include <memoria/memoria.h>

int TAM_MEMORIA;
short CANT_PAG;
short TAM_PAG;
char* PATH_INSTRUCCIONES;

void* memoria_contigua;

t_log* logger;
t_config* config;

t_tablaMemoria (*tablaMemoria);

int main (){
	char* puerto;
	int socket_servidor;
	int socket_cpu;
	int socket_kernel;
	int socket_IO;
	pthread_t hilo_cpu, hilo_kernel, hilo_IO;

	inicializar_memoria();

	// buscamos dato en config e inicializamos servidor para cpu y kernel como clientes
	puerto = buscar("PUERTO_ESCUCHA");
	socket_servidor = iniciar_servidor(puerto, "Memoria");

	//socket_cpu = esperar_cliente("CPU", socket_servidor);
	//pthread_create(&hilo_cpu, NULL, interactuar, (void*)socket_cpu);

	socket_kernel = esperar_cliente("Kernel", socket_servidor);
	pthread_create(&hilo_kernel, NULL, interactuar_Kernel, (void*)socket_kernel);

	//socket_IO = esperar_cliente("IO", socket_servidor);
	//pthread_create(&hilo_IO, NULL, interactuar, (void*)socket_IO);


	//cargar_proceso("instru.txt");
	//Enviar la lProcesos al Kernel para que la añada al PCB
	pthread_join(hilo_kernel, NULL);



	//pthread_join(hilo_cpu, NULL);

	//pthread_join(hilo_IO, NULL);

	finalizar_memoria();
	return 0;
}

