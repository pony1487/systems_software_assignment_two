CC=gcc
objects = client.o

assignment_two_client : $(objects)
	$(CC) -o assignment_two_client $(objects)

client.o : client.c
	$(CC) -c client.c


clean:
	rm assignment_two_client $(objects)