#include "cpu.h"

void finalizar_cpu(){
    printf("Kernel desconectado, finalizando CPU \n" );
    liberar_conexion(memoria);
    log_destroy(logger);
    config_destroy(config);
    pthread_mutex_destroy(&mIntr);
    queue_destroy(cIntr);
    free(aEnviar);
    //free(vectorDirecciones);
	exit(0);
}

int MMU(int DL){   
    int pag = DL/tam_pag;
    int desplazamiento = DL - pag*tam_pag;
    int DF;
    int marco;

    if(desplazamiento>tam_pag)
        return -1;

    if(!cant_entradas_TLB){
        enviar_int(pag, memoria, PAGINA);

        if(recibir_operacion(memoria)!=PAGINA){
            printf("La memoria me envió cualquier cosa... \n");
            return -1;
        }

        marco = recibir_int(memoria);
        DF = marco*tam_pag + desplazamiento;
        log_marco(pcb.pid, pag, marco);

        return DF;
    }else{

        bool buscar_pagina_TLB(void* elem){
            t_entradaTLB *una_entrada_TLB = (t_entradaTLB*) elem;
            return (una_entrada_TLB->pid == pcb.pid && una_entrada_TLB->pagina == pag);
        }

        entrada_TLB = list_find(tlb->elements, buscar_pagina_TLB);
        
        if(entrada_TLB){
            log_tlb_hit(pcb.pid, pag);

            if(!strcmp(algoritmo_TLB, "LRU")){
                list_remove_element(tlb->elements, entrada_TLB);
                queue_push(tlb, entrada_TLB);
            }

            DF = entrada_TLB->marco * tam_pag + desplazamiento;

            return DF;
        }else{
            log_tlb_miss(pcb.pid, pag);

            entrada_TLB = malloc(sizeof *entrada_TLB);

            enviar_int(pag, memoria, PAGINA);

            if(recibir_operacion(memoria)!=PAGINA){
                printf("La memoria me envió cualquier cosa... \n");
                return -1;
            }

            marco = recibir_int(memoria);
            DF = marco*tam_pag + desplazamiento;
            log_marco(pcb.pid, pag, marco);

            entrada_TLB->pid = pcb.pid;
            entrada_TLB->pagina = pag;
            entrada_TLB->marco = marco;

            acomodar_entradas_TLB(entrada_TLB);

            return DF;
        }
    }
}

void acomodar_entradas_TLB(t_entradaTLB *una_entrada_TLB){
    t_entradaTLB *entrada_aux;

    if(entradas_actuales_tlb){
        entradas_actuales_tlb--;
        queue_push(tlb, una_entrada_TLB);
    }else{
        free(queue_pop(tlb));
        queue_push(tlb, una_entrada_TLB);
    }

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
            printf("Se desconectaron las interrupciones \n");
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
        printf("La memoria me envío cualquier cosa... \n");
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
            exe_IO_GEN(instruccion.componentes);
            break;
        case IO_STDOUT_WRITE:
        case IO_STDIN_READ:
            exe_IO_STD(instruccion.componentes);
            break;
        case IO_FS_CREATE:
        case IO_FS_DELETE:
            exe_IO_FS_CD(instruccion.componentes);
            break;
        case IO_FS_TRUNCATE:
            exe_IO_FS_TRUNCATE(instruccion.componentes);
            break;
        case IO_FS_WRITE:
        case IO_FS_READ:
            exe_IO_FS_RW(instruccion.componentes);
            break;
        case EXIT:
            exe_EXIT();
            break;
        case MOV_IN:
            exe_MOV_IN(instruccion.componentes[1], instruccion.componentes[2]);
            break;
        case MOV_OUT:
            exe_MOV_OUT(instruccion.componentes[1], instruccion.componentes[2]);
            break;
        case RESIZE:
            exe_RESIZE(atoi(instruccion.componentes[1]));
            break;
        case COPY_STRING:
            exe_COPY_STRING(atoi(instruccion.componentes[1]));
            break;
        default:
            seVa=500;
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
    else if (!strcmp(instruccion, "IO_FS_CREATE"))
        return IO_FS_CREATE;
    else if (!strcmp(instruccion, "IO_FS_DELETE"))
        return IO_FS_DELETE;
    else if (!strcmp(instruccion, "IO_FS_TRUNCATE"))
        return IO_FS_TRUNCATE;
    else if (!strcmp(instruccion, "IO_FS_WRITE"))
        return IO_FS_WRITE;
    else if (!strcmp(instruccion, "IO_FS_READ"))
        return IO_FS_READ;
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
    seVa=PEDIRRECURSO;
}

