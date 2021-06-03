Server : server.o misc.o network.o commands.o
	gcc -o Server server.o misc.o network.o commands.o
server.o : server.c commands.h misc.h network.h
	gcc -c server.c
misc.o : misc.c misc.h
	gcc -c misc.c
network.o : network.c network.h misc.h
	gcc -c network.c
commands.o : commands.c commands.h misc.h
	gcc -c commands.c
clean :
	rm server.o misc.o network.o commands.o Server
