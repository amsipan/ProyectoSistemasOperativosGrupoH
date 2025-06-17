
# ***Proyecto Sistemas Operativos***

## ***Español***
**Ejecución de múltiples tareas con procesos y con hilos** 



## ***Autores*** 

- Jibaja Sebastian
- Mosquera Karen
- Oviedo Jorge
- Tamayo Oscar

## ***Requerimientos***

Para poder clonar y ejecutar este repositorio se recomienda el uso de ```Ubuntu server``` instalado en una máquina virtual como ```Oracle Virtual Machine```,, con las siguientes características mínimas:  
- 2 núcleos.
- 4GB de RAM.
- 256GB de Memoria.  
Además, se debe instalar el compilador para archivos ```.c``` llamado ```GCC, o GNU Compiler Collection```, para lo cual se debe ejecutar el siguiente comando dentro de la terminal de ```UbuntuServer```.
```bash
sudo apt update
sudo apt install build-essential
```


    
## ***Compilacion***

Los archivos ubicados en ```src``` son únicamente los códigos fuente, por lo cual se necesita la ejecución de los siguientes comandos.  

- *Para el archivo ```procesos.c```*
```bash
gcc procesos.c -o [nombre de salida] -lrt -lpthread -Wall
```  

- *Para el archivo ```hilos.c```*  
```bash
gcc hilos.c -o [nombre de salida] -O3 -march=native -flto -pthread -Wall
```  

- *Para el archivo ```run.c```*
```bash
gcc run.c -o [nombre de salida] -Wall
```  

## ***Ejecución***
Tras la previa compilación de los archivos, se tienen distintas formas de ejecutar los distintos archivos, por facilidad se nombrará a los ejecutables como:  

```procesos.c -> procesos_promedio```  
```hilos.c -> hilos_promedio```  
```run.c -> ejecutable```  

Por lo tanto, las distintas formas de ejecución se deberán realizar desde la terminal y seguirán los siguientes comandos:   
- *Únicamente hilos o únicamente procesos, con ejecución única de cada archivo*
```bash
./procesos_promedio
./hilos_promedio
```
- *Únicamente hilos o únicamente procesos, con ejecución por medio del archivo ejecutable*
```bash
./ejecutable procesos
./ejecutable hilos
```

- *Ejecución de procesos e hilos, con ejecución por medio del archivo ejecutable*
```bash
./ejecutable ambos
```

**Se recomienda que todos los archivos se encuentren dentro de una misma carpeta para evitar errores con las rutas de los archvios ```.txt```, en caso de que se trabaje con carpetas distintas revisar las rutas en cada uno de los archivos**
## ***Resultados Esperados***
- Impresión en consola la cual muestre los resultados, esta impresión seguirá la siguiente sintaxis para hilos y procesos, únicamente cambiando el título principal.
```bash
=== Procesos ===
Grupo [No Grupo] | Promedio: [promedio del grupo] | Reprobados: [número de reprobados] | Aprobados (18-27.99): [número de aprobados bajo] | Aprobados (28-40): [número de aprobados alto] | Tiempo: [tiempo de ejecución por grupo]  
=== Totales Globales ===
Reprobados: [número total de reprobados]
Aprobados: [número total de aprobados bajo]
Aprobados: [número total de aprobados alto]  
=== Resumen ===
Inicio: [Fecha y hora de inicio, incluyendo nanosegundos]
Fin: [Fecha y hora de fin, incluyendo nanosegundos]
Duración total: [Calculo de la duracion total del tiempo de ejecución]
```  

- Archivo ```.txt``` con los resultados obtenidos de cada ejecución, estos tendrán los nombres de ```resultados_hilos.txt``` y ```resultados_procesos```, los resultados se sobre escribirán con cada ejecución y tendrán la misma estructura de la sección anterior.

- Si se ejecutan ambos archivos mediante el uso del ejecutable, se imprimirá lo descrito en la primera sección para procesos e hilos y además se agrega la siguiente salida a la impresión.

```bash
=== Comparativa de tiempos ===
Procesos: [Duración total de procesos]
Hilos: [Duración total de hilos]

[Procesos o hilos] fue más rapido.
```

