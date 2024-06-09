#include "cpu.h"


void finalizar_cpu(){
    log_info(logger, "Kernel desconectado, finalizando CPU");
    liberar_conexion(memoria);
    log_destroy(logger);
    config_destroy(config);
    pthread_mutex_destroy(&mIntr);
    queue_destroy(cIntr);
    free(aEnviar);
	exit(0);
}

int MMU(int DL){
    int pag = DL/tam_pag;
    int desplazamiento = DL - pag*tam_pag;
    if(desplazamiento>tam_pag)
        return -1;
    enviar_int(pag, memoria, PAGINA);
    if(recibir_operacion(memoria)!=PAGINA)
        log_error(logger, "La memoria me envió cualquier cosa...");
    int marco = recibir_int(memoria);
    int DF = marco*tam_pag + desplazamiento;
    return DF;
}

int cuanto_leo(char* registro){
     if(!strcmp(registro, "AX")){
        return 1;
    } else if(!strcmp(registro, "BX")){
        return 1;
    } else if(!strcmp(registro, "CX")){
        return 1;
    } else if(!strcmp(registro, "DX")){
        return 1;
    } else if(!strcmp(registro, "EAX")){
        return 4;
    } else if(!strcmp(registro, "EBX")){
        return 4;
    } else if(!strcmp(registro, "ECX")){
        return 4;
    } else if(!strcmp(registro, "EDX")){
        return 4;
    }else{
        return -1;
    }
}


void interrupciones(int socket_interrupciones){
    int cod_op;
    sInterrupcion* interrupcion;
    while(1){
       cod_op = recibir_operacion(socket_interrupciones);       
        if(cod_op == -1){
            log_error(logger, "Se desconectaron las interrupciones");
            return;
        }
        interrupcion = malloc(sizeof(sInterrupcion));
        interrupcion->motivo = cod_op;
        interrupcion->pid = recibir_int(socket_interrupciones);
        pthread_mutex_lock(&mIntr);
        queue_push(cIntr, interrupcion);
        pthread_mutex_unlock(&mIntr);
    }
}


int set_registro(char* registro, int valor) {
    if(!strcmp(registro, "PC")){
        pcb.registros.PC = valor;
        return pcb.registros.PC;
    } else if(!strcmp(registro, "AX")){
        pcb.registros.AX = valor;
        return pcb.registros.AX;
    } else if(!strcmp(registro, "BX")){
        pcb.registros.BX = valor;
        return pcb.registros.BX;
    } else if(!strcmp(registro, "CX")){
        pcb.registros.CX = valor;
        return pcb.registros.CX;
    } else if(!strcmp(registro, "DX")){
        pcb.registros.DX = valor;
        return pcb.registros.DX;
    } else if(!strcmp(registro, "EAX")){
        pcb.registros.EAX = valor;
        return pcb.registros.EAX;
    } else if(!strcmp(registro, "EBX")){
        pcb.registros.EBX = valor;
        return pcb.registros.EBX;
    } else if(!strcmp(registro, "ECX")){
        pcb.registros.ECX = valor;
        return pcb.registros.ECX;
    } else if(!strcmp(registro, "EDX")){
        pcb.registros.EDX = valor;
        return pcb.registros.EDX;
    } else if(!strcmp(registro, "SI")){
        pcb.registros.SI = valor;
        return pcb.registros.SI;
    } else if(!strcmp(registro, "DI")){
        pcb.registros.DI = valor;
        return pcb.registros.DI;
    }else{
        return -1;
    }
}

