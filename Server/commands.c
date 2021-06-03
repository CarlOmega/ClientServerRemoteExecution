#include "commands.h"

/*!
 * Scans through the given directory to determine if
 * the program need recompiling. It does this by checking dates
 * on .c files and executables. Returns 1 if needs compiling 0 otherwise.
 */
int checkProject(char directory[DEFAULT_BUFLEN]) {
	printf("Checking to see if compiling needs\n");
	char filename[DEFAULT_BUFLEN], program[DEFAULT_BUFLEN];
	#ifndef _WIN32
	sprintf(program, "%s/program", directory);
	#else
	sprintf(program, "%s\\program.exe", directory);
	#endif

	struct stat statbuf;
	time_t programTime;
	if (stat(program, &statbuf) == 0){
		programTime = statbuf.st_mtime;
	} else {
		return 1;
	}

	DIR *directoryPointer = opendir(directory);
	struct dirent *entry;
	while((entry = readdir(directoryPointer)) != NULL) {
		#ifndef _WIN32
		sprintf(filename, "%s/%s", directory, entry->d_name);
		#else
		sprintf(filename, "%s\\%s", directory, entry->d_name);
		#endif
		if(stat(filename, &statbuf) == 0 && !(statbuf.st_mode & S_IFDIR)) {
			if (strcmp(filename+(strlen(filename)-2), ".c") == 0) {
				printf("File: %s", filename);
				if (statbuf.st_mtime > programTime)
					return 1;
			}
		}
	}
	// Close Directory
	closedir(directoryPointer);
	return 0;
}

/*!
 * Scans a directory and stores file paths and number of files
 * in the given pointers.
 */
void getSourceFiles(char directory[DEFAULT_BUFLEN], char cfiles[MAX_ARGS][DEFAULT_BUFLEN], int* cNum) {
	printf("Getting c files to compile.\n");
	DIR *directoryPointer = opendir(directory);
	char filename[DEFAULT_BUFLEN];
	struct stat statbuf;
	struct dirent *entry;
	while((entry = readdir(directoryPointer)) != NULL) {
		#ifndef _WIN32
		sprintf(filename, "%s/%s", directory, entry->d_name);
		#else
		sprintf(filename, "%s\\%s", directory, entry->d_name);
		#endif
		if(stat(filename, &statbuf) == 0 && !(statbuf.st_mode & S_IFDIR)) {
			if (strcmp(filename+(strlen(filename)-2), ".c") == 0) {
				strcpy(cfiles[(*cNum)++], filename);
			}
		}
	}
	// Close Directory
	closedir(directoryPointer);
}

/*!
 * Gets a list of files from give directory.
 * If programlist == 1 will print files inside otherwise just directoies.
 * If longlist == 1 then more details will be stored.
 */
void getList(char directory[DEFAULT_BUFLEN], char files[][DEFAULT_BUFLEN], int* num, int longlist, int programlist) {
	DIR *directoryPointer = opendir(directory);
	char filename[DEFAULT_BUFLEN], timeCreated[DEFAULT_BUFLEN];
	struct stat statbuf;
	struct dirent *entry;
	while((entry = readdir(directoryPointer)) != NULL) {
		#ifndef _WIN32
		sprintf(filename, "%s/%s", directory, entry->d_name);
		#else
		sprintf(filename, "%s\\%s", directory, entry->d_name);
		#endif
		if(stat(filename, &statbuf) == 0) {
			if (programlist) {
				if (!(statbuf.st_mode & S_IFDIR)) {
					if (!longlist) {
						sprintf(files[(*num)++], "|%24s|\n", entry->d_name);
					} else {
						// long list is name/file size/create date/access perms
						struct tm *tm = localtime(&statbuf.st_mtime);
						strftime(timeCreated, sizeof(timeCreated), "%c", tm);
						sprintf(files[(*num)++], "|%24s|%24llu|%24s|%24o|\n", entry->d_name, (unsigned long long)statbuf.st_size, timeCreated, statbuf.st_mode);
					}
				}
			} else {
				if (statbuf.st_mode & S_IFDIR && (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)) {
					if (!longlist) {
						sprintf(files[(*num)++], "|%24s|\n", entry->d_name);
					} else {
						// long list is name/file size/create date/access perms
						struct tm *tm = localtime(&statbuf.st_mtime);
						strftime(timeCreated, sizeof(timeCreated), "%c", tm);
						sprintf(files[(*num)++], "|%24s|%24llu|%24s|%24o|\n", entry->d_name, (unsigned long long)statbuf.st_size, timeCreated, statbuf.st_mode);
					}
				}
			}
		}
	}
	// Close Directory
	closedir(directoryPointer);
	// To flush or it gets stuck on unix.
	printf("Done.\n");
}

