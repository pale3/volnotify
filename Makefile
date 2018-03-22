SRC = vol.c
OBJ = $(SRC:.c=)

LIBS = alsa libnotify
CFLAGS = -Wall -Wextra -Os
LDFLAGS = -g $(shell pkg-config --cflags --libs $(LIBS))

all: volnot

volnot:
	@echo " [CC] $(SRC)"
	@$(CC) $(LDFLAGS) $(CFLAGS) $(SRC) -o $(OBJ)

clean:
	@echo " [CLEAN]: $(OBJ)"
	@rm -f $(OBJ)

.PHONY: all volnot clean