void exe_SIGNAL(char* recurso){
    strcpy (aEnviar, recurso);
    seVa=DARRECURSO;
}

void exe_IO_GEN (char** componentes){
    strcpy (aEnviar, componentes[0]);

    int i=1;
    while(componentes[i]) {
        strcat (aEnviar, " ");
        strcat (aEnviar, componentes[i]);
        i++;
    }
    seVa=IO;
}

void exe_IO_STD(char** componentes){
    int direccion = get_registro(componentes[2]);
    sprintf(componentes[2], "%i", direccion);

    int tamaño = get_registro(componentes[3]);
    float tam = tamaño;
    int desplazamiento = direccion%tam_pag;
    int espacioEnPag = tam_pag-(desplazamiento);
    if(tamaño<=espacioEnPag){
        int* vDirecciones = calloc(1, sizeof(int));
        vDirecciones[0] = MMU(direccion);
        vectorDirecciones = vDirecciones;
        tamañoVector = 1;
    }
    else{
        int i;
        int pagNecesarias = ceil((tam-espacioEnPag)/tam_pag)+1;
        int* vDirecciones = calloc(pagNecesarias, sizeof(int));
        vDirecciones[0]=MMU(direccion);
        for(i=0; i<pagNecesarias-1; i++)
            vDirecciones[i+1]=MMU(direccion+espacioEnPag+i*tam_pag);
        vectorDirecciones = vDirecciones;
        tamañoVector = pagNecesarias;
    }
    strcpy (aEnviar, componentes[0]);

    strcat (aEnviar, " ");
    strcat (aEnviar, componentes[1]);
    strcat(aEnviar, " ");
    string_append_with_format(&aEnviar, "%d %d", tamaño, tamañoVector);
    seVa=IO_VECTOR;
}

void exe_IO_FS_CD(char** componentes){
    strcpy (aEnviar, componentes[0]);
    string_append_with_format(&aEnviar, " %s %s", componentes[1], componentes[2]);
    seVa=IO;
}

void exe_IO_FS_TRUNCATE(char** componentes){
    strcpy (aEnviar, componentes[0]);
    string_append_with_format(&aEnviar, " %s %s %d", componentes[1], componentes[2], get_registro(componentes[3]));
    seVa=IO;
}

void exe_IO_FS_RW(char** componentes){
    int direccion = get_registro(componentes[3]);

    int tamaño = get_registro(componentes[4]);
    float tam = tamaño;
    int desplazamiento = direccion%tam_pag;
    int espacioEnPag = tam_pag-(desplazamiento);
    if(tamaño<=espacioEnPag){
        int* vDirecciones = calloc(1, sizeof(int));
        vDirecciones[0] = MMU(direccion);
        vectorDirecciones = vDirecciones;
        tamañoVector = 1;
    }
    else{
        int i;
        int pagNecesarias = ceil((tam-espacioEnPag)/tam_pag)+1;
        int* vDirecciones = calloc(pagNecesarias, sizeof(int));
        vDirecciones[0]=MMU(direccion);
        for(i=0; i<pagNecesarias-1; i++)
            vDirecciones[i+1]=MMU(direccion+espacioEnPag+i*tam_pag);
        vectorDirecciones = vDirecciones;
        tamañoVector = pagNecesarias;
    }
    strcpy (aEnviar, componentes[0]);
    string_append_with_format(&aEnviar, " %s %s %d %d %d", componentes[1], componentes[2], tamaño, tamañoVector, get_registro(componentes[5]));
    seVa=IO_VECTOR;
}