/*!
 * Validates put command args are correct.
 *
 * Then checks if -f is specified to see if overwriting is okay.
 * It then sends a message to the client saying it ready for files.
 * it then reads in all files and stores them under program directory.
 */
int put(V_SOCKET clientSocket, char args[MAX_ARGS][DEFAULT_BUFLEN], int argsCount) {
	char path[DEFAULT_BUFLEN], clientMessage[DEFAULT_BUFLEN], errorMessage[DEFAULT_BUFLEN];
	int force = 0, i, bytesRecv;
	// validate command Wont happen with client program but fail safe
	if (argsCount < 2) {
		// Send "error" then error details.
		sprintf(errorMessage, "Not enough args return error\n");
		printf("ERROR: %s", errorMessage);
		send(clientSocket, "error", strlen("error"), 0);
		send(clientSocket, errorMessage, strlen(errorMessage), 0);
		return -1;
	}

	// Make more generic for sending determine is force put
	if (strcmp(args[argsCount-1], "-f") == 0) {
		force = 1;
		argsCount--;
	}
	// Check directory
	struct stat dirInfo;

	#ifndef _WIN32
	sprintf(path, "./%s", args[0]);
	#else
	sprintf(path, ".\\%s", args[0]);
	#endif
	// Make directory or check if files exist if no -f
	if(stat(path, &dirInfo) == 0 && (dirInfo.st_mode & S_IFDIR)) {
		if (!force) {
			// Check no overlapping flies
			for (i = 1; i < argsCount; i++) {
				#ifndef _WIN32
				sprintf(path, "./%s/%s", args[0], args[i]);
				#else
				sprintf(path, ".\\%s\\%s", args[0], args[i]);
				#endif
				FILE* file = fopen(path, "r");
				if (file != NULL) {
					// ERROR file exists but no -f specified
					sprintf(errorMessage, "Cannot create no -f and file exists: %s\n", args[i]);
					printf("ERROR: %s", errorMessage);
					send(clientSocket, "error", strlen("error"), 0);
					send(clientSocket, errorMessage, strlen(errorMessage), 0);
					fclose(file);
					return -1;
				}
			}
		}
	} else {
		printf("Creating dir: %s\n", path);
		#ifdef _WIN32
			_mkdir(path);
		#else
			mkdir(path, 0755);
		#endif
	}
	// Send confirmation that files can be accepted now.
	send(clientSocket, "ready", strlen("ready"), 0);

	for (i = 1; i < argsCount; i++) {
		#ifndef _WIN32
		sprintf(path, "./%s/%s", args[0], args[i]);
		#else
		sprintf(path, ".\\%s\\%s", args[0], args[i]);
		#endif
		printf("Receiving file %s\n", path);
		FILE* file = fopen(path, "w");
		if (file != NULL) {
			do {
				bytesRecv = recv(clientSocket, clientMessage, DEFAULT_BUFLEN-1, 0);
				if (bytesRecv > 0) {
					// Set last char to null so no overflow
					clientMessage[bytesRecv] = '\0';
					// EOF string because "\0" wouldn't send.
					if (strcmp(clientMessage, "EOF") == 0) {
						printf("File done.\n");
						break;
					}
					fputs(clientMessage, file);
					send(clientSocket, "next", strlen("next"), 0);
				}
			} while (1);
			fclose(file);
		}
	}
	return 1;
}

