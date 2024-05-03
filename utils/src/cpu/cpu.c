#include "cpu.h"

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
    return readline(">");
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
            set(instruccion.componentes[1], instruccion.componentes[2]);
            break;
        case SUM:
            sum(instruccion.componentes[1], instruccion.componentes[2]);
            break;
			}
}


int get_cod_instruccion(char* instruccion){
     if(!strcmp(instruccion, "SUM"))
        return SUM;
     else if(!strcmp(instruccion, "SET"))  //FALTAN CASI TODOS 
        return SET;
     else 
    return -1;
}

//INSTRUCCIONES
void set (char* registro, char* valor){
  int x = atoi(valor);
  set_registro(registro, x);
}

void sum (char* reg1, char* reg2){
    int x=get_registro(reg1);
    int y=get_registro(reg2);
    x=x+y;
    set_registro(reg1, x);
}


//LOGS OBLIGATORIOS
void log_execute (int pid, char* instruccion, char* parametros){
    log_info(logger, "PID: %d - Ejecutando: %s - %s", pid, instruccion, parametros);
}