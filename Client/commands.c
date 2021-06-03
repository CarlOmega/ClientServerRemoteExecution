#include "commands.h"

/*!
 * Validates command then sends command to server.
 * Then waits for "ready" or "error".
 * if ready then sends all files content to server.
 */
int put(V_SOCKET serverSocket, char args[MAX_ARGS][DEFAULT_BUFLEN], int argsCount) {
	char path[DEFAULT_BUFLEN], serverMessage[DEFAULT_BUFLEN], receiveBuffer[DEFAULT_BUFLEN];
	int force = 0, i, bytesRecv;
	//validate all files exist
	if (argsCount < 2) {
		printf("No source file specified.\n");
		return -1;
	}
	// make more generic sending
	if (strcmp(args[argsCount-1], "-f") == 0) {
		force = 1;
		argsCount--;
	}
	for (i = 1; i < argsCount; i++) {
		#ifndef _WIN32
		sprintf(path, "./%s", args[i]);
		#else
		sprintf(path, ".\\%s", args[i]);
		#endif
		printf("Checking file %s\n", path);
		FILE* file = fopen(path, "r");
		if (file == NULL) {
			printf("ERROR: File not found\n");
			return -1;
		}
		fclose(file);
	}
	// Send original command
	strcpy(serverMessage, "put");
	for (i = 0; i < argsCount; i++) {
		strcat(serverMessage, " ");
		strcat(serverMessage, args[i]);
	}
	if (force)
		strcat(serverMessage, " -f");
	send(serverSocket, serverMessage, strlen(serverMessage), 0);
	// Maybe wait for response to say command is G.
	bytesRecv = recv(serverSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
	//check server accepted.
	if (bytesRecv > 0) {
		receiveBuffer[bytesRecv] = '\0';
		if (strcmp(receiveBuffer, "ready") == 0) {
			printf("Server accepted command sending files now...\n");
		} else if (strcmp(receiveBuffer, "error") == 0) {
			bytesRecv = recv(serverSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
			if (bytesRecv > 0)
				printf("ERROR: %s", receiveBuffer);
			return -1;
		}
	} else {
		return -1;
	}
	// Send files
	for (i = 1; i < argsCount; i++) {
		#ifndef _WIN32
		sprintf(path, "./%s", args[i]);
		#else
		sprintf(path, ".\\%s", args[i]);
		#endif
		printf("Sending file %s\n", path);
		FILE* file = fopen(path, "r");
		if (file != NULL) {
			while(fgets(serverMessage, DEFAULT_BUFLEN, file)) {
				send(serverSocket, serverMessage, strlen(serverMessage), 0);
				printf("Sent: %s", serverMessage);
				bytesRecv = recv(serverSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
				printf("Line accepted %s", receiveBuffer);
			}
			printf("File Done sending.\n");
			send(serverSocket, "EOF", strlen("EOF"), 0);
		} else {
			printf("ERROR: File could not be opened.\n");
			send(serverSocket, "EOF", strlen("EOF"), 0);
			break;
		}
		fclose(file);
	}
	return 1;
}

/*!
 * Validates command then sends command to server.
 * Then waits for "ready" or "error".
 * if ready Receive 40 lines and display to user
 * waiting for input to then display more.
 */
int get(V_SOCKET serverSocket, char args[MAX_ARGS][DEFAULT_BUFLEN], int argsCount) {
	char serverMessage[DEFAULT_BUFLEN], receiveBuffer[DEFAULT_BUFLEN];
	int bytesRecv, i, done, set;
	// validate command
	if (argsCount != 2) {
		printf("ERROR: get [progname] [sourcefile].\n");
		return -1;
	}
	// rebuild command to send
	strcpy(serverMessage, "get");
	for (i = 0; i < argsCount; i++) {
		strcat(serverMessage, " ");
		strcat(serverMessage, args[i]);
	}
	// Send and wait for ready
	send(serverSocket, serverMessage, strlen(serverMessage), 0);
	bytesRecv = recv(serverSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
	if (bytesRecv > 0) {
		receiveBuffer[bytesRecv] = '\0';
		if (strcmp(receiveBuffer, "ready") == 0) {
			receiveBuffer[bytesRecv] = '\0';
			printf("Server accepted command wait on file now...\n");
		} else if (strcmp(receiveBuffer, "error") == 0) {
			bytesRecv = recv(serverSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
			if (bytesRecv > 0)
				printf("ERROR: %s", receiveBuffer);
			return -1;
		}
	} else {
		return -1;
	}

	done = 0;
	set = 1;
	do {
		for (i = 1+(set-1)*40; i <= 40*set; i++) {
			bytesRecv = recv(serverSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
			if (bytesRecv > 0) {
				receiveBuffer[bytesRecv] = '\0';
				if (strcmp(receiveBuffer, "EOF") == 0) {
					printf("Get file is done.\n");
					done = 1;
					break;
				}
				printf("%d. %s", i, receiveBuffer);
				send(serverSocket, "next", strlen("next"), 0);
			} else {
				printf("ERROR: maybe disconnected.\n");
				return -1;
			}
		}
		if (!done)
			getc(stdin);
		set++;
	} while (!done);

	return 1;
}

/*!
 * Validates command then sends command to server.
 * Then waits for "ready" or "error".
 * If ready either displays return or writes to file given.
 */
int run(V_SOCKET serverSocket, char args[MAX_ARGS][DEFAULT_BUFLEN], int argsCount) {
	char serverMessage[DEFAULT_BUFLEN], receiveBuffer[DEFAULT_BUFLEN], fileOut[DEFAULT_BUFLEN];
	FILE* file = NULL;
	int fileOutput = 0, done = 0, bytesRecv, i;
	if (argsCount == 0) {
		printf("Error no prog to compile and run\n");
		return -1;
	}
	if (argsCount > 1) {
		// loop through args looking for -w run progname {} (-w [-f] localfile)
		if (argsCount > 2) {
			if (strcmp(args[argsCount-2], "-w") == 0) {
				//run progname {} (-w localfile)??
				fileOutput = 1;
				#ifndef _WIN32
				sprintf(fileOut, "./%s", args[argsCount-1]);
				#else
				sprintf(fileOut, ".\\%s", args[argsCount-1]);
				#endif
				// Check if file exists
				file = fopen(fileOut, "r");
				if (file != NULL) {
					printf("No -f cannot overwrite file.\n");
					return -1;
				}
				fclose(file);
				// Open file for writing
				file = fopen(fileOut, "w");
				if (file == NULL) {
					printf("Error opening file.\n");
					return -1;
				}
				argsCount -= 2;
			} else if ((strcmp(args[argsCount-3], "-w") == 0) && (strcmp(args[argsCount-2], "-f") == 0)) {
				//run progname {} (-w [-f] localfile)??
				fileOutput = 1;
				#ifndef _WIN32
				sprintf(fileOut, "./%s", args[argsCount-1]);
				#else
				sprintf(fileOut, ".\\%s", args[argsCount-1]);
				#endif
				file = fopen(fileOut, "w");
				if (file == NULL) {
					printf("Error opening file.\n");
					return -1;
				}
				argsCount -= 3;
			}
		}
	}
	// Rebuild command to send to server
	strcpy(serverMessage, "run");
	for (i = 0; i < argsCount; i++) {
		strcat(serverMessage, " ");
		strcat(serverMessage, args[i]);
	}

	send(serverSocket, serverMessage, strlen(serverMessage), 0);
	bytesRecv = recv(serverSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
	if (bytesRecv > 0) {
		receiveBuffer[bytesRecv] = '\0';
		if (strcmp(receiveBuffer, "ready") == 0) {
			printf("Server accepted command wait on file now...\n");
		} else if (strcmp(receiveBuffer, "error") == 0) {
			bytesRecv = recv(serverSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
			receiveBuffer[bytesRecv] = '\0';
			if (bytesRecv > 0)
				printf("ERROR: %s", receiveBuffer);
			if (fileOutput)
				fclose(file);
			return -1;
		}
	} else {
		return -1;
	}
	// Received response
	do {
		bytesRecv = recv(serverSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
		if (bytesRecv > 0) {
			receiveBuffer[bytesRecv] = '\0';
			if (strcmp(receiveBuffer, "EOF") == 0) {
				printf("Get file is done.\n");
				done = 1;
				break;
			}
			// if writing to local do stuff here
			if (fileOutput) {
				fputs(receiveBuffer, file);
			} else {
				printf("%s", receiveBuffer);
			}
			send(serverSocket, "next", strlen("next"), 0);
		}
	} while (!done);
	if (fileOutput)
		fclose(file);
	return 1;
}

/*!
 * Validates command then sends command to server.
 * Then waits for "ready" or "error".
 * if ready Receive 40 lines and display to user
 * waiting for input to then display more.
 */
int lst(V_SOCKET serverSocket, char args[MAX_ARGS][DEFAULT_BUFLEN], int argsCount) {
	char serverMessage[DEFAULT_BUFLEN], receiveBuffer[DEFAULT_BUFLEN];
	int bytesRecv, i, done, set;
	if (argsCount > 2) {
		printf("To many args.\n");
		return -1;
	}
	// Rebuild command to send to server
	strcpy(serverMessage, "list");
	for (i = 0; i < argsCount; i++) {
		strcat(serverMessage, " ");
		strcat(serverMessage, args[i]);
	}

	send(serverSocket, serverMessage, strlen(serverMessage), 0);
	bytesRecv = recv(serverSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
	if (bytesRecv > 0) {
		receiveBuffer[bytesRecv] = '\0';
		if (strcmp(receiveBuffer, "ready") == 0) {
			printf("Server accepted command wait on file now...\n");
		} else if (strcmp(receiveBuffer, "error") == 0) {
			bytesRecv = recv(serverSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
			if (bytesRecv > 0)
				printf("ERROR: %s", receiveBuffer);
			return -1;
		}
	} else {
		return -1;
	}

	// Received response
	done = 0;
	set = 1;
	do {
		for (i = 1+(set-1)*40; i <= 40*set; i++) {
			bytesRecv = recv(serverSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
			if (bytesRecv > 0) {
				receiveBuffer[bytesRecv] = '\0';
				if (strcmp(receiveBuffer, "EOF") == 0) {
					printf("List is done.\n");
					done = 1;
					break;
				}
				printf("%d. %s", i, receiveBuffer);
				send(serverSocket, "next", strlen("next"), 0);
			} else {
				printf("ERROR: maybe disconnected.\n");
				return -1;
			}
		}
		if (!done)
			getc(stdin);
		set++;
	} while (!done);

	return 1;
}

/*!
 * Sends command to server then displays the response if no error.
 */
int sys(V_SOCKET serverSocket, char args[MAX_ARGS][DEFAULT_BUFLEN], int argsCount) {
	char serverMessage[DEFAULT_BUFLEN], receiveBuffer[DEFAULT_BUFLEN];
	int bytesRecv, done = 0;

	send(serverSocket, "sys", strlen("sys"), 0);
	bytesRecv = recv(serverSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
	if (bytesRecv > 0) {
		receiveBuffer[bytesRecv] = '\0';
		if (strcmp(receiveBuffer, "ready") == 0) {
			printf("Server accepted command wait on sys info now...\n");
		} else if (strcmp(receiveBuffer, "error") == 0) {
			bytesRecv = recv(serverSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
			if (bytesRecv > 0)
				printf("ERROR: %s", receiveBuffer);
			return -1;
		}
	} else {
		return -1;
	}

	do {
		bytesRecv = recv(serverSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
		if (bytesRecv > 0) {
			receiveBuffer[bytesRecv] = '\0';
			if (strcmp(receiveBuffer, "EOF") == 0) {
				printf("Get sys is done.\n");
				done = 1;
				break;
			}
			printf("%s", receiveBuffer);
			send(serverSocket, "next", strlen("next"), 0);
		}
	} while (!done);
	return 1;
}
