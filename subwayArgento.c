#include <stdio.h>      // libreria estandar
#include <stdlib.h>     // para usar exit y funciones de la libreria standard
#include <string.h>
#include <pthread.h>    // para usar threads
#include <semaphore.h>  // para usar semaforos
#include <unistd.h>

#define LIMITE 50
#define LIMITE_ACCIONES 9
#define LIMITE_INGREDIENTES 10
#define CANT_EQUIPOS 4


//Creo estructura de semaforos 
struct semaforos {
    sem_t sem_mezclar;
	sem_t sem_sazonar_mezcla;
	sem_t sem_agregar_carne;
	sem_t sem_empanar_carne;
	sem_t sem_freir_milanesa;
	sem_t sem_hornear_pan;
	sem_t sem_cortar_vegetales;
	sem_t sem_armar_sandwich;
};

//Creo estructura de semaforos compartidos por los equipos
struct semaforos_compartidos {
    sem_t sem_imprimir_mensaje;
	sem_t sem_sazonar_mezcla;
	sem_t sem_freir_milanesa;
	sem_t sem_hornear_pan;
	sem_t sem_marcar_llegada;
};

struct semaforos_compartidos *semaforos_c;

int ordenFinalizacionEquipos[CANT_EQUIPOS] = {-1,-1,-1,-1};

//Creo los pasos con los ingredientes
struct ingrediente {
	char nombre_ingrediente[LIMITE];
};


struct accion {
	char nombre_accion[LIMITE];
    struct ingrediente ingredientes[LIMITE_INGREDIENTES];
};

//Creo los parametros de los hilos 
struct parametro {
 int equipo_param;
 struct semaforos semaforos_param[LIMITE_ACCIONES];
 struct accion pasos_param[LIMITE_ACCIONES];
};

//Inicializa la estructura de la receta
void inicializarStructAcciones(struct accion *acciones){
	int j;
	int x;
	for(j = 0; j < LIMITE_ACCIONES; j++){
		strcpy(acciones[j].nombre_accion, "");
		for(x = 0; x < LIMITE_INGREDIENTES; x++){		
			strcpy(acciones[j].ingredientes[x].nombre_ingrediente, "");
		}		
	}
}

//Funcion para levantar a memoria el archivo .txt con la receta
void obtenerReceta(struct accion *acciones){
	inicializarStructAcciones(acciones);
	FILE *origen;
	char linea[50];	

	origen= fopen("receta.txt","r");
	if (origen == NULL) {
		printf( "Problemas con la apertura de los ficheros.\n" );
		exit( 1 );
	}
	
	int j;
	int x;
	
	int indiceAccion = 0;
	while (feof(origen) == 0) {
		fgets(linea,50,origen);
		char nombre_accion[50];
		if(strcmp("Accion:\n",linea) == 0){
			fgets(nombre_accion,50,origen);
			strcpy(acciones[indiceAccion].nombre_accion, nombre_accion);
			fgets(linea,50,origen);
			int indiceIngrediente = 0;
			char nombre_ingrediente[50];
			fgets(nombre_ingrediente,50,origen);
			while(strcmp(nombre_ingrediente, "\n") != 0 & feof(origen) == 0){	
				strcpy(acciones[indiceAccion].ingredientes[indiceIngrediente].nombre_ingrediente, nombre_ingrediente);	
				fgets(nombre_ingrediente,50,origen);
				indiceIngrediente++;
			}
		}
		indiceAccion++;
	}
	
	if (fclose(origen)!= 0)
		printf( "Problemas al cerrar el fichero origen.txt\n" );
	
}

