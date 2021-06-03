#include "network.h"
#include "commands.h"

/*!
 * Connects to the server and returns socket descriptor.
 */
V_SOCKET connectClientSocket(char* hostname, int portNum) {
	struct sockaddr_in serv_addr = {0};
	V_SOCKET serverSocket;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET)
        error("ERROR opening socket");

	serv_addr.sin_addr.s_addr = inet_addr(hostname);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portNum);

	if (connect(serverSocket, (struct sockaddr *)&serv_addr , sizeof(serv_addr)) < 0) {
		printf("Could not connect %s", hostname);
		return INVALID_SOCKET;
	} else {
		printf("Client: connected to %s on port %d\n", inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));
	}
	return serverSocket;
}

/*!
 * Turns raw string into command and args then calls appropriate function.
 */
int manageCommand(V_SOCKET serverSocket, char buffer[DEFAULT_BUFLEN]) {
	char command[DEFAULT_BUFLEN], args[MAX_ARGS][DEFAULT_BUFLEN];
	int argsCount = 0, e;
	printf("%s\n", buffer);
	// validate command
	char* token = strtok(buffer, " ");
	if (token == NULL) {
		printf("No command\n");
		return -1;
	}
	strcpy(command, token);
	token = strtok(NULL, " ");
	if (token != NULL) {
		//there is args
		do {
			if (argsCount == MAX_ARGS) {
				printf("Too many args.\n");
				return -1;
			}
			strcpy(args[argsCount++], token);
			token = strtok(NULL, " ");
		} while (token != NULL);
	}
	//
	printf("Command: %s\n", command);
	for (int i = 0; i < argsCount; i++) {
		printf("Arg {%d}: %s\n", i, args[i]);
	}
	double timeDiff;
	#ifdef _WIN32
	LARGE_INTEGER freq;        // ticks per second
	LARGE_INTEGER start, end;           // ticks
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);
	#else
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	#endif
	if (strcmp(command, "put") == 0) {
		printf("Running put command\n");
		e = put(serverSocket, args, argsCount);
	} else if (strcmp(command, "get") == 0) {
		printf("Running get command\n");
		e = get(serverSocket, args, argsCount);
	} else if (strcmp(command, "run") == 0) {
		printf("Running run command\n");
		e = run(serverSocket, args, argsCount);
	} else if (strcmp(command, "list") == 0) {
		printf("Running list command\n");
		e = lst(serverSocket, args, argsCount);
	} else if (strcmp(command, "sys") == 0) {
		printf("Running sys command\n");
		e = sys(serverSocket, args, argsCount);
	} else {
		printf("Running unknown command\n");
	}
	#ifdef _WIN32
	QueryPerformanceCounter(&end);
	timeDiff = (end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;
	#else
	clock_gettime(CLOCK_MONOTONIC, &end);
	timeDiff = (end.tv_sec - start.tv_sec) * 1000.0;
	timeDiff += (end.tv_nsec - start.tv_nsec) / 1000000.0;
	#endif
	printf("Time command took: %lf\n", timeDiff);

	return e;

}
