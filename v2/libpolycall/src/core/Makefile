# Feature Makefile for core
include ../../config/features.xml

SOURCES = $(wildcard *.c)
OBJECTS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SOURCES))

all: $(OBJECTS)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@rm -f $(OBJ_DIR)/*.o

.PHONY: all clean
