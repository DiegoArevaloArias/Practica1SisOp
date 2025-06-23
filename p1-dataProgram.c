#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>

// Definición de constantes
#define TABLE_SIZE 211               // Tamaño de la tabla hash
#define MAX_TITLE_LEN 256            // Longitud máxima para títulos
#define SHM_NAME "/hash_table_shm"   // Nombre de la memoria compartida
#define MAX_RESULT_LEN 4096          // Tamaño máximo para resultados

// Estructura para compartir datos entre cliente y servidor
typedef struct {
    char title[MAX_TITLE_LEN];      // Título a buscar (ingresado por el usuario)
    int year;                       // Año a buscar (ingresado por el usuario)
    bool title_ready;               // Bandera para indicar que el título ha sido ingresado por el usuario
    bool year_ready;                // Bandera para indicar que el año ha sido ingresado por el usuario
    bool build_table_ready;         // Bandera para solicitar la construcción de la tabla hash

    bool result_ready;              // Bandera para indicar que el resultado está listo
    char result[MAX_RESULT_LEN];    // Buffer para almacenar el resultado
} SharedData;

int main(void) {
    // Crea la memoria compartida
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // Configura el tamaño de la memoria compartida
    if (ftruncate(shm_fd, sizeof(SharedData))) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    // Mapea la memoria compartida al espacio de direcciones del proceso
    SharedData *shared_data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // Inicializa la memoria compartida
    shared_data->title_ready = false;  
    shared_data->year_ready = false;  

    // Menú principal de interacción con el usuario
    printf("Bienvenido \n1. Construir HashTable \n2. Realizar búsqueda por título y año\n3. SALIR\n");
    int opcion;
    if (scanf("%d", &opcion) != 1) {//No hace nada para salir
        fprintf(stderr, "Opción inválida.\n");
        exit(EXIT_FAILURE);
    }

    // Limpia el buffer de entrada después de scanf
    getchar(); 

    // Procesar la opción seleccionada

    // Opción para construir la tabla hash
    if (opcion == 1) {
        printf("Construyendo tabla hash...\n");
        
        // Indica al servidor que construya la tabla
        shared_data->build_table_ready = true;  
        
        // Espera a que el servidor complete la construcción
        while (shared_data->build_table_ready) {
            // Espera 1 segundo entre verificaciones
            sleep(1); 
        }
        printf("Tabla hash construida.\n");
    // Opción para buscar por título y año
    } else if (opcion == 2) {

        // Solicita y lee el título
        printf("Ingrese el título original a buscar: ");
        if (fgets(shared_data->title, MAX_TITLE_LEN, stdin) == NULL) {
            fprintf(stderr, "Error leyendo título.\n");
            exit(EXIT_FAILURE);
        }
        // Elimina el salto de línea del final
        shared_data->title[strcspn(shared_data->title, "\n")] = '\0';

        // Marca título como listo al servidor
        shared_data->title_ready = true; 

        // Solicita y lee el año
        printf("Ingrese el año a buscar: ");
        if (scanf("%d", &shared_data->year) != 1) {
            fprintf(stderr, "Año inválido.\n");
            exit(EXIT_FAILURE);
        }
        // Marca año como listo al servidor
        shared_data->year_ready = true;  

        printf("Datos enviados al servidor. Esperando resultados...\n");
    } else if (opcion == 3) {
        exit(EXIT_SUCCESS);
    
    } else {
        // Opción no válida
        fprintf(stderr, "Opción desconocida.\n");
        exit(EXIT_FAILURE);
    }

    // Espera hasta que el servidor marque el resultado como listo
    while (!shared_data->result_ready) {
        usleep(10000);
    }

    // Muestra el resultado obtenido
    printf("\n=== RESULTADO ===\n%s\n", shared_data->result);

    // Libera los recursos utilizados
    munmap(shared_data, sizeof(SharedData));  // Desmapear memoria compartida
    close(shm_fd);                            // Cerrar descriptor de archivo

    return EXIT_SUCCESS;
}