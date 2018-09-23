CC=g++
Client: clients.o functions.o sha1.o
	$(CC) clients.o functions.o sha1.o -o Client -pthread

clients.o: clients.cpp headers.h json.hpp
	$(CC) -std=c++11 -c clients.cpp -pthread 

functions.o: functions.cpp headers.h json.hpp
	$(CC) -std=c++11 -c functions.cpp -pthread

sha1.o: sha1.cpp sha1.h
	$(CC) -c sha1.cpp
