CC = g++
CFLAGS = -Wall -std=c++1z
DEPS = headers.h sha1.h
OBJ = clients.o sha1.o headers.o
%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

RunClient: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^
