#include "entradasalida.h"

int pid;

void crear_interfaz_generica(char* nombre) {
    
	int unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
	int socket_kernel = conectar_kernel(nombre);

	// una vez conectado y avisado que me conecte, me quedo esperando solicitudes
	while(1) {
		int cod_op = recibir_operacion(socket_kernel);
		if (cod_op == OPERACION_IO) {
			int size;
			char* buffer = recibir_buffer(&size, socket_kernel);
			char** instruccion = string_n_split(buffer, 3, " ");
			pid = recibir_operacion(socket_kernel);
			log_operacion(pid, instruccion[0]);

			if (!strcmp(instruccion[0], "IO_GEN_SLEEP")){
				
				int cant_unidades_trabajo = atoi(instruccion[2]);
				usleep(cant_unidades_trabajo*unidad_trabajo * 1000);
				log_info(logger, "Me dormi %d milisegs", cant_unidades_trabajo*unidad_trabajo);
				// multiplicamos x mil para pasar de milisec a microsec (q es lo q toma usleep)

				log_info(logger, "Resultado de %s: io_success", nombre);
				enviar_operacion(socket_kernel, IO_SUCCESS);

			} else {
				log_info(logger, "Resultado de %s: io_failure", nombre);
				enviar_operacion(socket_kernel, IO_FAILURE);
			}

			free(buffer);
			string_array_destroy(instruccion);
		} else {
			return;
		}
	}

	liberar_conexion(socket_kernel);
}

void crear_interfaz_stdin (char* nombre){
	int socket_kernel = conectar_kernel (nombre);
	int socket_memoria = conectar_memoria (nombre); 

	tam_pagina = recibir_operacion(socket_memoria);

	while(1) {
		int cod_op = recibir_operacion(socket_kernel);
		if (cod_op == OPERACION_IO) {
			int size;
			char* buffer = recibir_buffer(&size, socket_kernel);
			char** instruccion = string_split(buffer, " ");
			pid = recibir_operacion(socket_kernel);
			log_operacion(pid, instruccion[0]);
			int tamaño = atoi(instruccion[2]);
			int tamañoVector = atoi(instruccion[3]);
			
			int* vectorDirecciones = recibir_vector(socket_kernel, tamañoVector);
			log_info(logger, "Operacion: %s", instruccion[0]);
			if (!strcmp(instruccion[0], "IO_STDIN_READ")){
				char valor[tamaño];
				printf("Ingrese hasta %i caracteres \n", tamaño);
				for(int i=0; i<tamaño; i++)
					valor[i] = getchar();
				valor[tamaño]='\0';
				getchar();

				log_info(logger, "Valor ingresado: %s", valor);
		int espacioEnPag = tam_pagina-vectorDirecciones[0]%tam_pagina;
		float tam = tamaño;
		if(tamaño<=espacioEnPag){
        enviar_string(valor, socket_memoria, ESCRITURA_STRING);
		enviar_operacion(socket_memoria, pid);
        enviar_operacion(socket_memoria, 1);
        enviar_operacion(socket_memoria, tamaño);
        enviar_operacion(socket_memoria, vectorDirecciones[0]);
    }
    else{
        int i;
        enviar_string(valor, socket_memoria, ESCRITURA_STRING);
		enviar_operacion(socket_memoria, pid);
        enviar_operacion(socket_memoria, tamañoVector);
        enviar_operacion(socket_memoria, espacioEnPag);
        enviar_operacion(socket_memoria, vectorDirecciones[0]);
        for(i=1; i<tamañoVector-1; i++){
            enviar_operacion(socket_memoria, tam_pagina);
            enviar_operacion(socket_memoria, vectorDirecciones[i]);
        }
		if((tamaño-espacioEnPag)%tam_pagina){
			enviar_operacion(socket_memoria, (tamaño-espacioEnPag)%tam_pagina);
			enviar_operacion(socket_memoria, vectorDirecciones[i]);
		}
		
		else{
        	enviar_operacion(socket_memoria, tam_pagina);
        	enviar_operacion(socket_memoria, vectorDirecciones[i]);
		}
    }

				log_info(logger, "Resultado de %s: io_success", nombre);
				enviar_operacion(socket_kernel, IO_SUCCESS);

			} else {
				log_info(logger, "Resultado de %s: io_failure", nombre);
				enviar_operacion(socket_kernel, IO_FAILURE);
			}

			free(buffer);
			string_array_destroy(instruccion);
		} else {
			return;
		}
	}
	
}

