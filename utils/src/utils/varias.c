#include "varias.h"
char* buscar(t_log* logger, t_config* config, char* clave){
    if(config_has_property(config, clave))
		return config_get_string_value(config, clave);
	else
	    log_error(logger, "No se ha encontrado %s", clave);
}