## ***Análisis de Problemas***
Dentro de la codificación de los distintos archivos, se presentaron distintos problemas, los cuales se describirán en esta sección:  
***1. Manejo de la memoria compartida.***  
Esto debido al control de los semáforos y mutex respectivamente, los cuales no se implementaban de manera adecuada y generaban que procesos sean notablemente más veloces en tiempos de ejecución con respecto a hilos, lo cual no tiene sentido.  
La solución que se encontró fue manejar las variables compartidas como un arreglo que se manejaba con un único semáforo o mutex según el caso, este arreglo se volvió la variable de memoria compartida controlada con los métodos de sincronización adecuados.  
***2. Impresiones durante la ejecución***   
Tras el arreglo correspondiente en semáforos y mutex, se presentó el problema de la impresión de resultados en consola, lo cual, dentro de hilos generaba un aumento significativo en la duración total del tiempo de ejecución, este problema se solucionó con la escritura en archivos ```.txt``` externos, y posteriormente, imprimir los resultados desde estos.  
***3. Manejo del tiempo***  
Para la implementación se usaron ```20000000``` notas para ejecución, Pero aún con esta medida, los tiempos en hilos no llegaban a segundos enteros, quedando en nanosegundos e incluso tiempos menores, esto suscitaba un inconveniente al momento de la comparación de velocidades entre procesos e hilos. Para lo cual se adecuó el formato del tiempo para que incluyera nanosegundos sin redondeos, arreglando así el problema para el cálculo de la duración total.  
***4. Uso de librerias en C***  
Debido al poco conociemiento sobre el lenguaje ```C``` se presentaron diversos inconvenientes con el manejo de las librerías como ```phtread.h``` o ```semaphore.h``` para la creación de los hilos y el uso de semaforos respectivamente.  Por lo cual se busco la documentación de cada una de ellas para identificar sus funciones y parámetros requeridos en cada una de ellas.
## ***Concluciones***
Tras la correcta implementación de los archivos ```prcesos``` e ```hilos``` se puede concluir que si se manejan de forma adecuada los respectivos métodos de sincronización en cada uno de ellos se puede observar de forma práctica lo aprendido durante las horas de clase. Además, el control y velocidad que ofrecen los hilos para la ejecución facilita la metodología de trabajo y su correspondiente implementación.  
Algunas diferencias muy notables y remarcables son en su mayoría en los hilos, puesto que en caso de existir una mala implementación se puede notar de forma muy rápida al visualizar los tiempos de ejecución, lo cual no sucede del todo en procesos, puesto que estos pueden solaparse y ejecutar en conjunto, llevando a una diferencia mayor. Otra diferencia adicional está presente en el uso de recursos para su ejecución, los procesos ocupan una mayor cantidad de recursos con respecto a hilos, lo que también afecta a la velocidad de ejecución, este mayor uso de recursos repercute al espacio de memoria, puesto que los procesos ocuparán más de esta con respecto a los hilos.  

## ***English***  

## ***Operative Systems Project***  

## ***Authors***  

- Jibaja Sebastian
- Mosquera Karen
- Oviedo Jorge
- Tamayo Oscar

To clone and run this repository, we recommend using ```Ubuntu Server``` installed on a ```Virtual Machine``` (e.g., Oracle VirtualBox), with the following minimum specifications:  
- 2 núcleos. 
- 4GB de RAM.
- 256GB de Memoria.  

Install the ```C``` compiler ```(GCC – GNU Compiler Collection)``` by running the following commands in the Ubuntu Server terminal:  
```bash
sudo apt update  
sudo apt install build-essential
```

## ***Compilation***  

The files in the src folder are source code only. Compile them using the following commands:  

- *For ```procesos.c```*  

```bash
gcc procesos.c -o [file name] -lrt -lpthread -Wall
```

- *For ```hilos.c```*  

```bash
gcc hilos.c -o [file name] -O3 -march=native -flto -pthread -Wall
```

- *For ```run.c```*

```bash
gcc run.c -o [file name] -Wall
```

## ***Execution***  

After compilation, the programs can be executed in the following ways from the terminal:  

- *Run each compiled file separately:*

```bash
./procesos_promedio  
./hilos_promedio
```

- *Run through the unified exectable:*

```bash
./ejecutable procesos  
./ejecutable hilos  
./ejecutable ambos
```

**It is recommended to keep all files in the same folder to avoid errors with ```.txt``` file paths. If using separate folders, update the paths in each file accordingly.**  

## ***Exepected Results***  

- Console output showing the results. Output format for both processes and threads is as follows (in Spanish):

```bash
=== Procesos ===  
Grupo [No Grupo] | Promedio: [promedio del grupo] | Reprobados: [número de reprobados] | Aprobados (18-27.99): [número de aprobados bajo] | Aprobados (28-40): [número de aprobados alto] | Tiempo: [tiempo de ejecución por grupo]  

=== Totales Globales ===  
Reprobados: [número total de reprobados]  
Aprobados: [número total de aprobados bajo]  
Aprobados: [número total de aprobados alto]  

=== Resumen ===  
Inicio: [Fecha y hora de inicio, incluyendo nanosegundos]  
Fin: [Fecha y hora de fin, incluyendo nanosegundos]  
Duración total: [Calculo de la duracion total del tiempo de ejecución]
```

- A ```.txt``` file will be created with the results of each execution: ```resultados_hilos.txt``` and  ```resultados_procesos.txt```. These files are overwritten with each run and follow the same structure as the console output.

- When running both via ejecutable, the following comparison is also printed:

```bash
=== Comparativa de tiempos ===
Procesos: [Duración total de procesos]  
Hilos: [Duración total de hilos]

[Procesos o hilos] fue más rápido.
```

## ***Problem Analysis***  

***1.Shared Memory Management***  
Initially, incorrect use of semaphores and mutexes caused processes to perform faster than threads. The solution involved storing shared variables in an array controlled by a single semaphore or mutex.  
***2.Console Output During Execution***  
Printing directly to the console increased execution time significantly in the threads version. Writing to .txt files and printing afterward fixed the issue.  
***3.Time Handling***  
Even with 20 million records, thread execution stayed within nanoseconds. Time formatting was updated to include nanoseconds without rounding for accurate total duration comparison.  
***4.Use of C Libraries***  
Limited knowledge of C led to issues using libraries like pthread.h and semaphore.h. Reading documentation for each helped solve these problems.  

## ***Conclusions***  

Proper implementation of synchronization methods in procesos and hilos allows practical demonstration of classroom concepts. Threads offer greater speed and resource efficiency but require careful implementation, as errors are more noticeable in execution time. Processes, while heavier on resources, can obscure poor design due to overlapping execution. Threads consume less memory, making them generally more efficient.  
