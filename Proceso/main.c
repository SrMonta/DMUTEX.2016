/* DMUTEX (2009) Sistemas Operativos Distribuidos
 * Codigo de Apoyo
 *
 * ESTE CODIGO DEBE COMPLETARLO EL ALUMNO:
 *    - Para desarrollar las funciones de mensajes, reloj y
 *      gestion del bucle de tareas se recomienda la implementaci�n
 *      de las mismas en diferentes ficheros.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "reloj.h"

int enviar(int mode, char *name);
void solicitar(int idSection);
void mutex(int idSection, char *name);
int insertList(int puerto, char *direccion, char *id);
int insertSection(char *name);
int searchSection(char *name);
void procesoAEspera(int idSection, int proceso);
int checkMinor(int *reloj1, int *reloj2);

/*METODOS AUXILIARES DE VISUALIZACION DE LISTAS DE PROCESOS Y SECCIONES.*/
void printList(); //Imprime una lista en pantalla con los procesos, mostrando su puerto, direccion y el ID en lista.
void printSections(); //Imprime una lista en pantalla con las secciones, mostrando su nombre y el ID en lista.
/*METODOS AUXILIARES DE VISUALIZACION DE LISTAS DE PROCESOS Y SECCIONES.*/

typedef enum {MSG = 0, LOCK = 1, OK = 2} msg_type;

typedef struct lProc{
	int puerto;
	char *direccion, *id;
} Process;

typedef struct section{
	int mutex, solicitado, nSolicitantes, nOks, *idSolicitantes;
	char *name;
} CriticalSection;

int sock_udp, puerto_udp, idProc, nProcesos = 0, nSecciones = 0;
char *proceso, *buffer;
Process *listaProc;
CriticalSection *listaSecciones;
RelojLogico reloj;

