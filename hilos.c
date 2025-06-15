/* hilos_promedio.c – versión optimizada CON sincronización explícita
 * Calcula promedios sobre 20 000 000 de notas usando tantos hilos como
 * núcleos lógicos disponibles (‐pthreads). Cada hilo trabaja en su bloque,
 * guarda el resultado en una estructura privada y el hilo principal emite la
 * salida al final. Se usan mutex para proteger recursos compartidos.
 *
 * Compilación recomendada:
 *   gcc hilos.c -o hilos_promedio -O3 -march=native -flto -pthread -Wall
 */
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
} resultado_t;

/* Datos pasados al hilo */
typedef struct {
    int id;               // Identificador del hilo (0..n-1)
    int *notas;           // Puntero al arreglo global de notas
    long inicio;          // Índice inicial de notas a procesar
    long cantidad;        // Cuántas notas procesa este hilo
    resultado_t *salida;  // Puntero a su celda resultado
} tdata_t;

// Arreglo global compartido para totales: [reprobados, aprobado_bajo, aprobado_alto]
int resumen_global[3] = {0, 0, 0};
pthread_mutex_t resumen_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Función que ejecuta cada hilo.
 * arg: puntero a tdata_t con los datos de trabajo y resultado.
 * Calcula el promedio y clasificaciones, mide su tiempo y actualiza los totales globales.
 */
static void *procesar(void *arg)
{
    tdata_t *d = (tdata_t *)arg; // Conversión de void* a tdata_t*
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0); // Marca de tiempo inicial del hilo
    long suma = 0;
    int rep = 0, ab = 0, aa = 0;
    // Procesa su bloque de notas
    for (long i = d->inicio; i < d->inicio + d->cantidad; ++i) {
        int n = d->notas[i];
        suma += n;
        if (n < 18) rep++;
        else if (n < 28) ab++;
        else aa++;
    }
    clock_gettime(CLOCK_MONOTONIC, &t1); // Marca de tiempo final del hilo
    d->salida->promedio       = (double)suma / d->cantidad;
    d->salida->reprobados     = rep;
    d->salida->aprobado_bajo  = ab;
    d->salida->aprobado_alto  = aa;
    d->salida->tiempo = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
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
void mostrar_y_guardar_resultados(const char *filename, resultado_t *res, int n_hilos, double tiempo_total, time_t inicio, time_t fin, struct timespec t0, struct timespec t1) {
    int rewrite = 0;
    FILE *fcheck = fopen(filename, "r");
    if (fcheck) {
        int lines = 0;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fcheck)) lines++;
        if (lines > 12) rewrite = 1;
        fclose(fcheck);
    }
    FILE *out = fopen(filename, rewrite ? "w" : "a");
    if (!out) { perror("fopen"); exit(EXIT_FAILURE); }
    for (int i = 0; i < n_hilos; ++i) {
        char letra = 'A' + i;
        fprintf(out,
                "Grupo %c | Promedio: %.2f | Reprobados: %d | Aprobados (18-27.99): %d | Aprobados (28-40): %d | Tiempo: %.6f s\n",
                letra, res[i].promedio, res[i].reprobados,
                res[i].aprobado_bajo, res[i].aprobado_alto, res[i].tiempo);
    }
    // Escribe el tiempo de inicio y fin en formato legible con nanosegundos
    char buf_inicio[64], buf_fin[64];
    strftime(buf_inicio, sizeof(buf_inicio), "%a %b %d %H:%M:%S", localtime(&inicio));
    strftime(buf_fin, sizeof(buf_fin), "%a %b %d %H:%M:%S", localtime(&fin));
    fprintf(out, "Inicio: %s:%09ld %d\n", buf_inicio, t0.tv_nsec, 1900 + localtime(&inicio)->tm_year);
    fprintf(out, "Fin: %s:%09ld %d\n", buf_fin, t1.tv_nsec, 1900 + localtime(&fin)->tm_year);
    fprintf(out, "Duración total: %.6f segundos\n", tiempo_total);
    fflush(out);
    fclose(out);
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
    tdata_t     *datos = malloc(sizeof(tdata_t)  * n_hilos); // Puntero a datos de cada hilo
    resultado_t *res   = calloc(n_hilos, sizeof(resultado_t)); // Puntero a resultados

    /* Medición de tiempo total */
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0); // Marca de tiempo inicial
    time_t tiempo_inicio = time(NULL);

    long idx = 0;
    for (int i = 0; i < n_hilos; ++i) {
        long cant = notas_por_hilo + (i == n_hilos - 1 ? resto : 0); // Cantidad para este hilo
        // Inicializa la estructura de datos para el hilo
        datos[i] = (tdata_t){ .id = i, .notas = notas,
                              .inicio = idx, .cantidad = cant,
                              .salida = &res[i] };
        // pthread_create: crea un hilo
        // &hilos[i]: puntero al identificador del hilo
        // NULL: atributos por defecto
        // procesar: función que ejecuta el hilo
        // &datos[i]: puntero a los datos del hilo
        if (pthread_create(&hilos[i], NULL, procesar, &datos[i]) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
        idx += cant;
    }

    // Espera a que todos los hilos terminen
    for (int i = 0; i < n_hilos; ++i)
        pthread_join(hilos[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &t1); // Marca de tiempo final

    double dt = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
    time_t tiempo_fin = time(NULL);
    // Escribe los resultados ANTES de leerlos para imprimir
    mostrar_y_guardar_resultados("resultados_hilos.txt", res, n_hilos, dt, tiempo_inicio, tiempo_fin, t0, t1);
    printf("\n=== HILOS ===\n");
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
    printf("Duración total: %.6f segundos\n", dt);
    pthread_mutex_destroy(&resumen_mutex); // Libera el mutex

    free(notas);  // Libera memoria dinámica
    free(hilos);
    free(datos);
    free(res);
    return EXIT_SUCCESS;
}

