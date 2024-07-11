#include "entradasalida.h"

int pid;
int CANT_BLOQUES;
int TAM_BLOQUE;
char* DIR_BASE;
char* DIR_METADATA;
t_bitarray* bitmap;
void* BLOQUES;
typedef struct 
{
	char* nombre;
	int bloqueInicial;
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
			config_destroy(config);
			liberar_conexion(socket_kernel);
			return;
		}
	}
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
			// RUTINA DE SALIDA, LIBERAMOS MEMORIA
			liberar_conexion(socket_kernel);
			liberar_conexion(socket_memoria);
			config_destroy(config);
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
			// RUTINA DE SALIDA, LIBERAMOS MEMORIA
			liberar_conexion(socket_kernel);
			liberar_conexion(socket_memoria);
			config_destroy(config);
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
				// instruccion[2] = nombre, [3] = tamaño a truncar
				log_truncamiento(pid, instruccion[2], atoi(instruccion[3]));
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

				if (!escribir_fs(instruccion[2], cadena, atoi(instruccion[5]))){
					log_error(logger, "La cadena a escribir no entra en el archivo");
				}

				//instruccion[2] = nombre, [5] = offset a parter del cual escribir dentro del archivo
				log_escritura(pid, instruccion[2], strlen(cadena), atoi(instruccion[5]));

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
		else {
			// RUTINA DE SALIDA, LIBERAMOS MEMORIA
			liberar_conexion(socket_kernel);
			liberar_conexion(socket_memoria);
			free(DIR_BASE);
			free(DIR_METADATA);
			
			// el puntero de inicio de mapeo se guardo en el campo bitarray cuando 
			// llamamos bitarray_create_with_mode(mmap(...), ...)
			munmap(bitmap->bitarray, bitmap->size);

			munmap(BLOQUES, TAM_BLOQUE*CANT_BLOQUES);

			bitarray_destroy(bitmap);
			free(BLOQUES);

			list_destroy_and_destroy_elements(FAT, liberarEntradaFat);
			return;	
		}
	}
}