int get_registro(char* registro) {
    if(!strcmp(registro, "PC")){
        return pcb.registros.PC;
    } else if(!strcmp(registro, "AX")){
        return pcb.registros.AX;
    } else if(!strcmp(registro, "BX")){
        return pcb.registros.BX;
    } else if(!strcmp(registro, "CX")){
        return pcb.registros.CX;
    } else if(!strcmp(registro, "DX")){
        return pcb.registros.DX;
    } else if(!strcmp(registro, "EAX")){
        return pcb.registros.EAX;
    } else if(!strcmp(registro, "EBX")){
        return pcb.registros.EBX;
    } else if(!strcmp(registro, "ECX")){
        return pcb.registros.ECX;
    } else if(!strcmp(registro, "EDX")){
        return pcb.registros.EDX;
    } else if(!strcmp(registro, "SI")){
        return pcb.registros.SI;
    } else if(!strcmp(registro, "DI")){
        return pcb.registros.DI;
    }else{
        return -1;
    }
}

char* fetch(){
    enviar_int(pcb.registros.PC, memoria, FETCH);
    int size;
    log_fetch(pcb.pid, pcb.registros.PC);
    if(recibir_operacion(memoria)!=FETCH)
        log_error(logger, "La memoria me envío cualquier cosa...");
    return recibir_buffer(&size, memoria);
}

sInstruccion decode(char* buffer){
	sInstruccion instruccion;
	char** array;
	array = string_split (buffer, " ");
    log_execute(pcb.pid, array[0], buffer+strlen(array[0]));
    //Log obligatorio
	instruccion.cod_instruccion = get_cod_instruccion(array[0]);
	instruccion.componentes=array;
	return instruccion;
}

void execute(sInstruccion instruccion){
	switch(instruccion.cod_instruccion){
		case SET:
            exe_SET(instruccion.componentes[1], instruccion.componentes[2]);
            break;
        case SUM:
            exe_SUM(instruccion.componentes[1], instruccion.componentes[2]);
            break;
        case SUB:
            exe_SUB(instruccion.componentes[1], instruccion.componentes[2]);
            break;
        case JNZ:
            exe_JNZ(instruccion.componentes[1], instruccion.componentes[2]);
            break;
        case WAIT:
            exe_WAIT(instruccion.componentes[1]);
            break;
        case SIGNAL:
            exe_SIGNAL(instruccion.componentes[1]);
            break;
        case IO_GEN_SLEEP: 
        case IO_STDIN_READ:
        case IO_STDOUT_WRITE:
            // todos los cases de los tipos de interfaces IO
            exe_IO(instruccion.componentes);
            break;
        case EXIT:
            exe_EXIT();
            break;
        case MOV_IN:
            exe_MOVE_IN(instruccion.componentes[1], instruccion.componentes[2]);
            break;
        case MOV_OUT:
            exe_MOVE_OUT(instruccion.componentes[1], instruccion.componentes[2]);
            break;
        case RESIZE:
            exe_RESIZE(atoi(instruccion.componentes[1]));
            break;
        case COPY_STRING:
            exe_COPY_STRING(atoi(instruccion.componentes[1]));
            break;
	}
}


int get_cod_instruccion(char* instruccion){
    if(!strcmp(instruccion, "SUM"))
        return SUM;
    else if(!strcmp(instruccion, "SET"))
        return SET;
    else if (!strcmp(instruccion, "SUB"))
        return SUB;
    else if (!strcmp(instruccion, "JNZ"))
        return JNZ;
    else if (!strcmp(instruccion, "EXIT"))
        return EXIT;
    else if (!strcmp(instruccion, "IO_GEN_SLEEP"))
        return IO_GEN_SLEEP;
    else if (!strcmp(instruccion, "IO_STDIN_READ"))
        return IO_STDIN_READ;
    else if (!strcmp(instruccion, "IO_STDOUT_WRITE"))
        return IO_STDOUT_WRITE;
    else if (!strcmp(instruccion, "WAIT"))
        return WAIT;
    else if (!strcmp(instruccion, "SIGNAL"))
        return SIGNAL;
    else if (!strcmp(instruccion,"MOV_IN"))
        return MOV_IN;
    else if (!strcmp(instruccion,"MOV_OUT"))
        return MOV_OUT;
    else if (!strcmp(instruccion,"RESIZE"))
        return RESIZE;
    else if (!strcmp(instruccion,"COPY_STRING"))
        return COPY_STRING;
    return -1;
}

