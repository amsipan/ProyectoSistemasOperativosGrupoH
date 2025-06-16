#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <string.h>
#include <sys/mman.h>

/* Parámetros de escalado */
#define TOTAL_NOTAS      20000000            // Total de notas a procesar
#define N_GRUPOS         8                   // Número de procesos hijos
#define NOTAS_POR_GRUPO (TOTAL_NOTAS / N_GRUPOS) // Notas por cada hijo
#define MAX_NOTA         40                  // Nota máxima posible
#define SEM_NAME         "/file_sem"        // Nombre del semáforo POSIX

// Estructura para almacenar el resultado de cada grupo
typedef struct {
    char letra;         // Letra identificadora del grupo (A, B, ...)
    double promedio;    // Promedio de notas del grupo
    int reprobados;     // Cantidad de reprobados (<18)
    int aprobado_bajo;  // Cantidad de aprobados bajos (18-27.99)
    int aprobado_alto;  // Cantidad de aprobados altos (28-40)
    double tiempo;      // Tiempo de ejecución del grupo en segundos
} resultado_por_grupo;

/*
 * Función que escribe el resultado de un grupo en el archivo de salida.
 * filename: nombre del archivo de salida.
 * r: puntero a la estructura resultado_por_grupo con los datos del grupo.
 * Si el archivo tiene 8 o más líneas, lo reescribe; si no, agrega al final.
 */
static void escribir_resultado(const char *filename, const resultado_por_grupo *resultado)
{
    int rewrite = 0;
    // Solo reescribe si el archivo tiene más de 11 líneas
    FILE *archivo = fopen(filename, "r");
    if (archivo) {
        int lines = 0;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), archivo)) lines++;
        if (lines > 12) rewrite = 1;
        fclose(archivo);
    }
    archivo = fopen(filename, rewrite ? "w" : "a");
    if (archivo) {
        fprintf(archivo,
                "Grupo %c | Promedio: %.2f | Reprobados: %d | Aprobados (18-27.99): %d | Aprobados (28-40): %d | Tiempo: %.6f s\n",
                resultado->letra, resultado->promedio, resultado->reprobados, resultado->aprobado_bajo, resultado->aprobado_alto, resultado->tiempo);
        fflush(archivo);
        fclose(archivo);
    }
}

// Función para escribir el tiempo total y marcas de tiempo al final del archivo de resultados
static void escribir_tiempos_finales(const char *filename, double tiempo_total, time_t inicio, time_t fin, struct timespec t0, struct timespec t1) {
    FILE *archivo = fopen(filename, "a");
    if (archivo) {
        char buf_inicio[64], buf_fin[64];
        strftime(buf_inicio, sizeof(buf_inicio), "%a %b %d %H:%M:%S", localtime(&inicio));
        strftime(buf_fin, sizeof(buf_fin), "%a %b %d %H:%M:%S", localtime(&fin));
        fprintf(archivo, "Inicio: %s:%09ld %d\n", buf_inicio, t0.tv_nsec, 1900 + localtime(&inicio)->tm_year);
        fprintf(archivo, "Fin: %s:%09ld %d\n", buf_fin, t1.tv_nsec, 1900 + localtime(&fin)->tm_year);
        fprintf(archivo, "Duración total: %.6f segundos\n", tiempo_total);
        fflush(archivo);
        fclose(archivo);
    }
}

