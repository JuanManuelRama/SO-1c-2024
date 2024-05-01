#include "cpu.h"

int set_registro(t_pcb pcb, char* registro, int valor) {
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

int get_registro(t_pcb pcb, char* registro) {
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