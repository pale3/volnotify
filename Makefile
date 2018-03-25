BIN = volnotify
SRC = volnotify.c
OBJ = $(SRC:.c=)
PREFIX?=/usr/local

LIBS = alsa libnotify
CFLAGS = -Wall -Wextra -Os
LDFLAGS = -g $(shell pkg-config --cflags --libs $(LIBS))

all: volnotify

volnotify:
	@echo " [CC] $(SRC)"
	@$(CC) $(LDFLAGS) $(CFLAGS) $(SRC) -o $(OBJ)

install: all
	install -m 0755 $(BIN) $(DESTDIR)/$(PREFIX)/bin/$(BIN)
	@rm -f $(BIN)

uninstall:
	@echo " [uninstall]: $(OBJ)"
	@rm -rf $(DESTDIR)/$(PREFIX)/bin/$(BIN)

.PHONY: all volnotify install uninstall