void exe_MOV_IN(char* reg_datos, char* reg_direccion){
    int tamaño=cuanto_leo(reg_datos);
    float tam = tamaño;
    int DF, size;
    int direccion = get_registro(reg_direccion);
    int desplazamiento = direccion%tam_pag;
    int espacioEnPag = tam_pag-desplazamiento;
    if(tamaño==1 || tamaño<=espacioEnPag){
        DF = MMU(direccion);
        enviar_operacion(memoria, LECTURA);
        enviar_operacion(memoria, DF);
        enviar_operacion(memoria, tamaño);
        enviar_operacion(memoria, 1);
    }
    else{        
        int i;
        int pagNecesarias = ceil((tam-espacioEnPag)/tam_pag)+1;
        int vDirecciones[pagNecesarias];
        vDirecciones[0]=MMU(direccion);
        DF=vDirecciones[0];
        for(i=0; i<pagNecesarias-1; i++)
           vDirecciones[i+1]=MMU(direccion+espacioEnPag+i*tam_pag);
        enviar_operacion(memoria, LECTURA);
        enviar_operacion(memoria, vDirecciones[0]);
        enviar_operacion(memoria, tamaño);
        enviar_operacion(memoria, pagNecesarias);
        for(i=1; i<pagNecesarias; i++)
            enviar_operacion(memoria, vDirecciones[i]);
    }
    int valor = recibir_operacion(memoria);
    log_rw(pcb.pid, "LEER", DF, valor);
    set_registro(reg_datos, valor);
}

void exe_MOV_OUT(char* reg_direccion, char* reg_datos){

    int tamaño=cuanto_leo(reg_datos);
    float tam = tamaño;
    int DF, size;
    int direccion = get_registro(reg_direccion);
    int desplazamiento = direccion%tam_pag;
    int espacioEnPag = tam_pag-desplazamiento;
    if(tamaño==1 || tamaño<=espacioEnPag){
        DF = MMU(direccion);
        enviar_operacion(memoria, ESCRITURA);
        enviar_operacion(memoria, DF);
        enviar_operacion(memoria, tamaño);
        enviar_operacion(memoria, 1);
        enviar_operacion(memoria, get_registro(reg_datos));
    }
    else{        
        int i;
        int pagNecesarias = ceil((tam-espacioEnPag)/tam_pag)+1;
        int vDirecciones[pagNecesarias];
        vDirecciones[0]=MMU(direccion);
        DF=vDirecciones[0];
        for(i=0; i<pagNecesarias-1; i++)
           vDirecciones[i+1]=MMU(direccion+espacioEnPag+i*tam_pag);
        enviar_operacion(memoria, ESCRITURA);
        enviar_operacion(memoria, vDirecciones[0]);
        enviar_operacion(memoria, tamaño);
        enviar_operacion(memoria, pagNecesarias);
        enviar_operacion(memoria, get_registro(reg_datos));
        for(i=1; i<pagNecesarias; i++)
            enviar_operacion(memoria, vDirecciones[i]);
    }
  log_rw(pcb.pid, "ESCRIBIR", DF, get_registro(reg_datos));
}

void exe_RESIZE(int tamanio){
    enviar_operacion(memoria, TAM_PROCESO);
    float tamanioActual = recibir_operacion(memoria);

    if(tamanioActual == tamanio){
        return;
    } else if(tamanioActual > tamanio){
        enviar_int(ceil((tamanioActual-tamanio)/tam_pag),memoria, MENOS_PAGINA);
    }
    else{
        enviar_int(ceil((tamanio-tamanioActual)/tam_pag),memoria, MAS_PAGINA);
        if(recibir_operacion(memoria) == OOM)
            seVa=OOM;
    }
}

