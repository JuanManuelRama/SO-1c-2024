#include "cpu.h"


void finalizar_cpu(){
    log_info(logger, "Kernel desconectado, finalizando CPU");
    liberar_conexion(memoria);
    log_destroy(logger);
    config_destroy(config);
    pthread_mutex_destroy(&mCdI);
	exit(0);
}

void interrupciones(int socket_interrupciones){
    int cod_op;
    while(1){
       cod_op = recibir_operacion(socket_interrupciones);
       pthread_mutex_lock(&mCdI);
        switch (cod_op){
            case FINALIZACION:
                if(recibir_int(socket_interrupciones)==pcb.pid)
                    seVa=FINALIZACION;
                break;
            case QUANTUM:
                if(recibir_int(socket_interrupciones)==pcb.pid && seVa==false)
                    seVa=QUANTUM;
                break;
            case -1:
                log_error(logger, "el cliente se desconecto");
                pthread_mutex_unlock(&mCdI);
                return;
            default:
                log_warning(logger, "Operacion no esperada por parte de este cliente");
                break;
        }
        pthread_mutex_unlock(&mCdI);
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
        case IO_GEN_SLEEP: 
            // todos los cases de los tipos de interfaces IO
            exe_IO(instruccion.componentes);
            break;
        case EXIT:
            exe_EXIT();
            break;

	}
}


int get_cod_instruccion(char* instruccion){
    if(!strcmp(instruccion, "SUM"))
        return SUM;
    else if(!strcmp(instruccion, "SET"))  //FALTAN CASI TODOS 
        return SET;
    else if (!strcmp(instruccion, "SUB"))
        return SUB;
    else if (!strcmp(instruccion, "JNZ"))
        return JNZ;
    else if (!strcmp(instruccion, "EXIT"))
        return EXIT;
    else if (!strcmp(instruccion, "IO_GEN_SLEEP"))
        return IO_GEN_SLEEP;
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

//LOGS OBLIGATORIOS
void log_fetch (int pid, int pc){
    log_info(logger, "PID: %d - FETCH - Program Counter: %d", pid, pc);
}

void log_execute (int pid, char* instruccion, char* parametros){
    log_info(logger, "PID: %d - Ejecutando: %s - %s", pid, instruccion, parametros);
}