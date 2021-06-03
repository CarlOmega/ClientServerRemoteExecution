#include "misc.h"

int put(V_SOCKET serverSocket, char args[MAX_ARGS][DEFAULT_BUFLEN], int argsCount);
int get(V_SOCKET serverSocket, char args[MAX_ARGS][DEFAULT_BUFLEN], int argsCount);
int run(V_SOCKET serverSocket, char args[MAX_ARGS][DEFAULT_BUFLEN], int argsCount);
int lst(V_SOCKET serverSocket, char args[MAX_ARGS][DEFAULT_BUFLEN], int argsCount);
int sys(V_SOCKET serverSocket, char args[MAX_ARGS][DEFAULT_BUFLEN], int argsCount);