void exe_COPY_STRING(int tamaño){
    float tam = tamaño;
    int DF, size;
    int direccion = get_registro("SI");
    int desplazamiento = direccion%tam_pag;
    int espacioEnPag = tam_pag-desplazamiento;
    if(tamaño<=espacioEnPag){
        DF = MMU(direccion);
        enviar_operacion(memoria, LECTURA_STRING);
        enviar_operacion(memoria, 1);
        enviar_operacion(memoria, tamaño);
        enviar_operacion(memoria, DF);
    }
    else{
        int i;
        int pagNecesarias = ceil((tam-espacioEnPag)/tam_pag)+1;
        int vDirecciones[pagNecesarias];
        vDirecciones[0]=MMU(direccion);
        DF=vDirecciones[0];
        for(i=0; i<pagNecesarias-1; i++)
            vDirecciones[i+1]=MMU(direccion+espacioEnPag+i*tam_pag);
        enviar_operacion(memoria, LECTURA_STRING);
        enviar_operacion(memoria, pagNecesarias);
        enviar_operacion(memoria, espacioEnPag);
        enviar_operacion(memoria, vDirecciones[0]);
        for(i=1; i<pagNecesarias-1; i++){
            enviar_operacion(memoria, tam_pag);
            enviar_operacion(memoria, vDirecciones[i]);
        }
        if((tamaño-espacioEnPag)%tam_pag){
             enviar_operacion(memoria, (tamaño-espacioEnPag)%tam_pag);
            enviar_operacion(memoria, vDirecciones[i]);
        }
        else{          
            enviar_operacion(memoria, tam_pag);
            enviar_operacion(memoria, vDirecciones[i]);
        }
    }
    recibir_operacion(memoria);
    char* cadena = recibir_buffer(&size,memoria);
    log_rws(pcb.pid, "LEER", DF, cadena);
    direccion = get_registro("DI");

    desplazamiento = direccion%tam_pag;
    espacioEnPag = tam_pag-(desplazamiento);
    if(tamaño<=espacioEnPag){
        DF = MMU(direccion);
        enviar_string(cadena, memoria, ESCRITURA_STRING);
        enviar_operacion(memoria, 1);
        enviar_operacion(memoria, tamaño);
        enviar_operacion(memoria, DF);
    }
    else{
        int i;
        int pagNecesarias = ceil((tam-espacioEnPag)/tam_pag)+1;
        int vDirecciones[pagNecesarias];
        vDirecciones[0]=MMU(direccion);
        DF=vDirecciones[0];
        for(i=0; i<pagNecesarias-1; i++)
            vDirecciones[i+1]=MMU(direccion+espacioEnPag+i*tam_pag);
        enviar_string(cadena, memoria, ESCRITURA_STRING);
        enviar_operacion(memoria, pagNecesarias);
        enviar_operacion(memoria, espacioEnPag);
        enviar_operacion(memoria, vDirecciones[0]);
        for(i=1; i<pagNecesarias-1; i++){
            enviar_operacion(memoria, tam_pag);
            enviar_operacion(memoria, vDirecciones[i]);
        }
        if((tamaño-espacioEnPag)%tam_pag){
            enviar_operacion(memoria, (tamaño-espacioEnPag)%tam_pag);
            enviar_operacion(memoria, vDirecciones[i]);
        }
        else{          
            enviar_operacion(memoria, tam_pag);
            enviar_operacion(memoria, vDirecciones[i]);
        }
    }
    log_rws(pcb.pid, "ESCRIBIR", DF, cadena);
    free (cadena);
}
//LOGS OBLIGATORIOS
void log_fetch (int pid, int pc){
    log_info(logger, "PID: %d - FETCH - Program Counter: %d", pid, pc);
}

void log_execute (int pid, char* instruccion, char* parametros){
    log_info(logger, "PID: %d - Ejecutando: %s - %s", pid, instruccion, parametros);
}

void log_marco(int pid, int pag, int marco){
    log_info(logger, "PID: %d - Página: %d - Marco: %d", pid, pag, marco);
}

void log_rw(int pid, char* operacion, int direccion, int valor){
    log_info(logger, "PID: %d - Acción: %s - Dirección Física: %d - Valor: %d", pid, operacion, direccion, valor);
}

void log_rws(int pid, char* operacion, int direccion, char* valor){
    log_info(logger, "PID: %d - Acción: %s - Dirección Física: %d - Valor: %s", pid, operacion, direccion, valor);
}

void log_tlb_hit(int pid, int pagina){
    log_info(logger, "PID: %d - TLB HIT - Página: %d", pid, pagina);
}

void log_tlb_miss(int pid, int pagina){
    log_info(logger, "PID: %d - TLB MISS - Página: %d", pid, pagina);
}