#ifdef _WIN32
#include "misc.h"
#include "network.h"
#include "commands.h"

/*!
 * Windows fork connect then manageCommand.
 */
int main(int argc, char* argv[]) {
	char buffer[DEFAULT_BUFLEN], hostname[DEFAULT_BUFLEN];
	strcpy(buffer, argv[1]);
	strcpy(hostname, argv[2]);
	int port = atoi(argv[3]);
	WSADATA wsaData;
	// Initialize Winsock
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
		error("WSAStartup failed.\n");
	}
	V_SOCKET serverSocket = connectClientSocket(hostname, port);


	if (serverSocket != INVALID_SOCKET) {
		printf("Starting child process:%s socket desc: %d argcount: %d\n", argv[1], serverSocket, argc);
		manageCommand(serverSocket, buffer);
		closesocket(serverSocket);
	} else {
		printf("Error connecting to server\n");
	}

	exit(1);
}
#endif
