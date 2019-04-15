CC=gcc
objects = server.o

assignment_two_server : $(objects)
	$(CC) -o assignment_two_server $(objects)

server.o : server.c
	$(CC) -c server.c


clean:
	rm assignment_two_server $(objects)