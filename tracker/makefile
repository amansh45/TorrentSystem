CC=g++
Tracker: tracker.o functions.o
	$(CC) tracker.o functions.o -o Tracker -pthread

tracker.o: tracker.cpp headers.h json.hpp
	$(CC) -std=c++11 -c tracker.cpp -pthread 

functions.o: functions.cpp headers.h json.hpp
	$(CC) -std=c++11 -c functions.cpp -pthread
