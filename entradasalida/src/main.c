#include <entradasalida/entradasalida.h>

t_log* logger;
t_config* config;

int main() {
	// el logger es lo unico compartido (habria que preguntar si no hay q hacer un logger por interfaz)
	logger = log_create("logIO.log", "LOGS IO", 1, LOG_LEVEL_INFO);
	
	pthread_t hilo_impresora;
	pthread_t hilo_monitor;

	pthread_create(&hilo_impresora, NULL, crear_interfaz_generica, crear_args_IO("IMPRESORA", "interfaz.config"));

	sleep(10);

	pthread_create(&hilo_monitor, NULL, crear_interfaz_generica, crear_args_IO("MONITOR", "interfaz.config"));

	pthread_join(hilo_impresora, NULL);
	pthread_join(hilo_monitor, NULL);

    log_destroy(logger);
    return 0;
}