//Funcion para imprimir las acciones y los ingredientes de la accion
void* imprimirAccion(void *data, char *accionIn) {
    sem_wait(&semaforos_c->sem_imprimir_mensaje);
    FILE* fichero;
    fichero = fopen("ResultadoFinal.txt", "a");
    
	struct parametro *mydata = data;
	int i;
	for(i = 0; i < LIMITE_ACCIONES; i++){
		//Pregunto si la accion del array es igual a la pasada por parametro (si es igual la funcion strcmp devuelve cero)
		if(strcmp(mydata->pasos_param[i].nombre_accion, accionIn) == 0){
        fprintf(fichero,"\tEquipo %d esta: %s \n " , mydata->equipo_param, mydata->pasos_param[i].nombre_accion);
		printf("\tEquipo %d esta: %s \n " , mydata->equipo_param, mydata->pasos_param[i].nombre_accion);
		int h;
        fprintf(fichero,"\tEquipo %d -----------ingredientes : ----------\n",mydata->equipo_param);
		printf("\tEquipo %d -----------ingredientes : ----------\n",mydata->equipo_param); 
			for(h = 0; h < LIMITE_INGREDIENTES; h++) {
				//Consulto si la posicion tiene valor porque no se cuantos ingredientes tengo por accion 
				if(strlen(mydata->pasos_param[i].ingredientes[h].nombre_ingrediente) != 0) {
                    fprintf(fichero,"\tEquipo %d ingrediente  %d : %s \n",mydata->equipo_param,h,mydata->pasos_param[i].ingredientes[h].nombre_ingrediente);
					printf("\tEquipo %d ingrediente  %d : %s \n",mydata->equipo_param,h,mydata->pasos_param[i].ingredientes[h].nombre_ingrediente);
				}
			}
		}		
	}
	printf("\t----------------------------------------------\n\n\n"); 
    fprintf(fichero, "\t----------------------------------------------\n\n\n");
    fclose(fichero);
    sem_post(&semaforos_c->sem_imprimir_mensaje);
}

//--------------------Comienzan las funciones que ejecutan los pasos de la receta--------------------------
void* picarVegetales(void *data) {
	
	//Creo el nombre de la accion de la funcion 
	char *accion = "Picando vegetales\n";
	//Creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
	struct parametro *mydata = data;
	//Llamo a la funcion imprimir le paso el struct y la accion de la funcion
	imprimirAccion(mydata,accion);
	//Uso sleep para simular que que pasa tiempo
	usleep( 2000000 );
	//Doy la señal a la siguiente accion (picarVegetales me habilita mezclar)
    sem_post(&mydata->semaforos_param->sem_mezclar);
	
    pthread_exit(NULL);
}

void* mezclar(void *data) {
	
	//Creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param->sem_mezclar);
	//Creo el nombre de la accion de la funcion 
	char *accion = "Mezclando vegetales con huevo\n";
	
	//Llamo a la funcion imprimir le paso el struct y la accion de la funcion
	imprimirAccion(mydata,accion);
	//Uso sleep para simular que que pasa tiempo
	usleep( 2000000 );
	//Doy la señal a la siguiente accion (mezclar me habilita sazonarMezcla)
    sem_post(&mydata->semaforos_param->sem_sazonar_mezcla);
	
    pthread_exit(NULL);
}

void* sazonarMezcla(void *data) {
	
	//Creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param->sem_sazonar_mezcla);
	sem_wait(&semaforos_c->sem_sazonar_mezcla);
	//Creo el nombre de la accion de la funcion 
	char *accion = "Sazonando\n";
	//Llamo a la funcion imprimir le paso el struct y la accion de la funcion
	imprimirAccion(mydata,accion);
	//Uso sleep para simular que que pasa tiempo
	usleep( 2000000 );
	//Doy la señal a la siguiente accion (sazonarMezcla me habilita agregarCarne)
    sem_post(&semaforos_c->sem_sazonar_mezcla);
	sem_post(&mydata->semaforos_param->sem_agregar_carne);
    pthread_exit(NULL);
}

void* agregarCarne(void *data) {
	
	//Creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param->sem_agregar_carne);
	//Creo el nombre de la accion de la funcion 
	char *accion = "Agregando carne a la mezcla\n";
	//Llamo a la funcion imprimir le paso el struct y la accion de la funcion
	imprimirAccion(mydata,accion);
	//Uso sleep para simular que que pasa tiempo
	usleep( 2000000 );
	//Doy la señal a la siguiente accion (agregarCarne me habilita empanarCarne)
	sem_post(&mydata->semaforos_param->sem_empanar_carne);
    pthread_exit(NULL);
}

void* empanarCarne(void *data) {
	
	//Creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param->sem_empanar_carne);
	//Creo el nombre de la accion de la funcion 
	char *accion = "Empanando carne\n";
	//Llamo a la funcion imprimir le paso el struct y la accion de la funcion
	imprimirAccion(mydata,accion);
	//Uso sleep para simular que que pasa tiempo
	usleep( 2000000 );
	//Doy la señal a la siguiente accion (empanarCarne me habilita freirMilanesa)
	sem_post(&mydata->semaforos_param->sem_freir_milanesa);
    pthread_exit(NULL);
}

