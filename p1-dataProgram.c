#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>

#define SHM_KEY 0x1234               // Clave única para la memoria compartida
#define TABLE_SIZE 500               // Tamaño de la tabla hash
#define MAX_TITLE_LEN 256            // Longitud máxima para títulos
#define MAX_RESULT_LEN 4096          // Tamaño máximo para resultados

// Estructura para compartir datos entre cliente y servidor
typedef struct {
    char title[MAX_TITLE_LEN];      // Título a buscar
    int year;                       // Año a buscar
    bool title_ready;               // Bandera: título listo
    bool year_ready;                // Bandera: año listo
    bool build_table_ready;         // Bandera: construir tabla hash
    bool result_ready;              // Bandera: resultado listo
    char result[MAX_RESULT_LEN];    // Resultado recibido del servidor
} SharedData;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s 1 (normal) o 2 (directo)\n", argv[0]);
        return EXIT_FAILURE;
    }

    int modo = atoi(argv[1]);

    // Crea o abre el segmento de memoria compartida System V
    int shm_id = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Se asocia al segmento de memoria compartida
    SharedData *shared_data = (SharedData *)shmat(shm_id, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Inicialización de las banderas
    shared_data->title_ready = false;
    shared_data->year_ready = false;
    shared_data->build_table_ready = false;
    shared_data->result_ready = false;

    if (modo == 2) {
        // MODO AUTOMÁTICO DIRECTO
        strcpy(shared_data->title, "The Matrix");
        shared_data->year = 1999;

        shared_data->title_ready = true;
        shared_data->year_ready = true;

        printf("Datos enviados al servidor (The Matrix, 1999). Esperando resultados...\n");

        while (!shared_data->result_ready) {
            usleep(100); // 0.1 ms
        }

        printf("\nResultado\n%s\n", shared_data->result);

    } else if (modo == 1) {
        // MODO NORMAL
        printf("Bienvenido \n1. Construir HashTable \n2. Realizar búsqueda por título y año\n3. SALIR\n");
        int opcion;
        if (scanf("%d", &opcion) != 1) {
            fprintf(stderr, "Opción inválida.\n");
            exit(EXIT_FAILURE);
        }

        getchar(); // Limpiar buffer

        if (opcion == 1) {
            printf("Construyendo tabla hash...\n");
            shared_data->build_table_ready = true;
            while (shared_data->build_table_ready) {
                usleep(1000);
            }
            printf("Tabla hash construida.\n");

        } else if (opcion == 2) {
            printf("Ingrese el título original a buscar: ");
            if (fgets(shared_data->title, MAX_TITLE_LEN, stdin) == NULL) {
                perror( "Error leyendo título.\n");
                exit(EXIT_FAILURE);
            }
            shared_data->title[strcspn(shared_data->title, "\n")] = '\0';
            shared_data->title_ready = true;

            printf("Ingrese el año a buscar: ");
            if (scanf("%d", &shared_data->year) != 1) {
                perror( "Año inválido.\n");
                exit(EXIT_FAILURE);
            }
            shared_data->year_ready = true;

            printf("Datos enviados al servidor. Esperando resultados...\n");

            while (!shared_data->result_ready) {
                usleep(100);
            }

            printf("\nResultado\n%s\n", shared_data->result);

        } else if (opcion == 3) {
            exit(EXIT_SUCCESS);
        } else {
            perror("Opción desconocida.\n");
            exit(EXIT_FAILURE);
        }

    } else {
        perror("Use 1 (normal) o 2 (directo).\n");
        exit(EXIT_FAILURE);
    }

    // Limpieza
    shmdt(shared_data);

    return EXIT_SUCCESS;
}
