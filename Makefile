CC := gcc 
CFLAGS := -std=c99 -Wall -pedantic
INC := -I../stfl-0.24 -I/usr/local/Cellar/ncurses/6.1/include -I/usr/local/opt/sqlite/include -L/usr/local/Cellar/ncurses/6.1/lib -L../stfl-0.24 -L/usr/local/opt/sqlite/lib 
LIB := -lstfl -lncursesw -lpthread -lsqlite3
SRCS := shelf.c sql.c structures.c

shelf:
	$(CC) $(CFLAGS) $(INC) $(SRCS) -o shelf.o $(LIB)

clean:
	rm *.o
