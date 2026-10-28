# Pruebas en la PC del codigo portable
#   make        -> vfd_host y screen_test
#   make check  -> verifica la demo, la barra del Doom (show y en vivo) y
#                  screen.c en el modelo del display

SRC    = ../../src
CC    ?= cc
CFLAGS = -std=c99 -Wall -Wextra -Wconversion -Werror -I$(SRC) -DHOST_TIMESTAMPS
LIB    = $(SRC)/vfd.c $(SRC)/vfd_char.c $(SRC)/screen.c

all: vfd_host screen_test doom_live_test

vfd_host: main.c hal_host.c $(LIB) $(SRC)/demo.c $(SRC)/doom.c $(SRC)/*.h
	$(CC) $(CFLAGS) -o $@ main.c hal_host.c $(LIB) $(SRC)/demo.c $(SRC)/doom.c

screen_test: screen_test.c hal_host.c $(LIB) $(SRC)/*.h
	$(CC) $(CFLAGS) -o $@ screen_test.c hal_host.c $(LIB)

doom_live_test: doom_live_test.c hal_host.c $(LIB) $(SRC)/doom_live.c $(SRC)/*.h
	$(CC) $(CFLAGS) -o $@ doom_live_test.c hal_host.c $(LIB) $(SRC)/doom_live.c

stream.txt: vfd_host
	./vfd_host > $@

doom.txt: vfd_host
	./vfd_host doom > $@

check: stream.txt doom.txt screen_test doom_live_test
	python3 ../../tools/vfd_model.py stream.txt
	python3 ../../tools/vfd_model.py --doom doom.txt
	./screen_test | python3 check_screen.py
	./doom_live_test | python3 check_doom_live.py

clean:
	rm -f vfd_host screen_test doom_live_test stream.txt doom.txt

.PHONY: all check clean
