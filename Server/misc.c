#include "misc.h"

/*!
 * Prints error then closes the program.
 */
void error(const char *msg)
{
    perror(msg);
#ifdef _WIN32
	    WSACleanup();
#endif
    exit(1);
}
#ifndef _WIN32
/*!
 * If any zombie signals it is then killed asap.
 */
void ZombieKill(int sig) {
	pid_t pid;
	waitpid(-1, &pid, WNOHANG);
	// if (waitpid(-1, &pid, WNOHANG) > 0)
	// 	printf("aahhhh, you killed me %d!\n", pid);
}
#endif
