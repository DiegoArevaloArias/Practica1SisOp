# Practica1SisOp
Practica 1 de sistemas operativos 2025-1

## Descripcion:

Se plantea la creacion de dos procesos para bucar campos en una base de datos.

- Proceso servidor: Crea una hashtable indexada a partir del titulo de la pelicula y que almacena nodos con indice en la base de datos, año, titulo, y nodo siguiente, en caso de que ya halla sido creada, recibe del cliente el titulo y busca el primer nodo, luego recibe el año y busca la coincidencia para mandar todo el campo de la base de datos al servidor.
  
- Proceso cliente: Recibe el titulo y año de la pelicula que se quiere mostrar, lo manda al servidor y recibe el campo completo y lo muestra.

## Base de datos:  

  https://www.kaggle.com/datasets/kunwarakash/imdbdatasets?select=title_basics.tsv

## Formato: 

tconst
alphanumeric unique identifier of the title


titleType
the type/format of the title (e.g. movie, short, tvseries, tvepisode, video, etc)


primaryTitle
the more popular title / the title used by the filmmakers on promotional materials at the point of release


originalTitle
original title, in the original language


isAdult
0: non-adult title; 1: adult title


startYear
represents the release year of a title. In the case of TV Series, it is the series start year


endYear
TV Series end year. ‘\N’ for all other title types


runtimeMinutes
primary runtime of the title, in minutes


genres
includes up to three genres associated with the title

## Modo de uso de los archivos:

1) Iniciar p2-dataProgram
1) Iniciar p1-dataProgram y crera la hash table si no ha sido creada
3) Buscar por titulo y año

## Explicacion general de funciones:


hash_string(const char *str): Calcula un hash del título usando el algoritmo djb2.


crear_tabla_hash_vacia(const char *path): Crea un archivo binario de tabla hash inicializando todo a cero.


insertar_en_disco(const char *path, const char *title, int year, off_t off): Inserta un nuevo nodo con título, año y offset al archivo hash enlazándolo al bucket correspondiente.


construir_desde_tsv(const char *hash_path, const char *tsv_path): Lee un archivo TSV y construye el archivo hash con títulos válidos y sus respectivos años.


 buscar_en_disco(const char *archivo_hash, const char *archivo_tsv, const char *title, SharedData *shared_data) : Recorre la lista de nodos títulos del año especificado y los añade al buffer de salida.


## Justificacion criterios:

Consideramos que el titulo y fecha son aquellas cosas mas comunes por las cuales se busca una pelicula, los otros criterios brindan informacion mucho mas especifica.

