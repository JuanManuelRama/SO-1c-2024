#include "entradasalida.h"

int pid;
int CANT_BLOQUES;
int TAM_BLOQUE;
char* DIR;
char* DIR_METADATA;
t_bitarray* bitmap;
void* BLOQUES;
typedef struct 
{
	char* nombre;
	int base;
	int largo;
} entradaFat;

t_list* FAT;

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

	iniciar_fs();

	while(1){
		op_code cod_op = recibir_operacion(socket_kernel);
		if(cod_op == OPERACION_IO){
			int size;
			char* buffer = recibir_buffer(&size, socket_kernel);
			char** instruccion = string_split(buffer, " ");
			pid = recibir_operacion(socket_kernel);
			log_operacion(pid, instruccion[0]);

			// CASO CREAR
			if(!strcmp(instruccion[0], "IO_FS_CREATE")){
				if(!crear_fs(instruccion[2])){
					log_info(logger, "No hay espacio libre en disco para crear archivo");
					enviar_operacion(socket_kernel, IO_FAILURE);
				}
			}
			// CASO DELETE
			else if(!strcmp(instruccion[0], "IO_FS_DELETE"))
				eliminar_fs(instruccion[2]);
			// CASO TRUNCAR
			else if(!strcmp(instruccion[0], "IO_FS_TRUNCATE")) {
				truncar_fs(instruccion[2], atoi(instruccion[3]));
			}
			// CASO ESCRIBIR EN ARCHIVO
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
			// CASO LEER DE ARCHIVO
			else if(!strcmp(instruccion[0], "IO_FS_READ")) {
				int tamaño = atoi(instruccion[3]);
				int tamañoVector = atoi(instruccion[4]);
				int* vectorDirecciones = recibir_vector(socket_kernel, tamañoVector);
				int desplazamiento = vectorDirecciones[0]%tam_pagina;
				int espacioEnPag = tam_pagina-(desplazamiento);

				// para leer del archivo necesitamos los datos: nombre de archivo, 
				// posicion de donde empezar a leer y cant de bytes a leer
				char *nombreArchivo = instruccion[2];
				int posicionEnArchivo = atoi(instruccion[5]);

				char *leidoDeArchivo = leer_fs(nombreArchivo, posicionEnArchivo, tamaño);

				if(tamaño<=espacioEnPag){
					enviar_string(leidoDeArchivo, socket_memoria, ESCRITURA_STRING);

					enviar_operacion(socket_memoria, pid);
					enviar_operacion(socket_memoria, 1);
					enviar_operacion(socket_memoria, tamaño);
					enviar_operacion(socket_memoria, vectorDirecciones[0]);
				}
				else{
					int i;
					enviar_string(leidoDeArchivo, socket_memoria, ESCRITURA_STRING);

					enviar_operacion(socket_memoria, pid);
					enviar_operacion(socket_memoria, tamañoVector);
					enviar_operacion(socket_memoria, espacioEnPag);
					enviar_operacion(socket_memoria, vectorDirecciones[0]);
					for(i = 1; i < tamañoVector-1; i++){
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
				free(vectorDirecciones);
				free(leidoDeArchivo); // puesto que leer_fs hizo el malloc
			}
			// CASO DE OPERACION NO SOPORTADA POR FS
			else{
				log_info(logger, "Resultado de %s: io_failure", nombre);
				enviar_operacion(socket_kernel, IO_FAILURE);
				free(buffer);
				string_array_destroy(instruccion);
				continue;
			}

			// SALIO TODO BIEN, DEVOLVEMOS SUCCESS Y LIBERAMOS ESTRUCTURAS
			enviar_operacion(socket_kernel, IO_SUCCESS);
			free(buffer);
			string_array_destroy(instruccion);
		}
		else
			return;
	}
}

bool crear_fs(char* nombre){
	int direccion = -1;
	log_creacion(pid, nombre);

	for(int i = 0; i < CANT_BLOQUES; i++){
		if (!bitarray_test_bit(bitmap, i)){
			bitarray_set_bit(bitmap, i);
			direccion = i;
			break;
		}
	}

	if (direccion == -1)
		return false;
	
	//Creamos el archivo de metadata
	t_dictionary* parametros = dictionary_create();

	char buffer1[100];
	char buffer2[100];

    sprintf(buffer1, "%d", direccion);
	dictionary_put(parametros, "BLOQUE_INICIAL", buffer1);

	sprintf(buffer2, "%d", 0);
	dictionary_put(parametros, "TAMANIO_ARCHIVO", buffer2);

	t_config* metadata = malloc(sizeof(t_config));
	metadata->path = armarPathMetadata(nombre);
	metadata->properties = parametros;

	config_save(metadata);

	//Agregamos su entrada en la tabla fat
	entradaFat* entrada = malloc(sizeof(entradaFat));
	char* nombreDeArchivo = malloc(255); //max tamaño de nombre de archivo en unix
	strcpy(nombreDeArchivo, nombre);
	entrada->nombre = nombreDeArchivo;
	entrada->base = direccion;
	entrada->largo = 0;
	list_add(FAT, entrada);

	return true;
}

void eliminar_fs(char* nombre){
	log_eliminacion(pid, nombre);
	
	// armamos el path absoluto (siempre deben estar en la carpeta de metadata)
	char* path = armarPathMetadata(nombre);

	// usamos las commons de config para sacar la metadata del archivo
	t_config* archivo = config_create(path);

	//Inner function para buscar la entrada del archivo en la fat
	bool esArchivo (void* elem) {
    	entradaFat* entrada = (entradaFat*)elem;
    	return !strcmp(entrada->nombre, nombre);
	}

	entradaFat* entrada = list_find(FAT, esArchivo);

	int base = config_get_int_value(archivo, "BLOQUE_INICIAL");
	int tamanio = config_get_int_value(archivo, "TAMANIO_ARCHIVO");
	
	int bloques_ocupados = tamanio / TAM_BLOQUE + 1;

	// liberamos los bloques que ocupaba
	for (int i = 0; i < bloques_ocupados; i++) {
		bitarray_clean_bit(bitmap, base + i);
	}

	//Eliminamos la entrada del archivo en la fat
	list_remove_element(FAT, entrada);
	free (entrada->nombre);
	free (entrada);

	// eliminamos el archivo
	remove(path);

	// liberamos la estructurita de config
	config_destroy(archivo);

	free(path);
}

void truncar_fs(char* nombre, int tamaño){
	//Inner function para buscar la entrada del archivo en la fat
	bool esArchivo (void* elem) {
    	entradaFat* entrada = (entradaFat*)elem;
    	return !strcmp(entrada->nombre, nombre);
	}

	entradaFat* entrada = list_find(FAT, esArchivo);

	//limpiamos los bits que ocupaba
	for(int i = 0; i <= (entrada->largo/TAM_BLOQUE); i++)
		bitarray_clean_bit(bitmap, entrada->base + i);
	
	entrada->largo = tamaño; // actualizamos al nuevo tamaño en la tabla

	//seteamos los nuevos bits que ocupa
	for(int i = 0; i <= (entrada->largo/TAM_BLOQUE); i++)
		bitarray_set_bit(bitmap, entrada->base + i);

	
	// persistimos a disco (o sea digamos, archivo de metadata)
	char *path = armarPathMetadata(entrada->nombre);
	t_config *metadata = config_create(path);

	char bufferString[100];
    sprintf(bufferString, "%d", tamaño);
	config_set_value(metadata, "TAMANIO_ARCHIVO", bufferString);
	config_save(metadata);

	log_truncamiento(pid, nombre, tamaño);
}

void escribir_fs(char* archivo, char* cadena, int DF){
	printf("Escribiendo en archivo %s: %s\n", archivo, cadena);
	log_escritura(pid, archivo, strlen(cadena), DF);
}

void leer_fs(char *nombre, int offset, int cantALeer){
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

void compactar (){
	entradaFat* entrada;
	char* path;
	t_config* metadata;
	void* bufferBloques;
	int largoArchivo;
	char bufferString[100];
	int inicioHueco;

	for (inicioHueco = 0; bitarray_test_bit(bitmap, inicioHueco); inicioHueco++) {
		; // buscamos el primer hueco libre
	}

	for(int inicioArchivo = 0; inicioArchivo < CANT_BLOQUES; inicioArchivo++){
		if(bitarray_test_bit(bitmap, inicioArchivo)){
			//Inner function para buscar la entrada del archivo en la fat
			bool esEntrada (void* elem) {
				entradaFat* entrada = (entradaFat*)elem;
				return (entrada->base == inicioArchivo);
			}

			entrada = list_find(FAT, esEntrada);
			entrada->base = inicioHueco;
			largoArchivo = entrada->largo;
			bufferBloques = malloc(largoArchivo);

			//base en metadata = inicioHueco
			path = armarPathMetadata(entrada->nombre);
			metadata = config_create(path);

			sprintf(bufferString, "%d", inicioHueco); // convertimos a string el numero 

			config_set_value(metadata, "BLOQUE_INICIAL", bufferString);
			config_save(metadata); // guardamos en archivo metadata

			//limpio largo/TAM_BLOQUE bits desde inicioArchivo en bitmap
			for(int i = inicioArchivo; i <= (inicioArchivo + largoArchivo/TAM_BLOQUE); i++)
				bitarray_clean_bit(bitmap, i);
			
			//seteo largo/TAM_BLOQUE bits desde inicioHueco en bitmap
			for(int i = inicioHueco; i <= (inicioHueco + largoArchivo/TAM_BLOQUE); i++)
				bitarray_set_bit(bitmap, i);

			//copio a buffer largo bytes desde inicioArchivo de bloques.dat
			memcpy(bufferBloques, BLOQUES + inicioArchivo*TAM_BLOQUE, largoArchivo);

			//copio de buffer largo bytes desde inicioHueco a bloques.dat
			memcpy(BLOQUES + inicioHueco*TAM_BLOQUE, bufferBloques, largoArchivo);

			inicioHueco += largoArchivo;
			inicioArchivo = inicioHueco;
			free(bufferBloques);
		}
	}
}

void iniciar_fs(){
	//VARIABLES DEL CONFIG
	CANT_BLOQUES = config_get_int_value(config, "BLOCK_COUNT");
	TAM_BLOQUE = config_get_int_value(config, "BLOCK_SIZE");
	DIR = config_get_string_value(config, "PATH_BASE_DIALFS");

	FILE *archivo_bitmap;
	FILE *archivo_bloques;

	//CREO BITMAP
	char* path = malloc(strlen(DIR) + strlen("bitmap.dat") + 2);

	mkdir(DIR, 0777);
	strcpy(path, DIR);
	strcat(path, "/");
	strcat(path, "bitmap.dat");

	archivo_bitmap = fopen(path, "w+b");

    truncate(path, CANT_BLOQUES/8);	// un bit por bloque

    bitmap = bitarray_create_with_mode(mmap(NULL , CANT_BLOQUES/8, PROT_WRITE | PROT_READ, MAP_SHARED, archivo_bitmap->_fileno, 0), CANT_BLOQUES/8, MSB_FIRST);
	
	for(int i = 0; i < CANT_BLOQUES; i++)
		bitarray_clean_bit(bitmap, i);

	free(path);

	fclose(archivo_bitmap);

	// Creamos carpeta de metadata
	DIR_METADATA = malloc(strlen(DIR) + strlen("metadata") + 2);

	strcpy(DIR_METADATA, DIR);
	strcat(DIR_METADATA, "/metadata");

	mkdir(DIR_METADATA, 0777);
	
	//CREO ARCHIVO DE BLOQUES
	path = malloc(strlen(DIR) + strlen("bloques.dat") + 2);
	strcpy(path, DIR);
	strcat(path, "/");
	strcat(path, "bloques.dat");
	archivo_bloques = fopen(path, "w+b");
	truncate(path, CANT_BLOQUES*TAM_BLOQUE);

	BLOQUES = mmap(NULL , CANT_BLOQUES*TAM_BLOQUE, PROT_WRITE | PROT_READ, MAP_SHARED, archivo_bloques->_fileno, 0);

	//Inicializo tabla FAT
	FAT = list_create();
	
	fclose(archivo_bloques);

	free(path);
}

char* armarPathMetadata (char* nombre){
	char* path = malloc(strlen(DIR_METADATA) + strlen(nombre) + 6);
	strcpy(path, DIR_METADATA);
	strcat(path, "/");
	strcat(path, nombre);
	strcat(path, ".txt");
	return path;
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