void* freirMilanesa(void *data) {
	
	//Creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
	struct parametro *mydata = data;	
	sem_wait(&mydata->semaforos_param->sem_freir_milanesa);
	sem_wait(&semaforos_c->sem_freir_milanesa);
	//Creo el nombre de la accion de la funcion 
	char *accion = "Fritando milanesa\n";
	//Llamo a la funcion imprimir le paso el struct y la accion de la funcion
	imprimirAccion(mydata,accion);
	//Uso sleep para simular que que pasa tiempo
	usleep( 2000000 );
	//Doy la señal a la siguiente accion (freirMilanesa me habilita hornearPan)
	sem_post(&semaforos_c->sem_freir_milanesa);
	sem_post(&mydata->semaforos_param->sem_hornear_pan);
    pthread_exit(NULL);
}

void* hornearPan(void *data) {
	
	//Creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
	struct parametro *mydata = data;	
	sem_wait(&mydata->semaforos_param->sem_hornear_pan);
	sem_wait(&semaforos_c->sem_hornear_pan);
	//Creo el nombre de la accion de la funcion 
	char *accion = "Horneando pan\n";
	//Llamo a la funcion imprimir le paso el struct y la accion de la funcion
	imprimirAccion(mydata,accion);
	//Uso sleep para simular que que pasa tiempo
	usleep( 2000000 );
	//Doy la señal a la siguiente accion (hornearPan me habilita cortarVegetales)
	sem_post(&semaforos_c->sem_hornear_pan);
	sem_post(&mydata->semaforos_param->sem_cortar_vegetales);
    pthread_exit(NULL);
}

void* cortarVegetales(void *data) {
	
	//Creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
	struct parametro *mydata = data;	
	sem_wait(&mydata->semaforos_param->sem_cortar_vegetales);
	//Creo el nombre de la accion de la funcion 
	char *accion = "Cortando vegetales\n";
	//Llamo a la funcion imprimir le paso el struct y la accion de la funcion
	imprimirAccion(mydata,accion);
	//Uso sleep para simular que que pasa tiempo
	usleep( 2000000 );
	//Doy la señal a la siguiente accion (cortarVegetales me habilita armarSandwich)
	sem_post(&mydata->semaforos_param->sem_armar_sandwich);
    pthread_exit(NULL);
}

void* armarSandwich(void *data) {
	
	//Creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param->sem_armar_sandwich);
	//Creo el nombre de la accion de la funcion 
	char *accion = "Armando sandwich\n";
	//Llamo a la funcion imprimir le paso el struct y la accion de la funcion
	imprimirAccion(mydata,accion);
	//Uso sleep para simular que que pasa tiempo
	usleep( 2000000 );
	//Doy la señal a la siguiente accion (armarSandwich me habilita a marcar llegada)
	sem_wait(&semaforos_c->sem_marcar_llegada);
	int i;
	for(i = 0; i < CANT_EQUIPOS; i++){
		if(ordenFinalizacionEquipos[i] == -1){
			ordenFinalizacionEquipos[i] = mydata->equipo_param;
			break;
		}
	}
	sem_post(&semaforos_c->sem_marcar_llegada);
    pthread_exit(NULL);
}

