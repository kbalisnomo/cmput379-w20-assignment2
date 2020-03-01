prodcon: prodcon.o tands.o queue.o
	gcc -lpthread -lrt -o prodcon prodcon.o tands.o queue.o

prodcon.o: prodcon.c prodcon.h tands.c tands.h queue.c queue.h
	gcc -c -lpthread -lrt prodcon.c tands.c queue.c

tands.o: tands.c tands.h
	gcc -c tands.c

queue.o: queue.c queue.c
	gcc -c queue.c

clean:
	rm -rf *.o *.log prodcon