# Nombre de los ejecutables
SERVER = p2-dataProgram
USER = p1-dataProgram

# Compilador
CC = gcc

# Opciones de compilación
CFLAGS = -Wall -O2

# Objetivos por defecto
all: $(SERVER) $(USER)

# Compilación de server
$(SERVER): p2-dataProgram.c
	$(CC) $(CFLAGS) -o $(SERVER) p2-dataProgram.c -lrt -pthread

# Compilación de user
$(USER): p1-dataProgram.c
	$(CC) $(CFLAGS) -o $(USER) p1-dataProgram.c -lrt -pthread

# Limpiar archivos generados
clean:
	rm -f $(SERVER) $(USER)
