#ifdef _WIN32
#include "misc.h"
#include "network.h"
#include "commands.h"

/*!
 * Windows fork manageCommand.
 */
int main(int argc, char* argv[]) {

	WSADATA wsaData;

	// Initialize Winsock
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
		error("WSAStartup failed.\n");
	}
	V_SOCKET clientSocket = atoi(argv[1]);

	printf("Starting child process:%s socket desc: %d argcount: %d\n", argv[1], clientSocket, argc);
	manageConnection(clientSocket);
	return 1;
}
#endif
