#include "misc.h"
#include "network.h"

// Note if host is windows need to connect to port 3000.
#define PORT 80

/*!
 * Loops waiting for user input then forks and deals with command.
 */
int main(int argc, char* argv[]) {
	V_SOCKET serverSocket;
	char buffer[DEFAULT_BUFLEN], hostname[DEFAULT_BUFLEN], command[DEFAULT_BUFLEN];
	int bytes, port = PORT;

	if (argc < 2) {
		error("No enough arguments");
	}
	if (argc == 3) {
		port = atoi(argv[2]);
	}
	strcpy(hostname, argv[1]);
	// Kill children
	#ifndef _WIN32
	signal(SIGCHLD, ZombieKill);
	#endif
	while (1) {
		fgets(buffer, DEFAULT_BUFLEN, stdin);
		buffer[strlen(buffer)-1] = 0;
		// Call create process which
		if (strcmp(buffer, "quit") == 0) {
			return 1;
		} else if (strcmp(buffer, "") != 0) {
			strcpy(command, buffer);
			#ifndef _WIN32
			pid_t childPID;
			childPID = fork();
			if (childPID < 0) {
				error("Error forking");
			} else if (childPID == 0) {
				serverSocket = connectClientSocket(hostname, port);
				if (serverSocket != INVALID_SOCKET) {
					manageCommand(serverSocket, buffer);
					close(serverSocket);
				} else {
					printf("Error connecting to server\n");
				}
				_exit(0);
			} else {
				char* token = strtok(buffer, " ");
				if (token != NULL && ((strcmp(token, "get") == 0) || (strcmp(token, "list") == 0))) {
					wait(NULL);
				}
			}
			#else
			STARTUPINFO si;
			PROCESS_INFORMATION pi;

			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			ZeroMemory(&pi, sizeof(pi));
			char cmdArgs[DEFAULT_BUFLEN] = "winchild.exe ", argBuffer[DEFAULT_BUFLEN];
			sprintf(argBuffer, "\"%s\" %s %d", buffer, hostname, port);
			strcat(cmdArgs, argBuffer);
			printf("%s\n", cmdArgs);
			CreateProcess(TEXT(".\\winchild.exe"),   // No module name (use command line)
				cmdArgs,        // Command line
				NULL,           // Process handle not inheritable
				NULL,           // Thread handle not inheritable
				TRUE,          // Set handle inheritance to FALSE
				0,              // No creation flags
				NULL,           // Use parent's environment block
				NULL,           // Use parent's starting directory
				&si,            // Pointer to STARTUPINFO structure
				&pi            // Pointer to PROCESS_INFORMATION structure
			);
			// WSADuplicateSocket(clientSocket, pi.dwProcessId, &protInfo); might need to use this later...
			char* token = strtok(buffer, " ");
			if (token != NULL && ((strcmp(token, "get") == 0) || (strcmp(token, "list") == 0))) {
				WaitForSingleObject(pi.hProcess,INFINITE);
			}
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
			#endif
		}
	}
	return 1;
}
