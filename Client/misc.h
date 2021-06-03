// Standard libraries.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// defines
#define DEFAULT_BUFLEN 1024
#define MAX_ARGS 10

// Unix libraries.
#ifndef _WIN32
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>

	// defines
	#define V_SOCKET int
	#define INVALID_SOCKET -1

// Windows libraries
#else
	#include <winsock2.h>
	#include <windows.h>
	#include <ws2tcpip.h>
	#include <iphlpapi.h>
	#include <direct.h>
	#pragma comment(lib, "Ws2_32.lib")

	// defines
	#define V_SOCKET SOCKET
#endif


// Function defines
void error(const char *msg);
#ifndef _WIN32
void ZombieKill(int sig);
#endif