void crear_interfaz_stdout (char* nombre){
	int socket_kernel = conectar_kernel (nombre);
	int socket_memoria = conectar_memoria (nombre); 

	tam_pagina = recibir_operacion(socket_memoria);

	while(1) {
		int cod_op = recibir_operacion(socket_kernel);
		if (cod_op == OPERACION_IO) {
			int size;
			char* buffer = recibir_buffer(&size, socket_kernel);
			char** instruccion = string_split(buffer, " ");
			pid = recibir_operacion(socket_kernel);
			log_operacion(pid, instruccion[0]);
			int tamaño = atoi(instruccion[2]);
			int tamañoVector = atoi(instruccion[3]);
			
			int* vectorDirecciones = recibir_vector(socket_kernel, tamañoVector);
			log_info(logger, "Operacion: %s", instruccion[0]);
			if (!strcmp(instruccion[0], "IO_STDOUT_WRITE")){
				float tam = tamaño;
				int size;
				int desplazamiento = vectorDirecciones[0]%tam_pagina;
				int espacioEnPag = tam_pagina-(desplazamiento);
				if(tamaño<=espacioEnPag){
					enviar_operacion(socket_memoria, LECTURA_STRING);
					enviar_operacion(socket_memoria, pid);
					enviar_operacion(socket_memoria, 1);
					enviar_operacion(socket_memoria, tamaño);
					enviar_operacion(socket_memoria, vectorDirecciones[0]);
				}
				else{
					int i;
					enviar_operacion(socket_memoria, LECTURA_STRING);
					enviar_operacion(socket_memoria, pid);
					enviar_operacion(socket_memoria, tamañoVector);
					enviar_operacion(socket_memoria, espacioEnPag);
					enviar_operacion(socket_memoria, vectorDirecciones[0]);
					for(i=1; i<tamañoVector-1; i++){
						enviar_operacion(socket_memoria, tam_pagina);
						enviar_operacion(socket_memoria, vectorDirecciones[i]);
					}
					if((tamaño-espacioEnPag)%tam_pagina){
					enviar_operacion(socket_memoria, (tamaño-espacioEnPag)%tam_pagina);
					enviar_operacion(socket_memoria, vectorDirecciones[i]);
					}
					else{
						enviar_operacion(socket_memoria, tam_pagina);
						enviar_operacion(socket_memoria, vectorDirecciones[i]);
					}
				}
				recibir_operacion(socket_memoria);
				char* cadena = recibir_buffer(&size,socket_memoria);
				printf("%s \n", cadena);
				free(cadena);
				log_info(logger, "Resultado de %s: io_success", nombre);
				enviar_operacion(socket_kernel, IO_SUCCESS);

			} else {
				log_info(logger, "Resultado de %s: io_failure", nombre);
				enviar_operacion(socket_kernel, IO_FAILURE);
			}

			free(buffer);
			string_array_destroy(instruccion);
		} else {
			return;
		}
	}
}

