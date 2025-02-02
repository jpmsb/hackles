# Nome do executável
TARGET = hackles

# Arquivos fonte
SRC = main.c navigation.c strip.c

# Compilador e flags
CC = gcc
CFLAGS = -Wall -std=c23 -O2

# Diretórios de includes e bibliotecas
INCLUDES = -I.
LIBS = ./raylib/libraylib.a -lm -ldl -lpthread -lGL -lrt -lX11 -Werror=return-type

# Regra padrão
all: $(TARGET)

# Regra para criar o executável
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(SRC) $(LIBS)

# Limpeza de arquivos compilados
clean:
	rm -f $(TARGET)

# Regra para reconstruir do zero
rebuild: clean all

.PHONY: all clean rebuild