#include <commons/config.h>
#include <commons/log.h>
#include <commons/config.h>

/**
	* @fn     buscar
	* @brief  Busca en config, loggea error de no encontrarlo
    * @param    logger loggea el error
    * @param    config 
    * @param    key Palabra a buscar
    * @return Resultado de la búsqueda
	*/
char* buscar (t_log*, t_config*, char*);