void crear_interfaz_fs(char* nombre){
	int socket_kernel = conectar_kernel (nombre);
	int socket_memoria = conectar_memoria (nombre); 
		tam_pagina = recibir_operacion(socket_memoria);
	while(1){
		op_code cod_op = recibir_operacion(socket_kernel);
		if(cod_op == OPERACION_IO){
			int size;
			char* buffer = recibir_buffer(&size, socket_kernel);
			char** instruccion = string_split(buffer, " ");
			pid = recibir_operacion(socket_kernel);
			log_operacion(pid, instruccion[0]);
			if(!strcmp(instruccion[0], "IO_FS_CREATE"))
				crear_fs(instruccion[2]);
			else if(!strcmp(instruccion[0], "IO_FS_DELETE"))
				eliminar_fs(instruccion[2]);
			else if(!strcmp(instruccion[0], "IO_FS_TRUNCATE"))
				truncar_fs(instruccion[2], atoi(instruccion[3]));
			else if(!strcmp(instruccion[0], "IO_FS_WRITE")){
				int tamaño = atoi(instruccion[3]);
				int tamañoVector = atoi(instruccion[4]);
				int* vectorDirecciones = recibir_vector(socket_kernel, tamañoVector);
				int desplazamiento = vectorDirecciones[0]%tam_pagina;
				int espacioEnPag = tam_pagina-(desplazamiento);
				if(tamaño<=espacioEnPag){
					enviar_operacion(socket_memoria, LECTURA_STRING);
					enviar_operacion(socket_memoria, pid);
					enviar_operacion(socket_memoria, 1);
					enviar_operacion(socket_memoria, tamaño);
					enviar_operacion(socket_memoria, vectorDirecciones[0]);
				}
				else{
					int i;
					enviar_operacion(socket_memoria, LECTURA_STRING);
					enviar_operacion(socket_memoria, pid);
					enviar_operacion(socket_memoria, tamañoVector);
					enviar_operacion(socket_memoria, espacioEnPag);
					enviar_operacion(socket_memoria, vectorDirecciones[0]);
					for(i=1; i<tamañoVector-1; i++){
						enviar_operacion(socket_memoria, tam_pagina);
						enviar_operacion(socket_memoria, vectorDirecciones[i]);
					}
					if((tamaño-espacioEnPag)%tam_pagina){
					enviar_operacion(socket_memoria, (tamaño-espacioEnPag)%tam_pagina);
					enviar_operacion(socket_memoria, vectorDirecciones[i]);
					}
					else{
						enviar_operacion(socket_memoria, tam_pagina);
						enviar_operacion(socket_memoria, vectorDirecciones[i]);
					}
				}
				recibir_operacion(socket_memoria);
				char* cadena = recibir_buffer(&size,socket_memoria);
				escribir_fs(instruccion[2], cadena, atoi(instruccion[5]));
				free(cadena);
				free(vectorDirecciones);
			}
			else if(!strcmp(instruccion[0], "IO_FS_READ"))
				leer_fs(instruccion);
			else{
				log_info(logger, "Resultado de %s: io_failure", nombre);
				enviar_operacion(socket_kernel, IO_FAILURE);
				string_array_destroy(instruccion);
				continue;
			}
			enviar_operacion(socket_kernel, IO_SUCCESS);
			string_array_destroy(instruccion);
		}
		else
			return;
	}
}

void crear_fs(char* nombre){
	log_creacion(pid, nombre);
}

void eliminar_fs(char* nombre){
	log_eliminacion(pid, nombre);
}

void truncar_fs(char* nombre, int tamaño){
	log_truncamiento(pid, nombre, tamaño);
}

void escribir_fs(char* archivo, char* cadena, int DF){
	printf("Escribiendo en archivo %s: %s\n", archivo, cadena);
	log_escritura(pid, archivo, strlen(cadena), DF);
}

void leer_fs(char** instruccion){
	log_info(logger, "archivo leido (en realidad no)");
}



int conectar_kernel (char* nombre){
	int socket;
	char* ip;
	char* puerto;

	ip = config_get_string_value(config, "IP_KERNEL");
	puerto = config_get_string_value(config, "PUERTO_KERNEL");

	socket = crear_conexion(ip, puerto, "Kernel");
	enviar_string(nombre, socket, NUEVA_IO);

	return socket;
}

int conectar_memoria (char* nombre){
	int socket;
	char* ip;
	char* puerto;

	ip = config_get_string_value(config, "IP_MEMORIA");
	puerto = config_get_string_value(config, "PUERTO_MEMORIA");

	socket = crear_conexion(ip, puerto, "Memoria");
	enviar_operacion(socket, NUEVA_IO);

	return socket;
}


//LOGS OBLIGATORIOS
void log_operacion(int pid, char* operacion){
	log_info(logger, "PID: %d - Operación: %s", pid, operacion);
}

void log_creacion(int pid, char* nombre){
	log_info(logger, "PID: %d - Crear Archivo %s", pid, nombre);
}

void log_eliminacion(int pid, char* nombre){
	log_info(logger, "PID: %d - Eliminar Archivo %s", pid, nombre);
}

void log_truncamiento(int pid, char* nombre, int tamaño){
	log_info(logger, "PID: %d - Truncar Archivo: %s - Tamaño: %d", pid, nombre, tamaño);
}

void log_escritura(int pid, char* archivo, int tamaño, int DF){
	log_info(logger, "PID: %d - Escribir en Archivo: %s - Tamaño: %d - Puntero Archivo: %d", pid, archivo, tamaño, DF);
}