// util para destruir entrada fat
void liberarEntradaFat(void *elem) {
	entradaFat* entrada = (entradaFat*)elem;
	free(entrada->nombre);
	free(entrada);
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
	entrada->bloqueInicial = direccion;
	entrada->largo = 0;
	list_add(FAT, entrada);

	dictionary_destroy(parametros);
	free(metadata->path);
	free(metadata);

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

	int bloqueBase = entrada->bloqueInicial;
	float tamanio = entrada->largo; 
	// int base = config_get_int_value(archivo, "BLOQUE_INICIAL");
	// float tamanio = config_get_float_value(archivo, "TAMANIO_ARCHIVO");

	int bloques_ocupados = ceil(tamanio / TAM_BLOQUE);

	// liberamos los bloques que ocupaba
	for (int i = 0; i < bloques_ocupados; i++) {
		bitarray_clean_bit(bitmap, bloqueBase + i);
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
	// HAY CUATRO CASOS POSIBLES:
	// - Truncar al mismo o menos tamaño (achicar o mantener): siempre se puede, facilito
	// - Truncar a mas: - me alcanza el espacio libre contiguo, no hace falta compactar
	// 					- me alcanza el espacio pero no esta contiguo -> compactar
	// no me alcanza el espacio, tiro error y a otra cosa

	//Inner function para buscar la entrada del archivo en la fat
	bool esArchivo (void* elem) {
    	entradaFat* entrada = (entradaFat*)elem;
    	return !strcmp(entrada->nombre, nombre);
	}

	entradaFat* entrada = list_find(FAT, esArchivo);

	if (entrada == NULL) {
		log_error(logger, "Archivo %s no existe (o no fue creado por este FS)", nombre);
		return;
	}

	int bloquesActuales, bloquesFinales;

	float largoArchivo = entrada->largo;
	float tamañoDeseado = tamaño;

	bloquesActuales = ceil(largoArchivo / TAM_BLOQUE);
	// si el tamaño es 0 igualmente se le asigna un bloque
	if (!bloquesActuales){
		bloquesActuales=1;
	}

	bloquesFinales = ceil(tamañoDeseado / TAM_BLOQUE);
	// si el tamaño es 0 igualmente se le asigna un bloque
	if (!bloquesFinales){
		bloquesFinales=1;
	}


	// CASO 1: ACHICAR O MANTENER
	if (bloquesActuales >= bloquesFinales) {
		// marcamos libres los bloques que le truncamos
		int bloquesDeAchicamiento = bloquesActuales - bloquesFinales;
		for(int i = 0; i < bloquesDeAchicamiento; i++) {
			bitarray_clean_bit(bitmap, entrada->bloqueInicial + bloquesFinales + i);
		}

		entrada->largo = tamaño; // actualizamos al nuevo tamaño en la tabla
		
		// persistimos a disco (o sea digamos, archivo de metadata)
		char *path = armarPathMetadata(entrada->nombre);
		t_config *metadata = config_create(path);

		char bufferString[100];
		sprintf(bufferString, "%d", tamaño);
		config_set_value(metadata, "TAMANIO_ARCHIVO", bufferString);
		config_save(metadata);

		config_destroy(metadata);
		free(path);
		return;
	}

	// CASOS 2: AGRANDAR

	// ME FIJO SI ME DA EL ESPACIO CONTIGUO

	// para facilitar la busqueda de hueco libre primero "me salgo" del bitmap y despues busco
	for (int i = 0; i < bloquesActuales; i++) {
		bitarray_clean_bit(bitmap, entrada->bloqueInicial + i);
	}

	int bloquesLibres = 0;
	int huecoLibreActual;
	int inicioHuecoLibre;
	bool elAnteriorFue0 = false;

	// recorremos el bitmap
	for (int i = 0; i < CANT_BLOQUES; i++) {
		// si me encuentro un bloque libre
		if (!bitarray_test_bit(bitmap, i)) {
			bloquesLibres++;

			if (elAnteriorFue0) {
				huecoLibreActual++;
			} else {
				huecoLibreActual = 1;
				inicioHuecoLibre = i;
			}

			// encontre un huequito (FIRST FIT)
			if (huecoLibreActual >= bloquesFinales) {
				// asigno los bloques
				for (int j = 0; j < bloquesFinales; j++) {
					bitarray_set_bit(bitmap, inicioHuecoLibre + j);
				}

				// copio los datos
				memcpy(BLOQUES + entrada->bloqueInicial * TAM_BLOQUE, BLOQUES + inicioHuecoLibre * TAM_BLOQUE, entrada->largo);

				// actualizo la gordita (aka FAT)
				entrada->bloqueInicial = inicioHuecoLibre;
				entrada->largo = tamaño; // actualizamos al nuevo tamaño en la tabla
	
				// actualizamos archivo de metadata
				char *path = armarPathMetadata(entrada->nombre);
				t_config *metadata = config_create(path);

				char bufferString1[100];
				sprintf(bufferString1, "%d", tamaño);
				config_set_value(metadata, "TAMANIO_ARCHIVO", bufferString1);

				char bufferString2[100];
				sprintf(bufferString2, "%d", inicioHuecoLibre);
				config_set_value(metadata, "BLOQUE_INICIAL", bufferString2);

				config_save(metadata);

				config_destroy(metadata);
				free(path);
				return;
			}

			elAnteriorFue0 = true;
		} else {
			elAnteriorFue0 = false;
		}
	}

	// si termino de recorrer el bitmap y sigo aca es pq no retorne, ergo, no encontre hueco libre contiguo

	// CASO 4: NO ME ENTRA
	if (bloquesFinales > bloquesActuales + bloquesLibres) {
		log_error("No se puede truncar a %d: no hay espacio", tamaño);
		return;
	}

	// si no me dio el espacio contigua pero SI me da el espacio total, necesito compactar
	// CASO 3: ME ENTRA PERO DEBO COMPACTAR


	// primero levanto al archivo que quiero truncar:
	// le desmarco los bloques
	for (int i = 0; i < bloquesActuales; i++) {
		bitarray_clean_bit(bitmap, entrada->bloqueInicial + i);
	}

	// me copio los datos a un buffer
    char* bufferDatos = malloc(entrada->largo);
    memcpy(bufferDatos, BLOQUES + entrada->bloqueInicial * TAM_BLOQUE, entrada->largo);

	compactar(); // como levante al archivo del bitmap, compactara a todos los demas

	// una vez que ya compacte todo el resto, pongo el archivo con su tamaño ampliado al final

	for (inicioHuecoLibre = 0; bitarray_test_bit(bitmap, inicioHuecoLibre); inicioHuecoLibre++) {
        ; // buscamos el inicio del espacio libre
    }

	// marco la nueva cantidad de bloques
	for (int i = 0; i < bloquesFinales; i++) {
		bitarray_set_bit(bitmap, inicioHuecoLibre + i);
	}

	// me copio los datos originales desde el buffer en el nuevo lugar
	memcpy(BLOQUES + inicioHuecoLibre * TAM_BLOQUE, bufferDatos, entrada->largo);

	free(bufferDatos); // el buffer cumplio su mision, lo dejamos descansar

	// actualizamos las estructuras administrativas

	// entrada FAT
	entrada->largo = tamaño;
	entrada->bloqueInicial = inicioHuecoLibre;

	// actualizamos archivo de metadata
	char *path = armarPathMetadata(entrada->nombre);
	t_config *metadata = config_create(path);

	char bufferString1[100];
	sprintf(bufferString1, "%d", tamaño);
	config_set_value(metadata, "TAMANIO_ARCHIVO", bufferString1);

	char bufferString2[100];
	sprintf(bufferString2, "%d", inicioHuecoLibre);
	config_set_value(metadata, "BLOQUE_INICIAL", bufferString2);

	config_save(metadata);

	config_destroy(metadata);
	free(path);
	return;
}

bool escribir_fs(char* nombreArchivo, char* cadena, int offset){
	printf("Escribiendo en archivo %s: %s\n", nombreArchivo, cadena);

	//Inner function para buscar la entrada del archivo en la fat
	bool esEntrada (void* elem) {
		entradaFat* entrada = (entradaFat*)elem;
		return !strcmp(entrada->nombre, nombreArchivo);
	}

	entradaFat* entrada = list_find(FAT, esEntrada);
	if(entrada==NULL){
		log_error(logger, "No se encontro archivo %s", nombreArchivo);
	}
	int bloqueBase = entrada->bloqueInicial;
	int largoArchivo = entrada->largo;

	//Si la cadena no entra en el archivo retorno error
	if (string_length(cadena) > largoArchivo-offset){
		return 0;
	}

	memcpy(BLOQUES+bloqueBase*TAM_BLOQUE+offset, cadena, string_length(cadena));
	return 1;
}

char* leer_fs(char *nombreArchivo, int offset, int cantALeer){
	//Inner function para buscar la entrada del archivo en la fat
	bool esEntrada (void* elem) {
		entradaFat* entrada = (entradaFat*)elem;
		return !strcmp(entrada->nombre, nombreArchivo);
	}

	entradaFat* entrada = list_find(FAT, esEntrada);
	int bloqueBase = entrada->bloqueInicial;
	int largoArchivo = entrada->largo;

	if (cantALeer > largoArchivo){
		log_error(logger, "Tamaño a leer mayor que tamaño de archivo, leemos archivo completo");
		cantALeer = largoArchivo;
	}

	//leo del archivo de bloques cantALeer bytes desde el offset dentro del bloque
	char* leidoDeArchivo = malloc(cantALeer);
	memcpy(leidoDeArchivo, BLOQUES+bloqueBase*TAM_BLOQUE+offset, cantALeer);

	log_info(logger, "Archivo %s leido", nombreArchivo);
	return leidoDeArchivo;  // devolvemos el puntero mallocado, se libera despues de ser usado por quien llama a la funcion
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
				return (entrada->bloqueInicial == inicioArchivo);
			}

			entrada = list_find(FAT, esEntrada);
			entrada->bloqueInicial = inicioHueco;
			largoArchivo = entrada->largo;
			bufferBloques = malloc(largoArchivo);

			//base en metadata = inicioHueco
			path = armarPathMetadata(entrada->nombre);
			metadata = config_create(path);

			sprintf(bufferString, "%d", inicioHueco); // convertimos a string el numero 

			config_set_value(metadata, "BLOQUE_INICIAL", bufferString);
			config_save(metadata); // guardamos en archivo metadata

			float largoArchivoFloat = largoArchivo; // transformar a float para poder dividir y meter ceil
			int cantBloques = ceil(largoArchivoFloat/TAM_BLOQUE); // calculamos cantBloques
			
			//limpio cantBloques bits desde inicioArchivo en bitmap
			for(int i = 0; i < cantBloques; i++)
				bitarray_clean_bit(bitmap, inicioArchivo + i);
			
			//seteo cantBloques bits desde inicioHueco en bitmap
			for(int i = 0; i < cantBloques; i++)
				bitarray_set_bit(bitmap, inicioHueco + i);

			//copio a buffer largo bytes desde inicioArchivo de bloques.dat
			memcpy(bufferBloques, BLOQUES + inicioArchivo*TAM_BLOQUE, largoArchivo);

			//copio de buffer largo bytes desde inicioHueco a bloques.dat
			memcpy(BLOQUES + inicioHueco*TAM_BLOQUE, bufferBloques, largoArchivo);

			inicioHueco += largoArchivo;
			inicioArchivo = inicioHueco;

			log_info(logger, "Compactando ando");

			config_destroy(metadata);
			free(bufferBloques);
			free(path);
		}
	}
}

