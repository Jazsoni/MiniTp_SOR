## SOR-Semáforos-1S-2021
Trabajo Práctico Semáforos primer semestre año 2021

## Subway Argento
Este proyecto fue desarrollado con el fin de demostrar los conocimientos aprendidos sobre el uso de semaforos en SOR.

## Requisitos
* Un sistema operativo que posea una terminal de linux
* Tener instalado el programa gcc para poder compilar el archivo subwayArgento.c

## Como correrlo en la PC
* Se deben descargar los archivos receta.txt y subwayArgento.c en un mismo directorio
* Abrir una terminal y ubicar el archivo subwayArgento.c
* Ejecutar el archivo con el comando: 
```bash
gcc subwayArgento.c -o ejecutable -lpthread
./ejecutable
```
* Ver el resultado

## Documentación del desarrollo
Para el desarrollo de este programa se tuvieron las siguientes consideraciones:
* Obtener receta desde archivo receta.txt
* Semáforos compartidos dentro de la ejecución de cada equipo
* Semáforos compartidos entre equipos
* Almacenamiento de resultados

## Obtener receta desde archivo receta.txt
El programa toma los pasos de ejecución que se encuentran dentro del archivo “receta.txt”. Dicho archivo debe contener respetar la siguiente estructura para el correcto funcionamiento del programa:
```bash
Acción:
[nombre_de_accion]
Ingredientes:
[nombre_ingrediente_1]
[nombre_ingrediente_2]
[nombre_ingrediente_n]
[enter]
```
Por ejemplo:


![Captura de pantalla de 2021-04-18 13-06-38](https://user-images.githubusercontent.com/24569650/115155917-56471f80-a058-11eb-90ee-d38ea0789947.png)


Cada nombre de acción debe corresponderse con el nombre que utiliza cada función que ejecutará un paso de la receta. Aquí presentamos un ejemplo:


![Captura de pantalla de 2021-04-18 13-14-20](https://user-images.githubusercontent.com/24569650/115155920-5e06c400-a058-11eb-8b91-a989b3020ddc.png)



La variable *accion almacena el nombre “Picando vegetales\n” y este se corresponde con una de las acciones que se especifica en el archivo “receta.txt”. Hay que tener en cuenta que el ‘\n’ que realiza el salto de línea no hay que escribirlo en el archivo .txt ya que este ya es interpretado de una forma en la cual se puede ver ese salto de línea desde el programa. 

Luego en el apartado de ingredientes de cada acción puede escribirse lo que sea.

## Semáforos compartidos dentro de la ejecución de cada equipo
Como existe un orden en el cual se va ejecutando la receta tenemos que implementar una serie de semáforos para hacer que la ejecución respete ese orden.
Para ello se crearon los siguientes semáforos que son compartidos dentro del hilo del equipo:
* sem_mezclar
* sem_sazonar_mezcla
* sem_agregar_carne
* sem_empanar_carne
* sem_freir_milanesa
* sem_hornear_pan
* sem_cortar_vegetales
* sem_armar_sandwich

Cada uno de estos semáforos son habilitados dentro de cada función de los pasos de la receta:
* picarVegetales()   => habilita el semáforo “sem_mezclar”
* mezclar()	      => habilita el semáforo “sem_sazonar_mezcla”
* sazonar()	      => habilita el semáforo “sem_agregar_carne”
* agregarCarne()     => habilita el semáforo “sem_empanar_carne”
* empanarCarne()   => habilita el semáforo “sem_freir_milanesa”
* freirMilanesa()      => habilita el semáforo “sem_hornear_pan”
* hornearPan()        => habilita el semáforo “sem_cortar_vegetales”
* cortarVegetales()  => habilita el semáforo “sem_armar_sandwich”
* armarSandwich() 

## Semáforos compartidos entre equipos

Estas habilitaciones de los semáforos internos de cada equipo marcan el flujo de ejecución de todas las tareas. Pero teniendo en cuenta que existen recursos compartidos por todos los equipos se implementaron también los siguientes semáforos de carácter global para controlar el acceso a estos recursos:
* sem_sazonar_mezcla   (Limita el uso del salero a un equipo a la vez)
* sem_freir_milanesa       (Limita el uso del sartén a un equipo a la vez)
* sem_hornear_pan	   (Limita el uso del horno a dos equipos a la vez)
* sem_marcar_llegada  (Limita el uso de la variable ordenFinalizacionEquipos a un equipo a la vez)
* sem_imprimir_mensaje  (Limita el uso de la escritura por pantalla y por archivo a un equipo a la vez)

Entonces las siguientes funciones además de esperar el semáforo interno que cada una tiene asignado tienen que esperar también el semáforo global que le corresponda:
* sazonar()	      => espera el semáforo global y el local “sem_sazonar_mezcla”. 
* freirMilanesa()      => espera el semáforo global y el local “sem_freir_milanesa”. 
* hornearPan()        => espera el semáforo global y el local “sem_hornear_pan”. 

Para finalizar, el último semáforo compartido que ejecuta cada equipo es el “sem_marcar_llegada” y este controla que la variable global “ordenFinalizacionEquipos” sea accedida por un grupo a la vez. Esta variable es un array de enteros que almacena el orden en el que cada equipo va terminando de hacer el sandwich. Luego este orden es utilizado para imprimir por pantalla el resultado final del juego.

## Almacenamiento de resultados

Existe un semáforo global llamado “sem_imprimir_mensaje” y este es utilizado para limitar el uso de la escritura por pantalla y por archivo. El archivo de salida donde se guarda toda la información del flujo de ejecución se llama ResutadoFinal.txt. Si no existe es creado y si existe se le agrega la información actual al final del mismo. Por eso es recomendable borrar el archivo antes de ejecutar nuevamente el programa ya que de lo contrario acumulará resultados anteriores.

Aquí mostramos el esquema en pseudocodigo de los semáforos utilizados:
![Captura de pantalla de 2021-04-18 14-58-23](https://user-images.githubusercontent.com/24569650/115155932-6a8b1c80-a058-11eb-8f90-f972d8e9c12b.png)





