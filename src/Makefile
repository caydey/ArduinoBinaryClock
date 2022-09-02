BOARD=arduino:avr:pro
PORT=/dev/ttyUSB0
all:
	arduino-cli compile -b $(BOARD)
	arduino-cli upload -p $(PORT) -b $(BOARD)

build:
	arduino-cli compile -b $(BOARD)

flash:
	arduino-cli upload -p $(PORT) -b $(BOARD)

