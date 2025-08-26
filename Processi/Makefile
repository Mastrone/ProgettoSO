CC = gcc
CFLAGS = -Wall -g 
LDFLAGS = -lncurses
TARGET = frogger
OBJECTS = main.o processes.o graphics.o utils.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

main.o: main.c common.h graphics.h processes.h utils.h
	$(CC) $(CFLAGS) -c main.c -o main.o

processes.o: processes.c processes.h common.h utils.h
	$(CC) $(CFLAGS) -c processes.c -o processes.o

graphics.o: graphics.c graphics.h common.h
	$(CC) $(CFLAGS) -c graphics.c -o graphics.o

utils.o: utils.c utils.h common.h
	$(CC) $(CFLAGS) -c utils.c -o utils.o

clean:
	rm -f $(OBJECTS) $(TARGET)
