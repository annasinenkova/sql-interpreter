CC = g++
CFLAGS = -g -std=c++11 -O2 -ftrapv -fsanitize=address,undefined -Wall -Werror -Wformat-security -Wignored-qualifiers -Winit-self -Wswitch-default -Wfloat-equal -Wshadow -Wpointer-arith -Wtype-limits -Wempty-body -Wlogical-op -Wmissing-field-initializers -Wcast-qual -Wwrite-strings -lm
LFLAGS = -fsanitize=address,undefined

all: server client clean

client: client.cpp unsocket.o exception.o
	$(CC) $(CFLAGS) $^ -o $@
	
server: server.cpp parser.o poliz.o scanner.o unsocket.o table.o exception.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.cpp %.hpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.o
	
clean_temp:
	rm client server mysocket Students
