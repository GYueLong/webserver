CC=g++
OBJ=./lib

server: ./lib/main.o ./lib/conf.o ./lib/http.o ./lib/log.o ./lib/server.o ./lib/thread.o ./lib/util.o
	$(CC) ./lib/main.o ./lib/conf.o ./lib/http.o ./lib/log.o ./lib/server.o ./lib/thread.o ./lib/util.o -o server

$(OBJ)/main.o: main.c ./include/head.h ./lib/server.o
	$(CC) -c main.c -I ./include 

$(OBJ)/conf.o: ./src/conf.cpp ./include/conf.h
	$(CC) -c ./src/conf.cpp -I ./include 

$(OBJ)/http.o: ./src/http.cpp ./include/http.h
	$(CC) -c ./src/http.cpp -I ./include 

$(OBJ)/log.o: ./src/log.cpp ./include/log.h ./lib/thread.o
	$(CC) -c ./src/log.cpp -I ./include 

$(OBJ)/server.o: ./src/server.cpp ./include/server.h ./lib/util.o
	$(CC) -c ./src/server.cpp -I ./include 

$(OBJ)/thread.o: ./src/thread.cpp ./include/threadpool.h
	$(CC) -c ./src/thread.cpp -I ./include -lpthread 

$(OBJ)/util.o: ./src/util.c ./include/util.h
	$(CC) -c ./src/util.c -I ./include  

	

clean: 
	rm *.o



 
 