/*!
 * Validates command then returns contents of file requested.
 */
int get(V_SOCKET clientSocket, char args[MAX_ARGS][DEFAULT_BUFLEN], int argsCount) {
	char path[DEFAULT_BUFLEN], clientMessage[DEFAULT_BUFLEN], errorMessage[DEFAULT_BUFLEN];
	int bytesRecv;
	if (argsCount != 2) {
		sprintf(errorMessage, "Error with Argument too many %s\n", path);
		printf("ERROR: %s", errorMessage);
		send(clientSocket, "error", strlen("error"), 0);
		send(clientSocket, errorMessage, strlen(errorMessage), 0);
		return -1;
	}
	// Check dir
	struct stat dirInfo;
	FILE* file;
	#ifndef _WIN32
	sprintf(path, "./%s", args[0]);
	#else
	sprintf(path, ".\\%s", args[0]);
	#endif
	// Validate progname is a directory
	if (stat(path, &dirInfo) == 0 && (dirInfo.st_mode & S_IFDIR)) {
		#ifndef _WIN32
		strcat(path, "/");
		#else
		strcat(path, "\\");
		#endif
		strcat(path, args[1]);
		file = fopen(path, "r");
		if (file != NULL) {
			send(clientSocket, "ready", strlen("ready"), 0);
		} else {
			sprintf(errorMessage, "Counld not open file: %s\n", path);
			printf("ERROR: %s", errorMessage);
			send(clientSocket, "error", strlen("error"), 0);
			send(clientSocket, errorMessage, strlen(errorMessage), 0);
			fclose(file);
			return -1;
		}
	} else {
		sprintf(errorMessage, "Could not open directory %s\n", path);
		printf("ERROR: %s", errorMessage);
		send(clientSocket, "error", strlen("error"), 0);
		send(clientSocket, errorMessage, strlen(errorMessage), 0);
		return -1;
	}

	while (fgets(clientMessage, DEFAULT_BUFLEN, file)) {
		send(clientSocket, clientMessage, strlen(clientMessage), 0);
		bytesRecv = recv(clientSocket, clientMessage, DEFAULT_BUFLEN-1, 0);
		if (bytesRecv == 0) {
			printf("Client disconnected stopping send.\n");
			break;
		}
	}
	send(clientSocket, "EOF", strlen("EOF"), 0);
	return 1;
}

/*!
 * Determines if file needs to be recompiled.
 * Then compiles if needed if any errors it returns the contents of the error
 * message.
 */