int main(void)
{
    struct timespec t0, t1;
    time_t tiempo_inicio = time(NULL); // Marca de tiempo de inicio (segundos desde epoch)
    clock_gettime(CLOCK_MONOTONIC, &t0); // Marca de tiempo alta resolución

    // Reserva memoria dinámica para todas las notas (puntero)
    int *notas = malloc(sizeof(int) * TOTAL_NOTAS); // Puntero a arreglo dinámico
    if (!notas) { perror("malloc"); return EXIT_FAILURE; }

    // Inicializa el arreglo de notas con valores aleatorios entre 0 y MAX_NOTA
    srand((unsigned)time(NULL));
    for (int i = 0; i < TOTAL_NOTAS; ++i)
        notas[i] = rand() % (MAX_NOTA + 1);

    // Crea o abre un semáforo POSIX para sincronizar acceso a recursos compartidos
    sem_t *semaforo = sem_open(SEM_NAME, O_CREAT, 0666, 1); // SEM_NAME: nombre, O_CREAT: crear si no existe, permisos 0666, valor inicial 1
    if (semaforo == SEM_FAILED) { perror("sem_open"); return EXIT_FAILURE; }

    // Crea memoria compartida para acumular los totales globales de todos los grupos
    int memoria_compartida = shm_open("/resumen_global", O_CREAT | O_RDWR, 0666); // /resumen_global: nombre, O_CREAT|O_RDWR: crear y leer/escribir, permisos 0666
    ftruncate(memoria_compartida, 3 * sizeof(int)); // Ajusta el tamaño de la memoria compartida
    int *resumen_global = mmap(0, 3 * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, memoria_compartida, 0); // Mapea la memoria compartida
    resumen_global[0] = 0; // reprobados
    resumen_global[1] = 0; // aprobado_bajo
    resumen_global[2] = 0; // aprobado_alto


    // Crea N_GRUPOS procesos hijos
    for (int g = 0; g < N_GRUPOS; ++g) {
        pid_t pid = fork(); // Crea un nuevo proceso hijo
        if (pid < 0) { perror("fork"); return EXIT_FAILURE; }
        else if (pid == 0) {
            // Cada hijo procesa su bloque de notas
            int inicio = g * NOTAS_POR_GRUPO; // Índice inicial de notas para este grupo
            int fin    = inicio + NOTAS_POR_GRUPO; // Índice final (no inclusivo)
            long suma = 0;
            int rep = 0, ab = 0, aa = 0;
            struct timespec t0g, t1g;
            clock_gettime(CLOCK_MONOTONIC, &t0g); // Marca de tiempo inicial del grupo
            for (int i = inicio; i < fin; ++i) {
                int n = notas[i];
                suma += n;
                if (n < 18) rep++;
                else if (n < 28) ab++;
                else aa++;
            }
            clock_gettime(CLOCK_MONOTONIC, &t1g); // Marca de tiempo final del grupo
            double tiempo = (t1g.tv_sec - t0g.tv_sec) + (t1g.tv_nsec - t0g.tv_nsec) / 1e9;
            // Llena la estructura resultado_por_grupo con los datos del grupo
            resultado_por_grupo resultado = { .letra = 'A' + g,
                              .promedio = (double)suma / NOTAS_POR_GRUPO,
                              .reprobados = rep,
                              .aprobado_bajo = ab,
                              .aprobado_alto = aa,
                              .tiempo = tiempo };

            // Sección crítica: actualiza los totales globales en memoria compartida
            sem_wait(semaforo); // Espera el semáforo antes de modificar el recurso compartido
            resumen_global[0] += rep;
            resumen_global[1] += ab;
            resumen_global[2] += aa;
            sem_post(semaforo); // Libera el semáforo

            // Sección crítica: escribe el resultado del grupo en el archivo
            sem_wait(semaforo);
            escribir_resultado("resultados_procesos.txt", &resultado);
            sem_post(semaforo);

            // Libera recursos antes de terminar el hijo
            sem_close(semaforo); // Cierra el semáforo
            munmap(resumen_global, 3 * sizeof(int)); // Libera la memoria compartida
            close(memoria_compartida); // Cierra el descriptor de la memoria compartida
            free(notas); // Libera la memoria dinámica
            _exit(0); // Termina el proceso hijo
        }
    }
    
    // Espera a que todos los hijos terminen y luego lee el archivo de resultados
    while (wait(NULL) > 0); // wait(NULL): espera a que terminen los hijos

    // Escribe los tiempos de inicio y fin en el archivo de resultados
    clock_gettime(CLOCK_MONOTONIC, &t1); // Marca de tiempo final total
    double duracion_total = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
    time_t tiempo_fin = time(NULL); // Marca de tiempo de fin
    escribir_tiempos_finales("resultados_procesos.txt", duracion_total, tiempo_inicio, tiempo_fin, t0, t1);

    // Imprime solo las primeras 8 líneas del archivo de resultados
    FILE *out = fopen("resultados_hilos.txt", "r");
    if (out) {
        char line[256];
        int count = 0;
        while (fgets(line, sizeof line, out) && count < 8) {
            printf("%s", line);
            count++;
        }
        fclose(out);
    }

    // Imprime los totales globales acumulados en memoria compartida
    printf("\n=== Totales Globales ===\n");
    printf("Reprobados: %d\n", resumen_global[0]);
    printf("Aprobados (18-27.99): %d\n", resumen_global[1]);
    printf("Aprobados (28-40): %d\n", resumen_global[2]);
    munmap(resumen_global, 3 * sizeof(int)); // Libera memoria compartida
    close(memoria_compartida); // Cierra descriptor de memoria compartida
    shm_unlink("/resumen_global"); // Elimina el objeto de memoria compartida

    clock_gettime(CLOCK_MONOTONIC, &t1); // Marca de tiempo final total
    duracion_total = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
    tiempo_fin = time(NULL); // Marca de tiempo de fin

    // Imprime el tiempo total de ejecución
    printf("\n=== Resumen (PROCESOS) ===\n");
    char buf_inicio[64], buf_fin[64];
    strftime(buf_inicio, sizeof(buf_inicio), "%a %b %d %H:%M:%S", localtime(&tiempo_inicio));
    strftime(buf_fin, sizeof(buf_fin), "%a %b %d %H:%M:%S", localtime(&tiempo_fin));
    printf("Inicio: %s:%09ld %d\n", buf_inicio, t0.tv_nsec, 1900 + localtime(&tiempo_inicio)->tm_year);
    printf("Fin: %s:%09ld %d\n", buf_fin, t1.tv_nsec, 1900 + localtime(&tiempo_fin)->tm_year);
    printf("Duración total: %.6f segundos\n", duracion_total);

    sem_close(semaforo);      // Cierra el semáforo
    sem_unlink(SEM_NAME);// Elimina el semáforo del sistema
    free(notas);         // Libera la memoria dinámica
    return EXIT_SUCCESS;
}
