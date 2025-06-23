# Nombre de los ejecutables
SERVER = servidor
USER = cliente

# Compilador
CC = gcc

# Opciones de compilación
CFLAGS = -Wall -O2

# Objetivos por defecto
all: $(SERVER) $(USER)

# Compilación de server
$(SERVER): server.c
	$(CC) $(CFLAGS) -o $(SERVER) servidor.c -lrt -pthread

# Compilación de user
$(USER): user.c
	$(CC) $(CFLAGS) -o $(USER) cliente.c -lrt -pthread

# Limpiar archivos generados
clean:
	rm -f $(SERVER) $(USER)
