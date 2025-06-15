#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <float.h>

/*
 * Función lanzar: ejecuta un comando externo usando system().
 * cmd: cadena con el comando a ejecutar (ej: "./procesos_promedio")
 * Imprime advertencias si el comando termina con error.
 */
static void lanzar(const char *cmd)
{
    printf("\n===== Lanzando %s =====\n", cmd);
    int status = system(cmd); // Ejecuta el comando externo
    if (status == -1) {
        perror("system");
    } else if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        fprintf(stderr, "\u26A0\uFE0F  %s terminó con código %d\n", cmd, WEXITSTATUS(status));
    }
}

/*
 * Función extraer_tiempo: busca y extrae el tiempo total desde un archivo de resultados.
 * filename: nombre del archivo a leer (ej: "resultados_procesos.txt")
 * Retorna el tiempo en segundos (double) o -1 si no se encuentra.
 */
static double extraer_tiempo(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;
    char line[256];
    double tiempo = -1;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "Duración total")) {
            sscanf(line, "%*[^:]: %lf", &tiempo); // Extrae el número después de ':'
            break;
        }
    }
    fclose(f);
    return tiempo;
}

int main(int argc, char *argv[])
{
    // argc: número de argumentos de línea de comandos
    // argv: arreglo de cadenas con los argumentos
    if (argc < 2) {
        fprintf(stderr, "Uso: %s [procesos|hilos|ambos]\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Lógica de selección según argumento
    if (strcmp(argv[1], "procesos") == 0)
        lanzar("./procesos_promedio");
    else if (strcmp(argv[1], "hilos") == 0)
        lanzar("./hilos_promedio");
    else if (strcmp(argv[1], "ambos") == 0) {
        lanzar("./procesos_promedio");
        lanzar("./hilos_promedio");
        // Extrae y compara los tiempos de ambos resultados
        double t_proc = extraer_tiempo("resultados_procesos.txt");
        double t_hilo = extraer_tiempo("resultados_hilos.txt");
        if (t_proc > 0 && t_hilo > 0) {
            printf("\n=== Comparativa de tiempos ===\n");
            printf("Procesos: %.6f s\n", t_proc);
            printf("Hilos   : %.6f s\n", t_hilo);
            if (t_proc < t_hilo)
                printf("\nProcesos fue más rápido.\n");
            else if (t_hilo < t_proc)
                printf("\nHilos fue más rápido.\n");
            else
                printf("\nAmbos tuvieron el mismo tiempo.\n");
        } else {
            printf("\nNo se pudo extraer el tiempo de ambos resultados.\n");
        }
    } else {
        fprintf(stderr, "\u274C  Opción no reconocida: %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
