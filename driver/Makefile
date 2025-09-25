CC = gcc
CFLAGS = -Wall -Wextra -pthread
TARGET = touchpad_driver
SOURCES = main.c common.c tcp_server.c touchpad_listener.c

all: compile link

compile:
	cc -Wall -Wextra -pthread -c src/main.c -o build/main.o
	cc -Wall -Wextra -pthread -c src/common.c -o build/common.o
	cc -Wall -Wextra -pthread -c src/tcp_server.c -o build/tcp_server.o
	cc -Wall -Wextra -pthread -c src/touchpad_listener.c -o build/touchpad_listener.o

link:
	gcc -Wall -Wextra -pthread -o build/touchpad_driver build/*.o

clean:
	rm -f $(OBJECTS) $(TARGET)

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/

.PHONY: all clean install