int run(V_SOCKET clientSocket, char args[MAX_ARGS][DEFAULT_BUFLEN], int argsCount) {
	char path[DEFAULT_BUFLEN],
		clientMessage[DEFAULT_BUFLEN],
		cfiles[MAX_ARGS][DEFAULT_BUFLEN],
		compileCommand[DEFAULT_BUFLEN],
		receiveBuffer[DEFAULT_BUFLEN],
		errorMessage[DEFAULT_BUFLEN];

	int compile = 0, status = 0, limit, bytesRecv, i, cNum = 0;
	FILE* output;
	// setup directory
	#ifndef _WIN32
	sprintf(path, "./%s", args[0]);
	#else
	sprintf(path, ".\\%s", args[0]);
	#endif
	struct stat dirInfo;
	// validate command...
	printf("Checking path: %s\n", path);
	if (stat(path, &dirInfo) == 0 && (dirInfo.st_mode & S_IFDIR)) {
		compile = checkProject(path);
	} else {
		// not a project
		sprintf(errorMessage, "No project: %s\n", path);
		printf("ERROR: %s", errorMessage);
		send(clientSocket, "error", strlen("error"), 0);
		send(clientSocket, errorMessage, strlen(errorMessage), 0);
		return -1;
	}
	// make sure compiled fine.
	if (compile) {
		#ifndef _WIN32
		sprintf(compileCommand, "gcc -o %s/program %s/*.c", path, path);
		#else
		getSourceFiles(path, cfiles, &cNum);
		sprintf(compileCommand, "gcc -o %s\\program.exe ", path);
		for (i=0; i < cNum; i++) {
			strcat(compileCommand, " ");
			strcat(compileCommand, cfiles[i]);
		}
		#endif
		//pipe errors
		strcat(compileCommand, " 2>&1");
		printf("\n\nRunning : %s", compileCommand);
		output = popen(compileCommand, "r");
		if (output == NULL) {
			// Error with popen
			sprintf(errorMessage, "Error with popen %s\n", compileCommand);
			printf("ERROR: %s", errorMessage);
			send(clientSocket, "error", strlen("error"), 0);
			send(clientSocket, errorMessage, strlen(errorMessage), 0);
			return -1;
		}
		// Check for errors then return result if any.
		if (fgets(clientMessage, DEFAULT_BUFLEN, output)) {
			sprintf(errorMessage, "Error with compile: %s\n", clientMessage);
			while(fgets(clientMessage, DEFAULT_BUFLEN, output)) {
				strcat(errorMessage, clientMessage);
			}
			printf("ERROR: %s\n", errorMessage);
			send(clientSocket, "error", strlen("error"), 0);
			send(clientSocket, errorMessage, strlen(errorMessage), 0);
			status = pclose(output);
			printf("Status of pclose on compile: %d\n", status);
			return -1;
		}
		status = pclose(output);
		printf("Status of pclose on compile: %d\n", status);
	}
	// run and return result.
	#ifndef _WIN32
	strcat(path, "/");
	strcat(path, "program");
	#else
	strcat(path, "\\");
	strcat(path, "program.exe");
	#endif
	for (i = 1; i < argsCount; i++) {
		strcat(path, " ");
		strcat(path, args[i]);
	}
	//pipe errors
	strcat(path, " 2>&1");
	output = popen(path, "r");
	if (output == NULL) {
		// Error with popen
		sprintf(errorMessage, "Error with with running program. %s\n", path);
		printf("ERROR: %s", errorMessage);
		send(clientSocket, "error", strlen("error"), 0);
		send(clientSocket, errorMessage, strlen(errorMessage), 0);
		return -1;
	}

	send(clientSocket, "ready", strlen("ready"), 0);
	limit = 0;
	while(fgets(clientMessage, DEFAULT_BUFLEN, output) && limit++ < 1000) {
		send(clientSocket, clientMessage, strlen(clientMessage), 0);
		printf("Sent: %s", clientMessage);
		bytesRecv = recv(clientSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
		if (bytesRecv == 0) {
			printf("Client disconnected stopping send.\n");
			break;
		}
	}
	printf("File Done running.\n");
	send(clientSocket, "EOF", strlen("EOF"), 0);

	pclose(output);

	return 1;
}

/*!
 * Validates command then sends a list of files or program.
 * if -l will send more details.
 */
int lst(V_SOCKET clientSocket, char args[MAX_ARGS][DEFAULT_BUFLEN], int argsCount) {
	char clientMessage[DEFAULT_BUFLEN],
		receiveBuffer[DEFAULT_BUFLEN],
		errorMessage[DEFAULT_BUFLEN],
		path[DEFAULT_BUFLEN],
		files[50][DEFAULT_BUFLEN],
		fileHeader[DEFAULT_BUFLEN];
	int bytesRecv, i, longlist = 0, programlist = 0, num = 0;
	if (argsCount > 2) {
		sprintf(errorMessage, "Error with Arguments: too many %s\n", path);
		printf("ERROR: %s", errorMessage);
		send(clientSocket, "error", strlen("error"), 0);
		send(clientSocket, errorMessage, strlen(errorMessage), 0);
		return -1;
	} else if (argsCount == 2) {
		if (strcmp(args[0], "-l") == 0) {
			// long list of program
			longlist = 1;
			programlist = 1;
			#ifndef _WIN32
			sprintf(path, "./%s", args[1]);
			#else
			sprintf(path, ".\\%s", args[1]);
			#endif
		} else if (strcmp(args[1], "-l") == 0) {
			// long list
			longlist = 1;
			programlist = 1;
			#ifndef _WIN32
			sprintf(path, "./%s", args[0]);
			#else
			sprintf(path, ".\\%s", args[0]);
			#endif
		} else {
			sprintf(errorMessage, "Error with Arguments: [-l] [progname] or [progname] [-l] %s\n", path);
			printf("ERROR: %s", errorMessage);
			send(clientSocket, "error", strlen("error"), 0);
			send(clientSocket, errorMessage, strlen(errorMessage), 0);
			return -1;
		}
	} else if (argsCount == 1) {
		if (strcmp(args[0], "-l") == 0) {
			// long list of just prognames
			longlist = 1;
			#ifndef _WIN32
			sprintf(path, "./");
			#else
			sprintf(path, ".\\");
			#endif
		} else {
			// list of progname files
			programlist = 1;
			#ifndef _WIN32
			sprintf(path, "./%s", args[0]);
			#else
			sprintf(path, ".\\%s", args[0]);
			#endif
		}
	} else {
		#ifndef _WIN32
		sprintf(path, "./");
		#else
		sprintf(path, ".\\");
		#endif
	}
	// long list is name/file size/create date/access perms
	if (programlist) {
		struct stat dirInfo;
		// check program exsist
		if (!(stat(path, &dirInfo) == 0 && (dirInfo.st_mode & S_IFDIR))) {
			sprintf(errorMessage, "No such program on the server%s\n", path);
			printf("ERROR: %s", errorMessage);
			send(clientSocket, "error", strlen("error"), 0);
			send(clientSocket, errorMessage, strlen(errorMessage), 0);
			return -1;
		}
	}
	printf("Getting file info\n");
	getList(path, files, &num, longlist, programlist);
	send(clientSocket, "ready", strlen("ready"), 0);
	if (longlist)
		sprintf(fileHeader, "|%24s|%24s|%24s|%24s|\n", "Name:", "Size (bytes):", "Date:", "Permissions:");
	else
		sprintf(fileHeader, "|%24s|\n", "Name:");
	send(clientSocket, fileHeader, strlen(fileHeader), 0);
	bytesRecv = recv(clientSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
	for (i = 0; i < num; i++) {
		send(clientSocket, files[i], strlen(files[i]), 0);
		bytesRecv = recv(clientSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
		if (bytesRecv == 0) {
			printf("Client disconnected stopping send.\n");
			break;
		}
	}
	send(clientSocket, "EOF", strlen("EOF"), 0);

	return 1;
}

/*!
 * Sends details of the server host.
 */
int sys(V_SOCKET clientSocket, char args[MAX_ARGS][DEFAULT_BUFLEN], int argsCount) {
	char clientMessage[DEFAULT_BUFLEN], receiveBuffer[DEFAULT_BUFLEN];
	int bytesRecv;
	#ifndef _WIN32
	char hostname[DEFAULT_BUFLEN], cpuType[DEFAULT_BUFLEN];
	struct utsname info;
	size_t buflen = DEFAULT_BUFLEN;
	hostname[DEFAULT_BUFLEN-1] = '\0';
	gethostname(hostname, DEFAULT_BUFLEN-1);
	uname(&info);
	// Ready to send sys info
	send(clientSocket, "ready", strlen("ready"), 0);
	// NAME
	sprintf(clientMessage, "|%20s: %s\n", "Name", hostname);
	send(clientSocket, clientMessage, strlen(clientMessage), 0);
	bytesRecv = recv(clientSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
	if (bytesRecv == 0) {
		printf("Client disconnected stopping send.\n");
		return -1;
	}
	// VERSION
	sprintf(clientMessage, "|%20s: %s %s\n", "Version", info.sysname, info.release);
	send(clientSocket, clientMessage, strlen(clientMessage), 0);
	bytesRecv = recv(clientSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
	if (bytesRecv == 0) {
		printf("Client disconnected stopping send.\n");
		return -1;
	}
	// No. Processors
	sprintf(clientMessage, "|%20s: %ld\n", "No. of Processors", sysconf(_SC_NPROCESSORS_ONLN));
	send(clientSocket, clientMessage, strlen(clientMessage), 0);
	bytesRecv = recv(clientSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
	if (bytesRecv == 0) {
		printf("Client disconnected stopping send.\n");
		return -1;
	}
	// Processor Type
	#ifdef __APPLE__
	// MAC____
	sysctlbyname("machdep.cpu.brand_string", &cpuType, &buflen, NULL, 0);
	sprintf(clientMessage, "|%20s: %s\n", "Processor Type", cpuType);
	#else
	//unix
	FILE* cpufile = popen("cat /proc/cpuinfo | grep 'model name' | uniq", "r");
	if (cpufile != NULL) {
		fgets(cpuType, DEFAULT_BUFLEN, file);
		pclose(cpuType);
	}
	sprintf(clientMessage, "|%20s: %s\n", "Processor Type", cpuType);
	#endif
	send(clientSocket, clientMessage, strlen(clientMessage), 0);
	bytesRecv = recv(clientSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
	if (bytesRecv == 0) {
		printf("Client disconnected stopping send.\n");
		return -1;
	}
	#else

	// WINDOWS__________________
	char systemVersion[DEFAULT_BUFLEN] = "Windows";
	SYSTEM_INFO siSysInfo;
	TCHAR infoBuf[32767];
	DWORD bufCharCount = 32767;
	FILE* file = popen("ver", "r");
	if (file != NULL) {
		// to ignore the \n
		fgets(systemVersion, DEFAULT_BUFLEN, file);
		fgets(systemVersion, DEFAULT_BUFLEN, file);
		pclose(file);
	}

	GetComputerName(infoBuf, &bufCharCount);
	GetSystemInfo(&siSysInfo);
	// Ready to send sys info
	send(clientSocket, "ready", strlen("ready"), 0);
	// NAME
	sprintf(clientMessage, "|%20s: %s\n", "Name", infoBuf);
	send(clientSocket, clientMessage, strlen(clientMessage), 0);
	bytesRecv = recv(clientSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
	if (bytesRecv == 0) {
		printf("Client disconnected stopping send.\n");
		return -1;
	}
	// VERSION
	sprintf(clientMessage, "|%20s: %s", "Version", systemVersion);
	send(clientSocket, clientMessage, strlen(clientMessage), 0);
	bytesRecv = recv(clientSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
	if (bytesRecv == 0) {
		printf("Client disconnected stopping send.\n");
		return -1;
	}
	// No. Processors
	sprintf(clientMessage, "|%20s: %d\n", "No. of Processors", siSysInfo.dwNumberOfProcessors);
	send(clientSocket, clientMessage, strlen(clientMessage), 0);
	bytesRecv = recv(clientSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
	if (bytesRecv == 0) {
		printf("Client disconnected stopping send.\n");
		return -1;
	}
	// Processor Type
	sprintf(clientMessage, "|%20s: %d\n", "Processor Type", siSysInfo.dwProcessorType);
	send(clientSocket, clientMessage, strlen(clientMessage), 0);
	bytesRecv = recv(clientSocket, receiveBuffer, DEFAULT_BUFLEN-1, 0);
	if (bytesRecv == 0) {
		printf("Client disconnected stopping send.\n");
		return -1;
	}
	#endif
	send(clientSocket, "EOF", strlen("EOF"), 0);
	return 1;
}