void* ejecutarReceta(void *i) {
	//Variables semaforos
	sem_t sem_mezclar;
	sem_t sem_sazonar_mezcla;
	sem_t sem_agregar_carne;
	sem_t sem_empanar_carne;
	sem_t sem_freir_milanesa;
	sem_t sem_hornear_pan;
	sem_t sem_cortar_vegetales;
	sem_t sem_armar_sandwich;
	
	//Variables hilos
	pthread_t h_picarVegetales;
	pthread_t h_mezclar;
	pthread_t h_sazonarMezcla;
	pthread_t h_agregarCarne;
	pthread_t h_empanarCarne;
	pthread_t h_freirMilanesa;
	pthread_t h_hornearPan;
	pthread_t h_cortarVegetales;
	pthread_t h_armarSandwich;
	
	//Numero del equipo (casteo el puntero a un int)
	int p = *((int *) i);
	
	//Reservo memoria para el struct
	struct parametro *pthread_data = malloc(sizeof(struct parametro));

	//Seteo los valores al struct
	
	//Seteo numero de grupo
	pthread_data->equipo_param = p;

	//Seteo semaforos
	pthread_data->semaforos_param->sem_mezclar = sem_mezclar;	
	pthread_data->semaforos_param->sem_sazonar_mezcla = sem_sazonar_mezcla;
	pthread_data->semaforos_param->sem_agregar_carne = sem_agregar_carne;
	pthread_data->semaforos_param->sem_empanar_carne = sem_empanar_carne;
	pthread_data->semaforos_param->sem_freir_milanesa = sem_freir_milanesa;
	pthread_data->semaforos_param->sem_hornear_pan = sem_hornear_pan;
	pthread_data->semaforos_param->sem_cortar_vegetales = sem_cortar_vegetales;
	pthread_data->semaforos_param->sem_armar_sandwich = sem_armar_sandwich;
	
	//Levanto a memoria toda la estructura de la receta
	obtenerReceta(pthread_data->pasos_param);
	
	//Inicializo los semaforos
	sem_init(&(pthread_data->semaforos_param->sem_mezclar),0,0);
	sem_init(&(pthread_data->semaforos_param->sem_sazonar_mezcla),0,0);   
	sem_init(&(pthread_data->semaforos_param->sem_agregar_carne),0,0);
	sem_init(&(pthread_data->semaforos_param->sem_empanar_carne),0,0);
	sem_init(&(pthread_data->semaforos_param->sem_freir_milanesa),0,0);   
	sem_init(&(pthread_data->semaforos_param->sem_hornear_pan),0,0);      
	sem_init(&(pthread_data->semaforos_param->sem_cortar_vegetales),0,0);
	sem_init(&(pthread_data->semaforos_param->sem_armar_sandwich),0,0);

	//Creo los hilos a todos les paso el struct creado (el mismo a todos los hilos) ya que todos comparten los semaforos 
    int r;
    r = pthread_create(&h_picarVegetales,                           //identificador unico
                            NULL,                          //atributos del thread
                                picarVegetales,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia
	
	
	r = pthread_create(&h_mezclar,                           //identificador unico
                            NULL,                          //atributos del thread
                                mezclar,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia
	
	
	r = pthread_create(&h_sazonarMezcla,                           //identificador unico
                            NULL,                          //atributos del thread
                                sazonarMezcla,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia
	

	r = pthread_create(&h_agregarCarne,                           //identificador unico
                            NULL,                          //atributos del thread
                                agregarCarne,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia
	
	r = pthread_create(&h_empanarCarne,                           //identificador unico
                            NULL,                          //atributos del thread
                                empanarCarne,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia
	
	r = pthread_create(&h_freirMilanesa,                           //identificador unico
                            NULL,                          //atributos del thread
                                freirMilanesa,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia
	
	r = pthread_create(&h_hornearPan,                           //identificador unico
                            NULL,                          //atributos del thread
                                hornearPan,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia
	
	r = pthread_create(&h_cortarVegetales,                           //identificador unico
                            NULL,                          //atributos del thread
                                cortarVegetales,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia
	
	r = pthread_create(&h_armarSandwich,                           //identificador unico
                            NULL,                          //atributos del thread
                                armarSandwich,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia
	
	//Join de todos los hilos
	pthread_join(h_picarVegetales,NULL);
	pthread_join(h_mezclar,NULL);
	pthread_join(h_sazonarMezcla,NULL);
	pthread_join(h_agregarCarne,NULL);
	pthread_join(h_empanarCarne,NULL);
	pthread_join(h_freirMilanesa,NULL);
	pthread_join(h_hornearPan,NULL);
	pthread_join(h_cortarVegetales,NULL);
	pthread_join(h_armarSandwich,NULL);

	//Valido que el hilo se alla creado bien 
    if (r){
       printf("Error:unable to create thread, %d \n", r);
       exit(-1);
     }
	 
	//Destruyo los semaforos 
	sem_destroy(&sem_mezclar);
	sem_destroy(&sem_sazonar_mezcla);	
	sem_destroy(&sem_agregar_carne);
	sem_destroy(&sem_empanar_carne);
	sem_destroy(&sem_freir_milanesa);
	sem_destroy(&sem_hornear_pan);
	sem_destroy(&sem_cortar_vegetales);
	sem_destroy(&sem_armar_sandwich);
	
	//Salida del hilo
	 pthread_exit(NULL);
}


int main ()
{
	//Inicializo los semaforos compartidos
	semaforos_c = malloc(sizeof(struct semaforos_compartidos));	
    sem_init(&(semaforos_c->sem_imprimir_mensaje),0,1);  
	sem_init(&(semaforos_c->sem_sazonar_mezcla),0,1);   
	sem_init(&(semaforos_c->sem_freir_milanesa),0,1);   
	sem_init(&(semaforos_c->sem_hornear_pan),0,2);      
	sem_init(&(semaforos_c->sem_marcar_llegada),0,1);   
	
	//Creo los nombres de los equipos 
	int rc;
	int *equipoNombre1 =malloc(sizeof(*equipoNombre1));
	int *equipoNombre2 =malloc(sizeof(*equipoNombre2));
	int *equipoNombre3 =malloc(sizeof(*equipoNombre3));
	int *equipoNombre4 =malloc(sizeof(*equipoNombre4));
  
	*equipoNombre1 = 1;
	*equipoNombre2 = 2;
	*equipoNombre3 = 3;
	*equipoNombre4 = 4;

	//Creo las variables los hilos de los equipos
	pthread_t equipo1; 
	pthread_t equipo2;
	pthread_t equipo3;
	pthread_t equipo4;
	
	//Inicializo los hilos de los equipos
    rc = pthread_create(&equipo1,                           //identificador unico
                        NULL,                          //atributos del thread
                        ejecutarReceta,             //funcion a ejecutar
                        equipoNombre1); 

    rc = pthread_create(&equipo2,                           //identificador unico
                        NULL,                          //atributos del thread
                        ejecutarReceta,             //funcion a ejecutar
                        equipoNombre2);
	
	rc = pthread_create(&equipo3,                           //identificador unico
                        NULL,                          //atributos del thread
                        ejecutarReceta,             //funcion a ejecutar
                        equipoNombre3);
	
	rc = pthread_create(&equipo4,                           //identificador unico
                        NULL,                          //atributos del thread
                        ejecutarReceta,             //funcion a ejecutar
                        equipoNombre4);


   if (rc){
       printf("Error:unable to create thread, %d \n", rc);
       exit(-1);
     } 

	//Join de todos los hilos
	pthread_join (equipo1,NULL);
	pthread_join (equipo2,NULL);
	pthread_join (equipo3,NULL);
	pthread_join (equipo4,NULL);
	
	//Imprimo resultados
	printf("Estos son los resultados: \n");
	printf("El primer lugar es para el Equipo %d \n",ordenFinalizacionEquipos[0]);
	printf("El segundo lugar es para el Equipo %d \n",ordenFinalizacionEquipos[1]);
	printf("El tercer lugar es para el Equipo %d \n",ordenFinalizacionEquipos[2]);
	printf("El último lugar es para el Equipo %d \n",ordenFinalizacionEquipos[3]);
    
    //Imprimo resultados en el archivo txt
    FILE* fichero;
    fichero = fopen("ResultadoFinal.txt", "a");
    fprintf(fichero,"Estos son los resultados: \n");
	fprintf(fichero,"El primer lugar es para el Equipo %d \n",ordenFinalizacionEquipos[0]);
	fprintf(fichero,"El segundo lugar es para el Equipo %d \n",ordenFinalizacionEquipos[1]);
	fprintf(fichero,"El tercer lugar es para el Equipo %d \n",ordenFinalizacionEquipos[2]);
	fprintf(fichero,"El último lugar es para el Equipo %d \n",ordenFinalizacionEquipos[3]);
    fclose(fichero);
	
	//Destruyo los semaforos compartidos
	sem_destroy(&(semaforos_c->sem_sazonar_mezcla));
	sem_destroy(&(semaforos_c->sem_freir_milanesa));
	sem_destroy(&(semaforos_c->sem_hornear_pan));
	sem_destroy(&(semaforos_c->sem_marcar_llegada));
    pthread_exit(NULL);
}


//Para compilar:   gcc subwayArgento.c -o ejecutable -lpthread
//Para ejecutar:   ./ejecutable
