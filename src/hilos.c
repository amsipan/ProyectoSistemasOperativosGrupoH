#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>   /* sysconf */
#include <string.h>

#define TOTAL_NOTAS 20000000   // Total de notas a procesar
#define MAX_NOTA    40         // Nota máxima posible

/* Resultado individual de cada hilo */
typedef struct {
    double promedio;      // Promedio de notas del grupo
    int reprobados;       // Cantidad de reprobados (<18)
    int aprobado_bajo;    // Cantidad de aprobados bajos (18-27.99)
    int aprobado_alto;    // Cantidad de aprobados altos (28-40)
    double tiempo;        // Tiempo de ejecución del hilo en segundos
} resultado_hilo;

/* Datos pasados al hilo */
typedef struct {
    int id;               // Identificador del hilo (0..n-1)
    int *notas;           // Puntero al arreglo global de notas
    long inicio;          // Índice inicial de notas a procesar
    long cantidad;        // Cuántas notas procesa este hilo
    resultado_hilo *resultado;  // Puntero a su celda resultado
} dato_hilo;

// Arreglo global compartido para totales: [reprobados, aprobado_bajo, aprobado_alto]
int resumen_global[3] = {0, 0, 0};
pthread_mutex_t resumen_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Función que ejecuta cada hilo.
 * arg: puntero a dato_hilo con los datos de trabajo y resultado.
 * Calcula el promedio y clasificaciones, mide su tiempo y actualiza los totales globales.
 */
static void *procesar(void *arg)
{
    dato_hilo *info = (dato_hilo *)arg; // Conversión de void* a dato_hilo*
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0); // Marca de tiempo inicial del hilo
    long suma = 0;
    int rep = 0, ab = 0, aa = 0;
    // Procesa su bloque de notas
    for (long i = info->inicio; i < info->inicio + info->cantidad; ++i) {
        int n = info->notas[i];
        suma += n;
        if (n < 18) rep++;
        else if (n < 28) ab++;
        else aa++;
    }
    clock_gettime(CLOCK_MONOTONIC, &t1); // Marca de tiempo final del hilo
    info->resultado->promedio       = (double)suma / info->cantidad;
    info->resultado->reprobados     = rep;
    info->resultado->aprobado_bajo  = ab;
    info->resultado->aprobado_alto  = aa;
    info->resultado->tiempo = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
    // Sección crítica: actualiza los totales globales
    pthread_mutex_lock(&resumen_mutex);
    resumen_global[0] += rep;
    resumen_global[1] += ab;
    resumen_global[2] += aa;
    pthread_mutex_unlock(&resumen_mutex);
    return NULL;
}

/*
 * Muestra y guarda los resultados de cada grupo/hilo.
 * filename: nombre del archivo de salida.
 * res: arreglo de resultados de cada hilo.
 * n_hilos: cantidad de hilos/grupos.
 */
void mostrar_y_guardar_resultados(const char *filename, resultado_hilo *res, int n_hilos, double tiempo_total, time_t inicio, time_t fin, struct timespec t0, struct timespec t1) {
    int rewrite = 0;
    FILE *revision = fopen(filename, "r");
    if (revision) {
        int lines = 0;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), revision)) lines++;
        if (lines > 12) rewrite = 1;
        fclose(revision);
    }
    FILE *escritura = fopen(filename, rewrite ? "w" : "a");
    if (!escritura) { perror("fopen"); exit(EXIT_FAILURE); }
    for (int i = 0; i < n_hilos; ++i) {
        char letra = 'A' + i;
        fprintf(escritura,
                "Grupo %c | Promedio: %.2f | Reprobados: %d | Aprobados (18-27.99): %d | Aprobados (28-40): %d | Tiempo: %.6f s\n",
                letra, res[i].promedio, res[i].reprobados,
                res[i].aprobado_bajo, res[i].aprobado_alto, res[i].tiempo);
    }
    // Escribe el tiempo de inicio y fin en formato legible con nanosegundos
    char buf_inicio[64], buf_fin[64];
    strftime(buf_inicio, sizeof(buf_inicio), "%a %b %d %H:%M:%S", localtime(&inicio));
    strftime(buf_fin, sizeof(buf_fin), "%a %b %d %H:%M:%S", localtime(&fin));
    fprintf(escritura, "Inicio: %s:%09ld %d\n", buf_inicio, t0.tv_nsec, 1900 + localtime(&inicio)->tm_year);
    fprintf(escritura, "Fin: %s:%09ld %d\n", buf_fin, t1.tv_nsec, 1900 + localtime(&fin)->tm_year);
    fprintf(escritura, "Duración total: %.6f segundos\n", tiempo_total);
    fflush(escritura);
    fclose(escritura);
}

