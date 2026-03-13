# Variáveis de Compilação
CC = gcc
CFLAGS = -Wall -Wextra -O3 -fPIC -Icore/include
LDFLAGS = -shared

# Destino da biblioteca
LIB_NAME = core/libengine.so

# Busca todos os arquivos .c dentro de core/src
SOURCES = $(wildcard core/src/*.c)
OBJECTS = $(SOURCES:.c=.o)

# Alvo principal
all: $(LIB_NAME)

# Cria a Shared Library a partir dos objetos
$(LIB_NAME): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)
	@echo "------------------------------------------------"
	@echo "Biblioteca $(LIB_NAME) gerada com sucesso!"
	@echo "------------------------------------------------"

# Compila cada .c em um .o (Position Independent Code)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza
clean:
	rm -f core/src/*.o $(LIB_NAME)

.PHONY: all clean