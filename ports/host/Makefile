# Pruebas en la PC del codigo portable
#   make        -> vfd_host y screen_test
#   make check  -> verifica la demo y screen.c en el modelo del display

SRC    = ../../src
CC    ?= cc
CFLAGS = -std=c99 -Wall -Wextra -Wconversion -Werror -I$(SRC) -DHOST_TIMESTAMPS
LIB    = $(SRC)/vfd.c $(SRC)/vfd_char.c $(SRC)/screen.c

all: vfd_host screen_test

vfd_host: main.c hal_host.c $(LIB) $(SRC)/demo.c $(SRC)/*.h
	$(CC) $(CFLAGS) -o $@ main.c hal_host.c $(LIB) $(SRC)/demo.c

screen_test: screen_test.c hal_host.c $(LIB) $(SRC)/*.h
	$(CC) $(CFLAGS) -o $@ screen_test.c hal_host.c $(LIB)

stream.txt: vfd_host
	./vfd_host > $@

check: stream.txt screen_test
	python3 ../../tools/vfd_model.py stream.txt
	./screen_test | python3 check_screen.py

clean:
	rm -f vfd_host screen_test stream.txt

.PHONY: all check clean
