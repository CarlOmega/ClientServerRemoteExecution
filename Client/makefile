Client : client.o misc.o network.o commands.o
	gcc -o Client client.o misc.o network.o commands.o
client.o : client.c misc.h network.h
	gcc -c client.c
misc.o : misc.c misc.h
	gcc -c misc.c
network.o : network.c network.h misc.h commands.h
	gcc -c network.c
commands.o : commands.c commands.h misc.h
	gcc -c commands.c
clean :
	rm client.o misc.o network.o commands.o Client
