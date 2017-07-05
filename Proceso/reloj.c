#include "reloj.h"

/*Funcion que nos imprime el estado del reloj que pasaamos como parametro.*/
void getclock(int nProcesos, int *estados, char* proceso){
	int i;
	printf("%s: LC[", proceso);
	for(i = 0; i < nProcesos; i++) {
		printf( "%d", estados[i]);
		if (i != nProcesos - 1) {
			printf(",");
		}
	}
	printf( "]\n");
}

/*Aumenta el valor del reloj logico del estado pasado como parametro.*/
void event(int nProceso, int *estados, char *proceso){
	estados[nProceso]++; //Aumentamos el valor.
	printf("%s: TICK\n", proceso); //Imprimimos el mensaje comprobador del aumento.
}