int main(void)
{
    /* Hilos a utilizar = núcleos lógicos */
    int n_hilos = 8;
    if (n_hilos < 1) n_hilos = 8; /* respaldo */

    long notas_por_hilo = TOTAL_NOTAS / n_hilos; // Notas por hilo
    long resto          = TOTAL_NOTAS % n_hilos; // Resto para el último hilo

    /* Generar notas aleatorias */
    int *notas = malloc(sizeof(int) * TOTAL_NOTAS); // Puntero a arreglo dinámico
    if (!notas) { perror("malloc"); return EXIT_FAILURE; }
    srand((unsigned)time(NULL));
    for (long i = 0; i < TOTAL_NOTAS; ++i)
        notas[i] = rand() % (MAX_NOTA + 1);

    pthread_t   *hilos = malloc(sizeof(pthread_t) * n_hilos); // Puntero a arreglo de hilos
    dato_hilo     *dato_por_hilo = malloc(sizeof(dato_hilo)  * n_hilos); // Puntero a datos de cada hilo
    resultado_hilo *res   = calloc(n_hilos, sizeof(resultado_hilo)); // Puntero a resultados

    /* Medición de tiempo total */
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0); // Marca de tiempo inicial
    time_t tiempo_inicio = time(NULL);

    long idx = 0;
    for (int i = 0; i < n_hilos; ++i) {
        long cant = notas_por_hilo + (i == n_hilos - 1 ? resto : 0); // Cantidad para este hilo
        // Inicializa la estructura de datos para el hilo
        dato_por_hilo[i] = (dato_hilo){ .id = i, .notas = notas,
                              .inicio = idx, .cantidad = cant,
                              .resultado = &res[i] };
        // pthread_create: crea un hilo
        // &hilos[i]: puntero al identificador del hilo
        // NULL: atributos por defecto
        // procesar: función que ejecuta el hilo
        // &dato_por_hilo[i]: puntero a los datos del hilo
        if (pthread_create(&hilos[i], NULL, procesar, &dato_por_hilo[i]) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
        idx += cant;
    }

    // Espera a que todos los hilos terminen
    for (int i = 0; i < n_hilos; ++i)
        pthread_join(hilos[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &t1); // Marca de tiempo final

    double duracion_total = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
    time_t tiempo_fin = time(NULL);
    // Escribe los resultados ANTES de leerlos para imprimir
    mostrar_y_guardar_resultados("resultados_hilos.txt", res, n_hilos, duracion_total, tiempo_inicio, tiempo_fin, t0, t1);
    printf("\n=== HILOS ===\n");
    // Imprime solo las primeras 8 líneas del archivo de resultados
    FILE *escritura = fopen("resultados_hilos.txt", "r");
    if (escritura) {
        char line[256];
        int count = 0;
        while (fgets(line, sizeof line, escritura) && count < 8) {
            printf("%s", line);
            count++;
        }
        fclose(escritura);
    }
    printf("\n=== Totales Globales (con mutex) ===\n");
    printf("Reprobados: %d\n", resumen_global[0]);
    printf("Aprobados (18-27.99): %d\n", resumen_global[1]);
    printf("Aprobados (28-40): %d\n", resumen_global[2]);
    printf("\n=== Resumen (HILOS) ===\n");
    char buf_inicio[64], buf_fin[64];
    strftime(buf_inicio, sizeof(buf_inicio), "%a %b %d %H:%M:%S", localtime(&tiempo_inicio));
    strftime(buf_fin, sizeof(buf_fin), "%a %b %d %H:%M:%S", localtime(&tiempo_fin));
    printf("Inicio: %s:%09ld %d\n", buf_inicio, t0.tv_nsec, 1900 + localtime(&tiempo_inicio)->tm_year);
    printf("Fin: %s:%09ld %d\n", buf_fin, t1.tv_nsec, 1900 + localtime(&tiempo_fin)->tm_year);
    printf("Duración total: %.6f segundos\n", duracion_total);
    pthread_mutex_destroy(&resumen_mutex); // Libera el mutex

    free(notas);  // Libera memoria dinámica
    free(hilos);
    free(dato_por_hilo);
    free(res);
    return EXIT_SUCCESS;
}

