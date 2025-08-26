# Variabili
CC = gcc
CFLAGS = -Wall -g -pthread
LDFLAGS = -lncurses -pthread
OBJ = main.o game.o graphics.o utils.o

# Regola principale
game.out: $(OBJ)
	$(CC) $(CFLAGS) -o frogger $(OBJ) $(LDFLAGS)

# Regole per ciascun file oggetto
main.o: main.c game.h graphics.h utils.h common.h
	$(CC) $(CFLAGS) -c main.c

game.o: game.c game.h common.h
	$(CC) $(CFLAGS) -c game.c

graphics.o: graphics.c graphics.h common.h
	$(CC) $(CFLAGS) -c graphics.c

utils.o: utils.c utils.h common.h
	$(CC) $(CFLAGS) -c utils.c

# Pulizia
clean:
	rm -f *.o frogger
