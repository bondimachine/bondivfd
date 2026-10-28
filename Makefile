# BondiVFD
#   make check    -> pruebas en la PC (demo y screen.c en el modelo del display)
#   make arduino  -> compila los ejemplos (arduino-cli)
#   make web      -> regenera examples/BondiWeb/index_html.h

BOARD ?= arduino:avr:uno
ESP32 ?= esp32:esp32:esp32

check:
	$(MAKE) -C ports/host check

arduino:
	arduino-cli compile -b $(BOARD) --library . examples/Demo
	arduino-cli compile -b $(ESP32) --library . examples/BondiWeb
	arduino-cli compile -b $(ESP32) --library . examples/PinTest

web:
	python3 embed_html.py

clean:
	$(MAKE) -C ports/host clean

.PHONY: check arduino web clean
