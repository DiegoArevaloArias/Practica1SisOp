# Nombre de los ejecutables
SERVER = server
USER = user

# Compilador
CC = gcc

# Opciones de compilación
CFLAGS = -Wall -O2

# Objetivos por defecto
all: $(SERVER) $(USER)

# Compilación de server
$(SERVER): server.c
	$(CC) $(CFLAGS) -o $(SERVER) server.c -lrt -pthread

# Compilación de user
$(USER): user.c
	$(CC) $(CFLAGS) -o $(USER) user.c -lrt -pthread

# Limpiar archivos generados
clean:
	rm -f $(SERVER) $(USER)
