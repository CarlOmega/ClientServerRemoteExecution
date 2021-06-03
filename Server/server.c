#include "misc.h"
#include "network.h"
#include "commands.h"

//Cannot bind to port 80 on windows due to security
#ifndef _WIN32
#define PORT 80
#else
#define PORT 3000
#endif

/*!
 * Loops accepting connection then forks and runs manageCommand.
 */
int main(int argc, char* argv[]) {
	V_SOCKET mainServerSocket, clientSocket;

	//create socket and bind
	mainServerSocket = createServerSocket(PORT);
	#ifndef _WIN32
	signal(SIGCHLD, ZombieKill);
	#endif
	//check for socket error
	if (mainServerSocket != INVALID_SOCKET) {
		printf("Server started on %d\n", PORT);
		while (1) {
			clientSocket = acceptNewConnection(mainServerSocket);
			if (clientSocket != INVALID_SOCKET) {
				#ifndef _WIN32
				pid_t childPID;
				childPID = fork();
				if (childPID < 0) {
					error("Error on forking.");
				} else if (childPID == 0) {
					printf("Succesfully forked, child %d of parent %d\n", getpid(), getppid());
					//process client messages here.
					manageConnection(clientSocket);
					#ifndef _WIN32
					close(clientSocket);
					#else
					closesocket(clientSocket);
					#endif
					_exit(0);
				} else {
					printf("Succesfully forked, parent is free to do shit %d\n", childPID);
				}
				#else
				STARTUPINFO si;
			    PROCESS_INFORMATION pi;

			    ZeroMemory(&si, sizeof(si));
			    si.cb = sizeof(si);
			    ZeroMemory(&pi, sizeof(pi));
				char cmdArgs[DEFAULT_BUFLEN] = "winchild.exe ", id[DEFAULT_BUFLEN];
				sprintf(id, "%d", clientSocket);
				strcat(cmdArgs, id);
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

				//WaitForSingleObject(pi.hProcess,INFINITE);
		        CloseHandle(pi.hThread);
		        CloseHandle(pi.hProcess);
				#endif
			} else {
				error("Failed to accept.");
			}

		}
	}

	close(mainServerSocket);
	return 0;
}
