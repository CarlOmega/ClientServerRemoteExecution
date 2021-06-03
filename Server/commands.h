#include "misc.h"

int checkProject(char directory[DEFAULT_BUFLEN]);
void getSourceFiles(char directory[DEFAULT_BUFLEN], char cfiles[MAX_ARGS][DEFAULT_BUFLEN], int* cNum);
void getList(char directory[DEFAULT_BUFLEN], char files[][DEFAULT_BUFLEN], int* num, int longlist, int programlist);
int put(V_SOCKET clientSocket, char args[MAX_ARGS][DEFAULT_BUFLEN], int argsCount);
int get(V_SOCKET clientSocket, char args[MAX_ARGS][DEFAULT_BUFLEN], int argsCount);
int run(V_SOCKET clientSocket, char args[MAX_ARGS][DEFAULT_BUFLEN], int argsCount);
int lst(V_SOCKET clientSocket, char args[MAX_ARGS][DEFAULT_BUFLEN], int argsCount);
int sys(V_SOCKET clientSocket, char args[MAX_ARGS][DEFAULT_BUFLEN], int argsCount);
