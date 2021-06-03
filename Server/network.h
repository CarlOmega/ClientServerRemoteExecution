#include "misc.h"
#include "commands.h"

// Function defines
V_SOCKET createServerSocket(int portNum);
V_SOCKET acceptNewConnection(V_SOCKET sockfd);
void manageConnection(V_SOCKET clientSocket);
int manageCommand(V_SOCKET clientSocket, char buffer[DEFAULT_BUFLEN]);
