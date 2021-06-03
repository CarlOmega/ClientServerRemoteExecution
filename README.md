Unix:
Use make for unix compiling

Windows:
Server:
gcc -o Server.exe server.c commands.c misc.c network.c -lWs2_32
gcc -o winchild.exe winchild.c commands.c misc.c network.c -lWs2_32

Client:
gcc -o Client.exe client.c commands.c misc.c network.c -lWs2_32
gcc -o winchild.exe winchild.c commands.c misc.c network.c -lWs2_32