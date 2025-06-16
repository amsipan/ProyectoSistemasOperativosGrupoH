# ***Proyecto Sistemas Operativos***

**Ejecucion de múltiples tareas con procesos y con hilos** 



## ***Autores*** 

- Jibaja Sebastian
- Mosquera Karen
- Oviedo Jorge
- Tamayo Oscar

## ***Requerimientos***

Para poder clonar y ejecutar este repositorio se recomienda el uso de ```Ubuntu server``` instalado en una maquina virtual como ```Oracle Virtual Machine```, con las sigueintes caracteristicas mínimas:  
- 2 nucleos.
- 4gb de RAM.
- 256gb de Memoria.  
Ademas se debe instalar el compilador para archivos ```.c``` llamado ```GCC, o GNU Compiler Collection```, para lo cual se debe ejecutar el siguiente comando dentro de la terminal de ```UbuntuServer```.
```bash
sudo apt update
sudo apt install build-essential
```


    
## ***Compilacion***

Los archivos ubicados en ```src``` son únicamente los código fuente, por lo cual se necesita la ejecucion de los siguientes comandos.  

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

## ***Ejecucion***
Tras la previa compilación de los archivos, se tienen distintas formas de ejecutar los distintos archivos, por facilidad se nombrara a los ejecutables como:  

```procesos.c -> procesos_promedio```  
```hilos.c -> hilos_promedio```  
```run.c -> ejecutable```  

Por lo cual, las distintas formas de ejecucion se deberan realizar desde la terminal y seguiran los siguientes comandos:  
- *Únicamente hilos o únicamente procesos, con ejecucion única de cada archivo*
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
- Impresión en consola la cual muestre los resultados, esta impresion seguira la siguiente sintaxis para hilos y procesos, únicamente cambiando el titulo principal.
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
- Archivo ```.txt``` con los resultados obtenidos de cada ejecución, estos tendran los nombres de ```resultados_hilos.txt``` y ```resultados_procesos```, los resultados se sobreescribiran con cada ejecución y tendran la misma estructura de la sección anterior..

- Si se ejecutan ambos archivos mediante el uso del ejecutable, se imprimira los descrito en la primera sección para proces e hilos y además se agrega la siguiente salida a la impresión.
```bash
=== Comparativa de tiempos ===
Procesos: [Duración total de procesos]
Hilos: [Duración total de hilos]

[Procesos o hilos] fue más rapido.
```




## ***Analisis de Problemas***
Dentro de la codificación de los distintos archivos, se presentaron distintos problemas, los cuales se describiran en esta sección:  
***1. Manejo de la memoria compartida.***  
Esto debido al control de los semaforos y mutex respectivamente, los cuales no se implementaban de manera adecuada y generaban que procesos sea notablemente más veloz en tiempos de ejecución con respecto a hilos, lo cual no tiene sentido.  
La solución que se consiguio fue manejar las varaibles compartidas como un arreglo, el cual se manejaba con un único semaforo o mutex segun el caso, este arreglo se volvio la variable de memoria compartida controlada con los metodos de sincronización adecuados.   
***2. Impresiones durante la ejecución***   
Tras el arreglo correspondiente en semaforos y mutex se presento el problema de la impresión de resultados en consola, lo cual dentro de hilos generaba un aumento significativo en la duración total del tiempo de ejecucíon, este problema se soluciono con la escritura en archivos ```.txt``` externos, y posteriormente imprimir los resultados desde estos.  
***3. Manejo del tiempo***  
Para la implementación se usaron ```20000000``` notas para ejecución, pero aun con esta medida los tiempos en hilos no llegaban a segundos enteros, quedando en nanosegundos e incluso tiempos menores, lo cual sucitaba un inconveniente al momento de la comparación de velocidades entre procesos e hilos.  
Para lo cual se adecuo el formato del tiempo para que incluyera nanosegundos sin redondeos, lo cual arreglaba el poblema para el calculo de la duración total.  
***4. Uso de librerias en C***  
Debido al poco conociemiento sobre el lenguaje ```C``` se presentaron diversos inconvenientes con el manejo de las librerias como ```phtread.h``` o ```semaphore.h``` para la creación de los hilos y el uso de semaforos respectivamente.  Por lo cual se busco la documentación de cada una de ellas para identificar sus funciones y parametros requeridos en cada una de ellas.
## ***Concluciones***
Tras la correcta implementación de los archivos ```prcesos``` e ```hilos``` se puede concluir que si se manejan de forma adecuada los respectivos metodos de sincronización en cada uno de ellos se puede observar de froma practica lo aprendido durante las horas clase, además, el control y velocidad que ofrecen los hilos para la ejecución facilita la metodologia de trabajo y su correspondiente implementación.  
Algunas diferencias muy notables y remarcables son en su mayoria en hilos, puesto que en caso de existir una mala implementación se puede notar de forma muy rapida al visualizar los tiempos de ejecución, lo cual no sucede del todo en procesos, puesto que estos pueden solaparse y ejecutar en conjunto, lo cual lleva a una diferencia mayor, otra diferencia esta presente en el uso de recursos para su ejecución, los procesos ocupan una mayor cantidad de recursos con respecto a hilos, lo que tambien afecta a la velocidad de ejecución, este mayor uso de recursos ademas repercute al espacio de memoria, puesto que procesos ocupara mas de esta con respecto a hilos.