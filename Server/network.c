#include "network.h"

/*!
 * Creates a socket. For both windows and unix.
 */
V_SOCKET createServerSocket(int portNum) {
	V_SOCKET sockfd = INVALID_SOCKET;
	struct sockaddr_in serv_addr;
#ifdef _WIN32
	WSADATA wsaData;
	// Initialize Winsock
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
		error("WSAStartup failed.\n");
	}
#endif

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == INVALID_SOCKET) {
		error("Failed to create socket.");
	    return INVALID_SOCKET;
	}
	// Setup socket settings.
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(portNum);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == INVALID_SOCKET) {
		error("Failed to bind to socket.");
	}

	if(listen(sockfd, 5) == INVALID_SOCKET) {
		error("Failed to listen to socket.");
	}

	return sockfd;
}

/*!
 * Accepts a connection. For both windows and unix.
 */
V_SOCKET acceptNewConnection(V_SOCKET sockfd) {
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);

	V_SOCKET newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

	if (newsockfd == INVALID_SOCKET) {
		error("Failed accepting connection.");
		return INVALID_SOCKET;
	}


	printf("Server: got connection from %s port %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
	return newsockfd;
}

/*!
 * Receives first command from client then calls manageCommand with the message.
 */
void manageConnection(V_SOCKET clientSocket) {
	char receiveBuffer[DEFAULT_BUFLEN], returnBuffer[DEFAULT_BUFLEN];
	int bytesRecv;
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);

	bytesRecv = recv(clientSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
	if (bytesRecv > 0) {
		receiveBuffer[bytesRecv] = '\0';
		printf("%s\n", receiveBuffer);
		if (manageCommand(clientSocket, receiveBuffer) > 0) {
			printf("Successful command.");
		} else {
			printf("Error faild command.");
		}
		// send(clientSocket, receiveBuffer, strlen(receiveBuffer), 0);
	}


}

/*!
 * Turns raw string into command and args then calls appropriate function.
 */
int manageCommand(V_SOCKET clientSocket, char buffer[DEFAULT_BUFLEN]) {
	char command[DEFAULT_BUFLEN], args[MAX_ARGS][DEFAULT_BUFLEN], errorMessage[DEFAULT_BUFLEN];
	int argsCount = 0, e;
	// validate command
	char* token = strtok(buffer, " ");
	if (token == NULL) {
		sprintf(errorMessage, "No command given.\n");
		printf("ERROR: %s", errorMessage);
		send(clientSocket, "error", strlen("error"), 0);
		send(clientSocket, errorMessage, strlen(errorMessage), 0);
		return -1;
	}
	strcpy(command, token);
	token = strtok(NULL, " ");
	if (token != NULL) {
		//there is args
		do {
			if (argsCount == MAX_ARGS) {
				sprintf(errorMessage, "Too many args.\n");
				printf("ERROR: %s", errorMessage);
				send(clientSocket, "error", strlen("error"), 0);
				send(clientSocket, errorMessage, strlen(errorMessage), 0);
				return -1;
			}
			strcpy(args[argsCount++], token);
			token = strtok(NULL, " ");
		} while (token != NULL);
	}
	// Debug showing command and args
	printf("Command: %s\n", command);
	for (int i = 0; i < argsCount; i++) {
		printf("Arg {%d}: %s\n", i, args[i]);
	}

	for (int i = 0; i < argsCount; i++) {
		if (strstr(args[i], "/") != NULL || strstr(args[i], "\\") != NULL) {
			sprintf(errorMessage, "Cannot give directoies as args. : %s\n", args[i]);
			printf("ERROR: %s", errorMessage);
			send(clientSocket, "error", strlen("error"), 0);
			send(clientSocket, errorMessage, strlen(errorMessage), 0);
			return -1;
		} else if (strcmp(args[i], ".") == 0 || strcmp(args[i], "..") == 0) {
			sprintf(errorMessage, "Cannot use reletive paths as args. : %s\n", args[i]);
			printf("ERROR: %s", errorMessage);
			send(clientSocket, "error", strlen("error"), 0);
			send(clientSocket, errorMessage, strlen(errorMessage), 0);
			return -1;
		}
	}
	// check commands to determin which protocol
	if (strcmp(command, "put") == 0) {
		printf("Running put command\n");
		e = put(clientSocket, args, argsCount);
	} else if (strcmp(command, "get") == 0) {
		printf("Running get command\n");
		e = get(clientSocket, args, argsCount);
	} else if (strcmp(command, "run") == 0) {
		printf("Running run command\n");
		e = run(clientSocket, args, argsCount);
	} else if (strcmp(command, "list") == 0) {
		printf("Running list command\n");
		e = lst(clientSocket, args, argsCount);
	} else if (strcmp(command, "sys") == 0) {
		printf("Running sys command\n");
		e = sys(clientSocket, args, argsCount);
	} else {
		printf("Running unknown command\n");
	}


	return e;

}