void iniciar_fs(){

	//TODO chequear si existe carpeta dialfs.
	//si existe: no creamos los archivos, los leemos para recuperar la informacion (tablaFAT y bitmap y mapeo de memoria de bloques)
	//si no: se crean los archivos

	//logs obligatorios

	// Sleeps en las interfaces

	//rutinas de liberacion de recursos de todas las interfaces

	//VARIABLES DEL CONFIG
	CANT_BLOQUES = config_get_int_value(config, "BLOCK_COUNT");
	TAM_BLOQUE = config_get_int_value(config, "BLOCK_SIZE");
	DIR_BASE = config_get_string_value(config, "PATH_BASE_DIALFS");

	FILE *archivo_bitmap;
	FILE *archivo_bloques;


	//BITMAP
	char* path = malloc(strlen(DIR_BASE) + strlen("bitmap.dat") + 2);

	mkdir(DIR_BASE, 0777);
	strcpy(path, DIR_BASE);
	strcat(path, "/");
	strcat(path, "bitmap.dat");

	bool existeBitmap = access(path, F_OK) != -1;

	// Chequeo existencia del archivo de bitmap
	if (!existeBitmap){

		// Si no existe, lo creo y le doy el tamaño
		archivo_bitmap = fopen(path, "w+b");
		truncate(path, CANT_BLOQUES/8);	// un bit por bloque

	} else{

		// Si existe, lo abro
		archivo_bitmap = fopen(path, "r+b");

	}

    bitmap = bitarray_create_with_mode(mmap(NULL , CANT_BLOQUES/8, PROT_WRITE | PROT_READ, MAP_SHARED, archivo_bitmap->_fileno, 0), CANT_BLOQUES/8, MSB_FIRST);
	
	// Si no existia lo inicializo en 0
	if (!existeBitmap){
		for(int i = 0; i < CANT_BLOQUES; i++){
			bitarray_clean_bit(bitmap, i);
		}
	}

	free(path);

	fclose(archivo_bitmap);


	//Inicializo tabla FAT
	FAT = list_create();


	// METADATA
	DIR_METADATA = malloc(strlen(DIR_BASE) + strlen("metadata") + 2);

	strcpy(DIR_METADATA, DIR_BASE);
	strcat(DIR_METADATA, "/metadata");

	// Chequeo existencia del directorio de metadata
	if (access(DIR_METADATA, F_OK) == -1){

		// Si no existe, lo creo
		mkdir(DIR_METADATA, 0777);

	} else{

		// Si existe, armo la FAT recorriendo los archivos de la carpeta metadata
		DIR* directorio = opendir(DIR_METADATA);
		struct dirent *subdirectorio;

		while ((subdirectorio = readdir(directorio)) != NULL) {

			char* nombreArchivo = malloc(256);
			strcpy (nombreArchivo, subdirectorio->d_name);

			// No mapea si es cualquiera de estos 4 archivos
			if (!strcmp(nombreArchivo, ".") || !strcmp(nombreArchivo, "..") || !strcmp(nombreArchivo, "bloques.dat") || !strcmp(nombreArchivo, "bitmap.dat")){
				free (nombreArchivo);
				continue;
			}

			char* path = armarPathMetadata(nombreArchivo);

			t_config* metadata = config_create(path);

			//Agregamos su entrada en la tabla fat
			entradaFat* entrada = malloc(sizeof(entradaFat));
			entrada->nombre = nombreArchivo;
			entrada->bloqueInicial = config_get_int_value(metadata, "BLOQUE_INICIAL");
			entrada->largo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");
			list_add(FAT, entrada);

			free (path);
			config_destroy (metadata);

		}
  
    	closedir(directorio); 
		

	}
	

	//ARCHIVO DE BLOQUES
	path = malloc(strlen(DIR_BASE) + strlen("bloques.dat") + 2);
	strcpy(path, DIR_BASE);
	strcat(path, "/");
	strcat(path, "bloques.dat");

	// Chequeo existencia del archivo de bloques
	if (access(path, F_OK) == -1){

		// Si no existe, lo creo y le doy el tamaño
		archivo_bloques = fopen(path, "w+b");
		truncate(path, CANT_BLOQUES*TAM_BLOQUE);

	} else{

		// Si existe, lo abro
		archivo_bloques = fopen(path, "r+b");

	}

	BLOQUES = mmap(NULL , CANT_BLOQUES*TAM_BLOQUE, PROT_WRITE | PROT_READ, MAP_SHARED, archivo_bloques->_fileno, 0);


	fclose(archivo_bloques);
	free(path);
}

char* armarPathMetadata (char* nombre){
	char* path = malloc(strlen(DIR_METADATA) + strlen(nombre) + 2);
	strcpy(path, DIR_METADATA);
	strcat(path, "/");
	strcat(path, nombre);
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