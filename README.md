# Practica1SisOp

Práctica 1: Búsqueda de Películas en Base de Datos

Este proyecto, desarrollado para la asignatura de Sistemas Operativos 2025-1, implementa un sistema de búsqueda de películas utilizando dos procesos interconectados: un servidor y un cliente.


## Integrantes:
- Diego Alejandro Arevalo Arias
- Angel David Beltran Garcia


## Descripcion:

El objetivo principal es permitir la búsqueda eficiente de títulos de películas en una base de datos. Para ello, se emplean dos procesos principales:

- Proceso Servidor:
   - Crea y gestiona una HashTable indexada por el título de la película. Cada entrada de la HashTabel almacena nodos con el índice en la base de datos, año, título y un puntero al siguiente nodo (en caso de colisiones).
   - Si la tabla hash ya existe, el servidor recibe el título y el año de la película del cliente.
   - Busca el primer nodo que coincida con el título y luego filtra por el año para encontrar la entrada exacta en la base de datos, enviando el campo completo al cliente.
- Proceso Cliente:
   - Solicita al usuario el título y el año de la película que desea buscar.
   - Envía esta información al proceso servidor.
   - Recibe el campo completo de la película del servidor y lo muestra en pantalla.


## Base de Datos:  

La base de datos utilizada contiene información sobre películas lanzadas desde 1890 hasta la actualidad. Se obtuvo del siguiente enlace:

  https://www.kaggle.com/datasets/kunwarakash/imdbdatasets?select=title_basics.tsv


## Formato de los Datos: 

El archivo title_basics.tsv contiene las siguientes columnas:
 * tconst: Identificador alfanumérico único del título.
 * titleType: Tipo/formato del título (ej. movie, short, tvseries, tvepisode, video).
 * primaryTitle: El título más popular o el utilizado en materiales promocionales.
 * originalTitle: Título original, en el idioma original.
 * isAdult: Indicador de contenido para adultos (0: no-adulto; 1: adulto).
 * startYear: Año de lanzamiento del título. En series de TV, es el año de inicio.
 * endYear: Año de fin de la serie de TV. \N para otros tipos de títulos.
 * runtimeMinutes: Duración principal del título en minutos.
 * genres: Hasta tres géneros asociados al título.


## Modo de uso de los archivos:

1) Iniciar el proceso cliente p1-dataProgram.
1) Iniciar el proceso servidor p2-dataProgram. Desde p1-dataProgram generar la orden de crear la HashTable, si no ha sido creada.
3) Utilizar p1-dataProgram para buscar películas por título y año.


## Explicación General de Funciones Clave

Las siguientes funciones son fundamentales para el funcionamiento del proyecto:
- hash_string(const char *str): Calcula el valor hash de un título utilizando el algoritmo djb2.
- crear_tabla_hash_vacia(const char *path): Crea un archivo binario para la tabla hash, inicializando todas sus entradas a cero.
- insertar_en_disco(const char *path, const char *title, int year, off_t off): Inserta un nuevo nodo (con título, año y offset en la base de datos) en el archivo hash, enlazándolo al bucket correspondiente.
- construir_desde_tsv(const char *hash_path, const char *tsv_path): Lee el archivo TSV y construye el archivo hash, incluyendo solo los títulos válidos y sus años correspondientes.
- buscar_en_disco(const char *archivo_hash, const char *archivo_tsv, const char *title, SharedData *shared_data): Recorre la lista de nodos asociados a un título y año específicos, añadiendo los resultados al búfer de salida compartido.


## Justificación de Criterios de Búsqueda

Consideramos que el título y el año de lanzamiento son los criterios más óptimos para la consulta, ya que nos permiten acercarnos a un resultado único y preciso. Aunque pueden existir varias películas con el mismo nombre, su año de lanzamiento suele ser diferente, lo que facilita la identificación de la entrada deseada en la base de datos.

## Tiempos de Ejecución
Para evaluar el rendimiento del sistema, se utilizó la utilidad time a fin de medir el tiempo de ejecución completo del proceso de búsqueda. Se consideraron dos escenarios distintos:

- Película existente en la base de datos
Se utilizó como ejemplo la búsqueda del título "Matrix" (1999). El sistema realiza la búsqueda correctamente y retorna el registro correspondiente desde el archivo TSV.
Resultado de la medición:
  - https://github.com/user-attachments/assets/24e9459e-da6c-4662-b191-433863e83d97
- Película inexistente en la base de datos
Se buscó la película "Matrix" (2099), la cual no se encuentra en la base de datos. El sistema recorre los nodos correspondientes sin encontrar coincidencias.
Resultado de la medición:
  - https://github.com/user-attachments/assets/5e2b9fe2-1c19-4cc9-939f-a88761046c7a




