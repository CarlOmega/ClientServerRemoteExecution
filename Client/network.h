#include "misc.h"

// Function defines
V_SOCKET connectClientSocket(char* hostname, int portNum);
int manageCommand(V_SOCKET serverSocket, char buffer[DEFAULT_BUFLEN]);
