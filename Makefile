nsh:	main-parse.o parse.o
	gcc -o nsh main-parse.o parse.o

main-parse.o:  main-parse.c parse.h
	gcc -c -o main-parse.o main-parse.c

parse.o:  parse.c parse.h
	gcc -c -o parse.o parse.c

clean:
	rm *.o nsh