int main(int argc, char* argv[]){

    int port;
    struct sockaddr_in addr;
    char line[80],proc[80], *addr_udp;
		listaProc = (Process *)malloc(sizeof(struct lProc));
		listaSecciones = (CriticalSection *)malloc(sizeof(struct section));

		/*Comprobamos la ejecucion del programa.*/
    if(argc < 2){
    	fprintf(stderr,"Uso: proceso <ID>\n");
    	return 1;
    }

    /*Creamos el socket UDP por el que recibiremos la informacion de otros procesos.*/
    if ((sock_udp = socket (AF_INET, SOCK_DGRAM, 0)) == -1) {
        fprintf(stderr, "Error en la creacion del socket UDP.\n");
        close(sock_udp);
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(0);

    /*Obtenemos una direccion y un puerto para el proceso actual.*/
    if (bind (sock_udp, (struct sockaddr *)&addr, sizeof(addr))) {
			fprintf(stderr, "Error en el bind del socket.\n");
			close(sock_udp);
			return -1;
		}

		struct sockaddr_in asignacion; //Guardaremos aqui la informacion del proceso.
		socklen_t addrlen = sizeof(asignacion);

		/*Obtenemos la informacion resultante del bind.*/
		if (getsockname(sock_udp, (struct sockaddr *)&asignacion, (socklen_t *)&addrlen) == -1) {
			fprintf(stderr, "Error al ejecutar getsockname().\n");
			close(sock_udp);
			return -1;
		}

    proceso = argv[1]; //El nombre del proceso.
		puerto_udp = asignacion.sin_port; //El puerto para el proceso ser� el obtenido en el bind.
		addr_udp = inet_ntoa(asignacion.sin_addr); //Guardamos la direcci�n de la m�quina. (En esta pr�ctica no es importante.)

    fprintf(stdout, "%s: %d\n", proceso, puerto_udp);

  	/* Establece el modo buffer de entrada/salida a l�nea */
 		setvbuf(stdout,(char*)malloc(sizeof(char)*80),_IOLBF,80);
  	setvbuf(stdin,(char*)malloc(sizeof(char)*80),_IOLBF,80);

  	for(;fgets(line,80,stdin);){
   		if(!strcmp(line,"START\n")) break;

    	sscanf(line,"%[^:]: %d",proc,&port);

    	if(!strcmp(proc,argv[1])){ idProc = nProcesos;}

    	/* Habra que guardarlo en algun sitio */
    	/*Insertamos el proceso en la lista din�mica de procesos.*/
    	if(insertList(port, addr_udp, proc) == -1) return -1;
  	}

  	/* Inicializar Reloj */

  	reloj.procesos = (int *)malloc(sizeof(int) * nProcesos);
  	int i;
		for (i = 0; i < nProcesos; i++) reloj.procesos[i] = 0;

  	/* Procesar Acciones */

		for(;fgets(line,80,stdin);){

			char accion[10];
			char accionParam[70];
			sscanf(line, "%s %s", accion, accionParam);
			buffer = (char*) malloc((sizeof(int)*(nProcesos+2))+strlen(accionParam));

			if (!strcmp(line, "EVENT\n")) event(idProc, reloj.procesos, proceso);

			else if (!strcmp(line, "GETCLOCK\n")) getclock(nProcesos, reloj.procesos, proceso);

			else if (strcmp(accion, "MESSAGETO")==0){
				event(idProc, reloj.procesos, proceso);
				enviar(0, accionParam);
			}

			else if (!strncmp(line, "LOCK", strlen("LOCK"))){
				char seccion[strlen(accionParam)];
				strcpy(seccion, accionParam);

				int idSection = searchSection(seccion);
				if(idSection == -1)	solicitar(insertSection(seccion));
				else solicitar(idSection);

				event(idProc, reloj.procesos, proceso);
				enviar(1, seccion);
			}

			else if (!strncmp(line, "UNLOCK", strlen("UNLOCK"))){
				char seccion[strlen(accionParam)];
				strcpy(seccion, accionParam);

				enviar(3, seccion);

				int idSection = searchSection(seccion);
				listaSecciones[idSection].idSolicitantes = (int*) malloc(sizeof(int));
				listaSecciones[idSection].mutex = 0;

			}

			else if (!strcmp(line, "RECEIVE\n")){
				if (read(sock_udp, buffer, sizeof(buffer)) < 0) {
					fprintf(stderr, "Error en el read\n");
					close(sock_udp);
					return -1;
				}

				if(buffer[0] == MSG) printf("%s: RECEIVE(MSG,%s)\n", proceso, listaProc[(int)buffer[1]].id);
				else if(buffer[0] == LOCK) printf("%s: RECEIVE(LOCK,%s)\n", proceso, listaProc[(int)buffer[1]].id);
				else printf("%s: RECEIVE(OK,%s)\n", proceso, listaProc[(int)buffer[1]].id);
				event(idProc, reloj.procesos, proceso);

				int relojTmp[nProcesos], i;
				for (i = 0; i < nProcesos; i++) relojTmp[i] = buffer[i+2];
				int flagMinor = checkMinor(relojTmp, reloj.procesos);
				if(flagMinor == 1) flagMinor = (checkMinor(reloj.procesos, relojTmp) == 1)? 2 : 1;

				for (i = 0; i < nProcesos; i++) if(reloj.procesos[i] < buffer[i+2]) reloj.procesos[i] = buffer[i+2];

				if (buffer[0] == LOCK){
					char name[strlen(&buffer[nProcesos+2])];
					strcpy(name, &buffer[nProcesos+2]);

					int idSection = searchSection(name);

					if(idSection == -1)	{
						event(idProc, reloj.procesos, proceso);
						enviar(2, name);
						insertSection(name);
					}

					else {
						if(listaSecciones[idSection].mutex == 1) procesoAEspera(idSection, buffer[1]);
						else if(listaSecciones[idSection].solicitado == 1){
							if(flagMinor == 0) {
								event(idProc, reloj.procesos, proceso);
								enviar(2, name);
							}
							else if(flagMinor == 1) procesoAEspera(idSection, buffer[1]);
							else{
								if(idProc > (int)buffer[1]){
									event(idProc, reloj.procesos, proceso);
									enviar(2, name);
								}
								else procesoAEspera(idSection, buffer[1]);
							}
						}
						else{
							event(idProc, reloj.procesos, proceso);
							enviar(2, name);
						}
					}
				}

				else if (buffer[0] == OK){
					int idSection = searchSection(&buffer[nProcesos+2]);
					listaSecciones[idSection].nOks--;
					if(listaSecciones[idSection].nOks == 0) mutex(idSection, &buffer[nProcesos+2]);
				}
		}

		else if (!strcmp(line, "FINISH\n")) break;
		free(buffer);
	}

  return 0;
}

/******************************************************************************/
/******************************METODOS AUXILIARES******************************/
/******************************************************************************/
/*Metodo que usaremos para enviar cosas al resto de procesos.*/
int enviar(int mode, char *name){
	struct sockaddr_in addr_env;
	addr_env.sin_family = AF_INET;
	addr_env.sin_addr.s_addr = INADDR_ANY;

	int j;
	for(j = 0; j < nProcesos; j++) buffer[j+2] = reloj.procesos[j];

	if(mode == 0){
		int i;
		for(i = 0; i < nProcesos; i++) if(strcmp(listaProc[i].id, name) == 0) {addr_env.sin_port = listaProc[i].puerto; break;}

		buffer[0] = MSG;
		buffer[1] = idProc;

		if (sendto(sock_udp, buffer, sizeof(buffer), 0, (struct sockaddr *) &addr_env, sizeof(addr_env)) < 0) {
			printf("Error en el sendto\n");
			close(sock_udp);
			return -1;
		}

		printf("%s: SEND(MSG,%s)\n", proceso, name);
	}

	else if(mode == 1){
		int i;
		for(i = 0; i < nProcesos; i++){
			if(i == idProc) continue;
			addr_env.sin_port = listaProc[i].puerto;

			buffer[0] = LOCK;
			buffer[1] = idProc;
			strcpy(&buffer[nProcesos+2], name);

			if (sendto(sock_udp, buffer, sizeof(buffer), 0, (struct sockaddr *) &addr_env, sizeof(addr_env)) < 0) {
				printf("Error en el sendto\n");
				close(sock_udp);
				return -1;
			}

			printf("%s: SEND(LOCK,%s)\n", proceso, listaProc[i].id);
		}
	}

	else if(mode == 2){
		int idProcDestino = (int) buffer[1];
		addr_env.sin_port = listaProc[idProcDestino].puerto;

		buffer[0] = OK;
		buffer[1] = idProc;
		strcpy(&buffer[nProcesos+2], name);

		if (sendto(sock_udp, buffer, sizeof(buffer), 0, (struct sockaddr *) &addr_env, sizeof(addr_env)) < 0) {
			printf("Error en el sendto\n");
			close(sock_udp);
			return -1;
		}

		printf("%s: SEND(OK,%s)\n", proceso, listaProc[idProcDestino].id);
	}

	else if(mode == 3){
		int i, idSection = searchSection(name);
		for(i = 0; i < listaSecciones[idSection].nSolicitantes; i++){
			event(idProc, reloj.procesos, proceso);
			addr_env.sin_port = listaProc[listaSecciones[idSection].idSolicitantes[i]].puerto;

			int x;
			for(x = 0; x < nProcesos; x++) memcpy((int *) &buffer[x+2], (int *) &reloj.procesos[x], sizeof(int));

			buffer[0] = OK;
			buffer[1] = idProc;
			strcpy(&buffer[nProcesos+2], name);

			if (sendto(sock_udp, buffer, sizeof(buffer), 0, (struct sockaddr *) &addr_env, sizeof(addr_env)) < 0) {
				printf("Error en el sendto\n");
				close(sock_udp);
				return -1;
			}
			printf("%s: SEND(OK,%s)\n", proceso, listaProc[listaSecciones[idSection].idSolicitantes[i]].id);
		}
	}

	return 0;
}

void solicitar(int idSection){
	listaSecciones[idSection].solicitado = 1;
}

void mutex(int idSection, char *name){
	listaSecciones[idSection].solicitado = 0;
	listaSecciones[idSection].mutex = 1;
	printf("%s: MUTEX(%s)\n", proceso, name);
}

/*Metodo usado para comprobar el estado de los relojes logicos. Devuelve 0 si reloj1 es menor que reloj2, 1 en otro caso.*/
int checkMinor(int *reloj1, int *reloj2){
	int i, flag = 1;
	for(i = 0; i < nProcesos; i++){
		if(reloj1[i] > reloj2[i]) return 1;
		if(reloj1[i] < reloj2[i]) flag = 0;
	}
	return flag;
}

/*Insertamos un nuevo elemento a la lista y aumentamos su espacio en memoria.*/
int insertList(int puerto, char *direccion, char *id){
	Process tmp;
	tmp.puerto = puerto;
	tmp.direccion = (char *) malloc(strlen(direccion));
	strcpy(tmp.direccion, direccion);
	tmp.id = (char *) malloc(strlen(id));
	strcpy(tmp.id, id);
	listaProc[nProcesos++] = tmp;

	listaProc = (Process *)realloc(listaProc,(nProcesos+1)*sizeof(struct lProc));
	if(listaProc == NULL){
		fprintf(stderr, "No se pudo ubicar la memoria dinamica necesaria\n");
		return -1;
	}

	return 0;
}

/*Insertamos un nuevo elemento a la lista y aumentamos su espacio en memoria.*/
int insertSection(char *name){
	CriticalSection tmp;
	tmp.mutex = 0;
	tmp.nSolicitantes = 0;
	tmp.solicitado = 0;
	tmp.name = (char *) malloc(strlen(name));
	strcpy(tmp.name, name);
	tmp.nOks = nProcesos - 1;
	tmp.idSolicitantes = (int *) malloc(sizeof(int));
	listaSecciones[nSecciones++] = tmp;

	listaSecciones = (CriticalSection *)realloc(listaSecciones,(nSecciones+1)*sizeof(struct section));
	if(listaSecciones == NULL){
		fprintf(stderr, "No se pudo ubicar la memoria dinamica necesaria\n");
		return -1;
	}

	return nSecciones -1;
}

void procesoAEspera(int idSection, int proceso){
	listaSecciones[idSection].idSolicitantes[listaSecciones[idSection].nSolicitantes++] = (int)proceso;
	listaSecciones[idSection].idSolicitantes = (int*) realloc(listaSecciones[idSection].idSolicitantes, (listaSecciones[idSection].nSolicitantes+1)*sizeof(int));
}

int searchSection(char *name){
	int i, pos = -1;
	for(i = 0; i < nSecciones && pos == -1; i++) if(!strcmp(listaSecciones[i].name, name)) pos = i;
	return pos;
}
/******************************************************************************/
/******************************METODOS AUXILIARES******************************/
/******************************************************************************/

/******************************************************************************/
/****METODOS AUXILIARES DE VISUALIZACION DE LISTAS DE PROCESOS Y SECCIONES.****/
/******************************************************************************/
/*Imprimimos una vista con los procesos que estan incluidos en la lista.*/
void printList(){
	int i;
	printf("+-----------------------------------------------+\n");
	printf("| Puerto \t\t| Direccion \t| ID \t\t|\n");
	for(i=0; i<nProcesos; i++){
		printf("| %d \t\t| %s \t| %s \t\t|\n", listaProc[i].puerto, listaProc[i].direccion, listaProc[i].id);
	}
	printf("+-----------------------------------------------+\n");
}

/*Imprimimos una vista con las secciones que estan incluidas en la lista.*/
void printSections(){
	int i;
	printf("+-----------------------------------------------+\n");
	printf("| Name \t\t| ID \t\n");
	for(i=0; i<nSecciones; i++){
		printf("| %s \t\t| %d \t\n", listaSecciones[i].name, i);
	}
	printf("+-----------------------------------------------+\n");
}
/******************************************************************************/
/****METODOS AUXILIARES DE VISUALIZACION DE LISTAS DE PROCESOS Y SECCIONES.****/
/******************************************************************************/
