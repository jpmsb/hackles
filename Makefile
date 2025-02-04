# Executable name
TARGET = hackles

# Source files
SRC = src/main.c src/navigation.c src/strip.c

# Compiler and flags
CC = gcc
CFLAGS = -Wall -O2

# Include dirs and libraries
INCLUDES = -I. -I src
LIBS = lib/libraylib.a -lm -ldl -lpthread -lGL -lrt -lX11 -Werror=return-type

# Main target
all: $(TARGET)

# Executable creation rule
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(SRC) $(LIBS)

# Clean rule
clean:
	rm -f $(TARGET)

# Rebuild rule
rebuild: clean all

.PHONY: all clean rebuild