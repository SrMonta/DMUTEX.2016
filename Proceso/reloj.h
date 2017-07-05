#ifndef COMMANDS_H_
#define COMMANDS_H_

#include <stdio.h>

/*Estructura para almacenar la lista de procesos en una variable que podamos localizar facilmente.*/
typedef struct clock{
	int *procesos;
} RelojLogico;

/*Metodos auxiliares del reloj, que mostrara el estado del reloj o lo actualizará.*/
void getclock(int nProcesos, int *estados, char* proceso);
void event(int nProceso, int *estados, char *proceso);

#endif
