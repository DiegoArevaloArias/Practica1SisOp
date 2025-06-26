/*
 * p2-dataProgram.c.c
 *
 * HashTable persistente en disco para índices de películas por año.
 * Limita el uso de RAM (<10 MB) leyendo/escribiendo directamente en archivo.
 *
 * Ejemplo de registro TSV (separado por tabuladores):
 * tconst      titleType   primaryTitle    originalTitle   isAdult startYear endYear runtimeMinutes genres
 * tt0055722   movie       Abasheshe       Abasheshe       0       1962      \N      120            Drama
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <strings.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define _POSIX_C_SOURCE 200809L

#define TABLE_SIZE 500
#define MAX_TITLE_LEN 256
#define SHM_NAME "/hash_table_shm"
#define MAX_RESULT_LEN 4096
#define SHM_KEY 0x1234

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

// Estructura para los nodos de la HashTable
typedef struct {
    off_t offset_original;              // Posición en el archivo TSV
    char originalTitle[MAX_TITLE_LEN];  // Título original
    int year;                           // Año
    off_t siguiente;                    // Offset al siguiente nodo en la lista
} NodoDisco;

// Función de hash (djb2)
static inline unsigned hash_string(const char *str) {
    unsigned hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; // Hash * 33 + c

    return hash % TABLE_SIZE;
}

// Crea una HashTable vacía en disco
void crear_tabla_hash_vacia(const char *nombre) {
    FILE *f = fopen(nombre, "wb");
    if (!f) {
        fprintf(stderr, "Error creando '%s': %s\n", nombre, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Inicializa la tabla con ceros (listas vacías)
    off_t cero = 0;
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (fwrite(&cero, sizeof(cero), 1, f) != 1) {
            fprintf(stderr, "Error inicializando tabla: %s\n", strerror(errno));
            fclose(f);
            exit(EXIT_FAILURE);
        }
    }
    fclose(f);
}

// Inserta un nodo en la HashTable en disco
bool insertar_en_disco(const char *archivo, const char *title, int year, off_t offset_tsv) {
    // Abre la HashTable
    FILE *f = fopen(archivo, "r+b");
    if (!f) {
        fprintf(stderr, "Error abriendo '%s' para insertar: %s\n", archivo, strerror(errno));
        return false;
    }

    // Calcula el bucket usando la función de hash
    unsigned h = hash_string(title);
    off_t cabeza;

    // Lee la cabeza de la lista correspondiente al bucket
    if (fseeko(f, h * sizeof(off_t), SEEK_SET) != 0 ||
        fread(&cabeza, sizeof(cabeza), 1, f) != 1) {
        fprintf(stderr, "Error leyendo cabeza de lista: %s\n", strerror(errno));
        fclose(f);
        return false;
    }

    // Mueve el puntero al final del archivo para insertar el nuevo nodo
    if (fseeko(f, 0, SEEK_END) != 0) {
        fprintf(stderr, "Error buscando fin de archivo: %s\n", strerror(errno));
        fclose(f);
        return false;
    }

    off_t nuevo_offset = ftello(f);
    NodoDisco nodo;
    strncpy(nodo.originalTitle, title, MAX_TITLE_LEN - 1);
    nodo.originalTitle[MAX_TITLE_LEN - 1] = '\0';
    nodo.year = year;
    nodo.offset_original = offset_tsv;
    nodo.siguiente = cabeza;  // El nuevo nodo apunta a la antigua cabeza

    // Escribe el nodo en disco
    if (fwrite(&nodo, sizeof(nodo), 1, f) != 1) {
        fprintf(stderr, "Error escribiendo nodo: %s\n", strerror(errno));
        fclose(f);
        return false;
    }

    // Actualiza la cabeza de la lista en el bucket
    if (fseeko(f, h * sizeof(off_t), SEEK_SET) != 0 ||
        fwrite(&nuevo_offset, sizeof(nuevo_offset), 1, f) != 1) {
        fprintf(stderr, "Error actualizando cabeza de lista: %s\n", strerror(errno));
        fclose(f);
        return false;
    }

    // Ciera el archivo
    fclose(f);
    return true;
}

// Construye la HashTable desde un archivo TSV
void construir_desde_tsv(const char *archivo_hash, const char *archivo_tsv) {
    crear_tabla_hash_vacia(archivo_hash);

    // Abre el DataSet
    FILE *f_tsv = fopen(archivo_tsv, "r");
    if (!f_tsv) {
        fprintf(stderr, "Error abriendo TSV '%s': %s\n", archivo_tsv, strerror(errno));
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t cap = 0;
    ssize_t len;

    // Descarta el encabezado del TSV
    if ((len = getline(&line, &cap, f_tsv)) == -1) {
        fprintf(stderr, "TSV vacío o error al leer encabezado\n");
        free(line);
        fclose(f_tsv);
        exit(EXIT_FAILURE);
    }

    // Procesa cada línea del TSV
    while ((len = getline(&line, &cap, f_tsv)) != -1) {
        off_t pos = ftello(f_tsv) - len;  // Posición de inicio de la línea

        // Extrae los campos relevantes (título original y año)
        char *saveptr = NULL;
        char *token;
        int field = 0;
        int year = 0;
        char originalTitle[MAX_TITLE_LEN] = {0};

        token = strtok_r(line, "\t\n", &saveptr);
        while (token) {
            switch (field) {
                // originalTitle
                case 3:
                    strncpy(originalTitle, token, MAX_TITLE_LEN - 1);
                    originalTitle[MAX_TITLE_LEN - 1] = '\0';
                    break;
                // startYear
                case 5:  
                    if (strcmp(token, "\\N") != 0) {
                        year = atoi(token);
                    }
                    break;
            }
            token = strtok_r(NULL, "\t\n", &saveptr);
            field++;
        }

        // Inserta en la tabla hash si los datos son válidos
        if (originalTitle[0] != '\0' && year > 1000 && year < 2100) {
            if (!insertar_en_disco(archivo_hash, originalTitle, year, pos)) {
                fprintf(stderr, "Fallo al insertar título '%s' (año %d) en offset %lld\n",
                        originalTitle, year, (long long)pos);
            }
        }
    }

    // Libera los recursos utlizados
    free(line);
    fclose(f_tsv);
    printf("Tabla hash construida en '%s'\n", archivo_hash);
}

// Busca en la tabla hash por título y año
void buscar_en_disco(const char *archivo_hash, const char *archivo_tsv, const char *title, SharedData *shared_data) {
    // Abre el DataSet y la HashTable
    FILE *f_hash = fopen(archivo_hash, "rb");
    FILE *f_tsv  = fopen(archivo_tsv,  "r");

    if (!f_hash || !f_tsv) {
        fprintf(stderr, "Error abriendo archivos para buscar: %s %s\n",
                !f_hash ? archivo_hash : "",
                !f_tsv  ? archivo_tsv  : "");
        if (f_hash) fclose(f_hash);
        if (f_tsv ) fclose(f_tsv);
        return;
    }

    // Calcula el bucket usando la función de hash
    unsigned h = hash_string(title);
    off_t pos_cabeza;

    // Lee la cabeza de la lista correspondiente al bucket
    if (fseeko(f_hash, h * sizeof(off_t), SEEK_SET) != 0 ||
        fread(&pos_cabeza, sizeof(pos_cabeza), 1, f_hash) != 1) {
        fprintf(stderr, "Error leyendo cabeza de bucket: %s\n", strerror(errno));
        fclose(f_hash);
        fclose(f_tsv);
        return;
    }

    NodoDisco nodo;
    char *line = NULL;
    size_t cap = 0;
    bool encontrado = false;

    // Recorre la lista enlazada del bucket
    for (off_t pos = pos_cabeza; pos != 0; ) {
        if (fseeko(f_hash, pos, SEEK_SET) != 0 ||
            fread(&nodo, sizeof(nodo), 1, f_hash) != 1) {
            fprintf(stderr, "Error leyendo nodo en %lld\n", (long long)pos);
            break;
        }

        // Compara el título con el ingresado por el usuario
        if (strcmp(nodo.originalTitle, title) == 0) {
            // Lee la línea correspondiente en el TSV
            if (fseeko(f_tsv, nodo.offset_original, SEEK_SET) == 0 &&
                getline(&line, &cap, f_tsv) != -1) {

                // Espera hasta que el cliente ingrese el año
                while (!shared_data->year_ready) {
                    usleep(1000);
                }

                // Compara el año con el ingreado por el usuario
                if (nodo.year == shared_data->year) {
                    // Guarda el resultado en la memoria compartida
                    snprintf(shared_data->result, MAX_RESULT_LEN, "\n%s", line);
                    shared_data->result_ready = true;

                    encontrado = true;

                    // Termina la búsqueda al encontrar la primera coincidencia
                    break;  
                }
            }
        }

        // Avanza al siguiente nodo
        pos = nodo.siguiente;  
    }

    // Si no se encontró ningún registro, envía "NA" como resultado
    if (!encontrado) {
        snprintf(shared_data->result, MAX_RESULT_LEN, "NA");
        shared_data->result_ready = true;
    }

    // Libera los recursos utilizados
    if (line) {
        free(line);
    }
    fclose(f_hash);
    fclose(f_tsv);
}


int main(void) {
    const char *ruta_tsv = "/home/acbel/Documentos/Sistemas Operativos/Practica1/title.basics.tsv";
    const char *ruta_hash = "hash_table_title.dat";

    // Crear o acceder a memoria compartida con System V
    int shm_id = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Asociar el segmento de memoria al proceso
    SharedData *shared_data = (SharedData *)shmat(shm_id, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Bucle principal del servidor
    while (!shared_data->result_ready) {
        if (shared_data->build_table_ready) {
            printf("Construyendo tabla hash desde TSV...\n");
            construir_desde_tsv(ruta_hash, ruta_tsv);
            shared_data->build_table_ready = false;
        }

        if (shared_data->title_ready) {
            buscar_en_disco(ruta_hash, ruta_tsv, shared_data->title, shared_data);
            shared_data->title_ready = false;
        }

        usleep(1000);  // Espera breve
    }

    // Desvincular la memoria del proceso
    shmdt(shared_data);

    // (Opcional) Eliminar la memoria del sistema
    shmctl(shm_id, IPC_RMID, NULL);

    return EXIT_SUCCESS;
}