//INSTRUCCIONES
void exe_SET(char* registro, char* valor){
  set_registro(registro, atoi(valor));
}

void exe_SUM(char* reg_destino, char* reg_origen){
    int valor_destino = get_registro(reg_destino);
    int valor_origen = get_registro(reg_origen);
    valor_destino = valor_destino + valor_origen;
    set_registro(reg_destino, valor_destino);
}

void exe_SUB(char* reg_destino, char* reg_origen) {
    int valor_destino = get_registro(reg_destino);
    int valor_origen = -get_registro(reg_origen);
    valor_destino = valor_destino - valor_origen;
    set_registro(reg_destino, valor_destino);
}

void exe_JNZ(char* registro, char* numero_instruccion) {
    if ((get_registro(registro)))
        set_registro("PC", atoi(numero_instruccion)-1);
}
void exe_EXIT(){
    seVa=FINALIZACION;
}

void exe_WAIT(char* recurso){
    strcpy (aEnviar, recurso);
    seVa=WAIT;
}

void exe_SIGNAL(char* recurso){
    strcpy (aEnviar, recurso);
    seVa=SIGNAL;
}

void exe_IO (char** componentes){
    strcpy (aEnviar, componentes[0]);

    int i=1;
    while(componentes[i]) {
        strcat (aEnviar, " ");
        strcat (aEnviar, componentes[i]);
        i++;
    }
    seVa=IO;
}

void exe_MOVE_IN(char* reg_datos, char* reg_direccion){
    int DF = MMU(get_registro(reg_direccion));
    int leer = cuanto_leo(reg_datos);

    if(DF == -1){
        seVa=SEG_FAULT;
        return 0;
    }

    enviar_int(DF, memoria, LECTURA);
    enviar_operacion(leer, memoria);

    if(leer == 4){
        set_registro(reg_datos,recibir_int(memoria));
    }
    else{
        set_registro(reg_datos,recibir_byte(memoria));
    }
}

void exe_MOVE_OUT(char* reg_direccion, char* reg_datos){
    int DF = MMU(get_registro(reg_direccion));
    int escribir = cuanto_leo(reg_datos);
    int valor = get_registro(reg_datos);

    if(DF == -1){
        seVa=SEG_FAULT;
        return 0;
    }

    enviar_int(DF, memoria, ESCRITURA);
    enviar_operacion(escribir, memoria);
    enviar_operacion(valor,memoria);
}

void exe_RESIZE(int tamanio){
    enviar_operacion(TAM_PROCESO,memoria);
    int tamanioActual = recibir_operacion(memoria);

    if(tamanioActual == tamanio){
        return;
    } else if(tamanioActual > tamanio){
        enviar_int((tamanioActual-tamanio)/tam_pag,MENOS_PAGINA,memoria);
    }
    else{
        enviar_int((tamanio-tamanioActual)/tam_pag,MAS_PAGINA,memoria);
    }
}

void exe_COPY_STRING(int tamanio){
    int DF = MMU(pcb.registros.SI);
    int size;

    enviar_int(DF, memoria, LECTURA_STRING);
    char* cadenaCompleta = recibir_buffer(&size,memoria);

    DF = MMU(pcb.registros.DI);
   
    char* cadena = malloc(tamanio+1);
    strncpy(cadena,cadenaCompleta,tamanio);
    enviar_string(cadena,memoria,ESCRITURA_STRING);
    enviar_operacion(DF,memoria);

    free (cadenaCompleta);
    free (cadena);
}

//LOGS OBLIGATORIOS
void log_fetch (int pid, int pc){
    log_info(logger, "PID: %d - FETCH - Program Counter: %d", pid, pc);
}

void log_execute (int pid, char* instruccion, char* parametros){
    log_info(logger, "PID: %d - Ejecutando: %s - %s", pid, instruccion, parametros);
}