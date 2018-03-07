#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

typedef enum {
	false, true
} bool;
const int Size = (1 << 20); // Equivalent to (2 ^ 20)
const short LINE_FEED = 0x0a; // Equivalent to "\n"
const short CARRIAGE_RETURN = 0x0d; // Equivalent to "\r"

/*
 * A template of Header and HTML code returned in case of a Bad Request response
 * It contains format specifiers so it can be used with sprintf function to add
 * variable values into the response.
 */
const char *BAD_REQUEST = "HTTP/1.0 400 Bad Request\r\nServer: %s\r\n"
		"Date: %s\r\nContent-Type: %s\r\nContent-Length: 153\r\n\r\n"
		"<HTML>\n"
		"<HEAD>\n"
		"<TITLE>Bad Request</TITLE>\n"
		"<BODY><h2>Bad Request - Invalid URL</h2>\n"
		"<hr><p>HTTP Error 400. The request URL is invalid.</p>\n"
		"</BODY>\n"
		"</HTML>\n";

/*
 * A template of Header and HTML code returned in case of a Not Found response
 * The same reason as above for format specifiers.
 */
const char *NOT_FOUND = "HTTP/1.0 404 Not Found\r\nServer: %s\r\n"
		"Date: %s\r\nContent-Type: %s\r\nContent-Length: 109\r\n\r\n"
		"<HTML>\n"
		"<HEAD><TITLE>404 Not Found</TITLE></HEAD>\n"
		"<H4>404 Not Found</H4>\n"
		"File not found.\n"
		"<HR>\n"
		"</BODY>\n"
		"</HTML>\n";

/*
 * A template of Header returned in case of a Ok response
 * The same reason as above for format specifiers.
 */
const char *OK = "HTTP/1.0 200 Ok\r\nServer: %s\r\n"
		"Date: %s\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n";

/*
 * A function to return time in some defined format
 *  using types and functions that can represent the current time in a string buffer.
 */
void GetTime(char *Buffer) {
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(Buffer, 80, "%a, %d %b %Y %X %Z", timeinfo);
}

/*
 * A function to read, parse, create log file, and return response  for each request.
 */

void DoIt(int conn, char * FileName, char *ClientIp) {
	int n, contLen, PathLen;
	char InBuffer[Size], OutBuffer[(1 << 10) + 1], tmp[1024], TimeBuffer[128],
			Path[(1 << 10)], Type[16];

	while ((n = read(conn, InBuffer, Size - 1)) > 0) {
		/*
		 * This Buffer will hold information about the client, requested URI, response,
		 * and time of request.
		 * Which will be added gradually, after response it will be written on the log file
		 */
		char LogInfo[(1 << 10)];
		InBuffer[n] = 0;
		GetTime(TimeBuffer);
		sprintf(LogInfo, "Time of request: %s\n", TimeBuffer); // Time Of request.
		sscanf(InBuffer, "%s %s", tmp, Path); // Extract the Method, and the path of requested file
		strcpy(Type, "text/html");

		if (strcmp(tmp, "POST") == 0) { // POST REQUEST
			GetTime(TimeBuffer);
			sprintf(LogInfo + strlen(LogInfo),
					"Requested file: %s\nClientIp: %s\n", Path, ClientIp);
			sprintf(LogInfo + strlen(LogInfo), "Status: HTTP/1.0 200 Ok\n");
			char *p = Path;
			++p;
			strcpy(Path, p);
			PathLen = strlen(Path);
			FILE *f = fopen(Path, "w"); // GROUP11.html
			char *post = "<html><head><title> Success </title></head>"
					"<body><h1>You have entered your ID successfully </h1>"
					"</body></html>";

			p = strstr(InBuffer, "idgroup11");
			p++;
			p++;
			p++;
			sprintf(LogInfo + strlen(LogInfo), "Data entered (ID) : %s\n", p);
			contLen = strlen(post);
			sprintf(OutBuffer, OK, FileName, TimeBuffer, Type, contLen);
			write(conn, OutBuffer, strlen(OutBuffer));
			write(conn, post, strlen(post));

		} else if (strcmp(tmp, "GET") != 0) { //BAD REQUEST, if any method other than GET
			sprintf(LogInfo + strlen(LogInfo),
					"ClientIp: %s\nStatus: HTTP/1.0 400 Bad Request\n",
					ClientIp); // Add Client Ip, Response status to the logBuffer
			contLen = strlen(BAD_REQUEST);
			GetTime(TimeBuffer);
			sprintf(OutBuffer, BAD_REQUEST, FileName, TimeBuffer, Type); // Write complete response to OutBuffer
			write(conn, OutBuffer, strlen(OutBuffer));
		} else {
			sprintf(LogInfo + strlen(LogInfo),
					"Requested file: %s\nClientIp: %s\n", Path, ClientIp); // Add Requested file path, Client Ip to the logBuffer.
			/*
			 * The following three lines will discard the first "/" from the path
			 * Because the root folder for a C executable code does not begin with "/"
			 */
			char *p = Path;
			++p;
			strcpy(Path, p);
			PathLen = strlen(Path);

			bool NotFound = false; // C does not support Boolean type, But this is defined up as an enum type :P

			/*
			 * The following if else statements will find the content type,
			 * If a dot '.' is not found, then this is assumed to be a directory, "index.html" will be appended
			 * other wise, it's a file with either "html" or "jpg" extensions,
			 * other wise it will return Not Found status.
			 */
			if ((p = strstr(Path, ".")) == NULL) {
				if (PathLen && Path[PathLen - 1] != '/')
					strcat(Path, "/");
				strcat(Path, "index.html");
			} else if (*(++p) == 'j')
				strcpy(Type, "image/jpg");
			else if (*p != 'h')
				NotFound = true;

			FILE *fp = fopen(Path, "r");
			if (NotFound == true || fp == NULL) { // Not Found status
				sprintf(LogInfo + strlen(LogInfo),
						"Status: HTTP/1.0 404 Not Found\n"); // Add Response status to the logBuffer.
				contLen = strlen(NOT_FOUND);
				GetTime(TimeBuffer);
				strcpy(Type, "text/html");
				sprintf(OutBuffer, NOT_FOUND, FileName, TimeBuffer, Type); // Write complete response to OutBuffer
				write(conn, OutBuffer, strlen(OutBuffer));
			} else {
				sprintf(LogInfo + strlen(LogInfo), "Status: HTTP/1.0 200 Ok\n"); // Add Response status to the logBuffer.
				/*
				 * These three lines will throw the file indicator to the end of the file,
				 * Then function ftell will return the position of the indicator
				 * This is equivalent to the file size
				 * rewinde will return the indicator to the beginning of the file.
				 */
				fseek(fp, 0, SEEK_END);
				contLen = ftell(fp);
				rewind(fp);
				GetTime(TimeBuffer);
				sprintf(OutBuffer, OK, FileName, TimeBuffer, Type, contLen); // Write Only response header to OutBuffer
				write(conn, OutBuffer, strlen(OutBuffer));
				/*
				 * In this status, a file will be read gradually and written on the socket after the response header.
				 */
				int m;
				while ((m = fread(OutBuffer, 1, 1024, fp)) > 0)
					write(conn, OutBuffer, m);
				fclose(fp);
			}
		}
		/*
		 * In this step, the LogBuffer will be appended to the LogFile, each request at once.
		 */
		strcat(LogInfo, "------------------\n");
		FILE *pf = fopen("log.out", "a");
		fputs(LogInfo, pf);
		fclose(pf);
	}
}
/*
 * This function is called by the signal handler when catching SIGCHILD signal
 * This will ensure that the child's entry in the process table will be removed
 */

void sig_handle(int signo) {
	pid_t pid;
	int stat;
	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		;
}
int main(int argc, char **argv) {
	struct sockaddr_in serv, clinet;
	socklen_t len = sizeof(clinet);
	memset(&serv, 0, sizeof serv);
	int listnfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listnfd < 0) {
		perror("Error creating listening socket.\n");
		exit(EXIT_FAILURE);
	}
	serv.sin_family = AF_INET;
	serv.sin_port = htons(80);
	serv.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listnfd, (struct sockaddr*) &serv, sizeof serv) < 0) {
		perror("Error calling bind()\n");
		exit(EXIT_FAILURE);
	}
	if (listen(listnfd, 100) < 0) {
		perror("Error calling listen()\n");
		exit(EXIT_FAILURE);
	}
	signal(SIGCHLD, sig_handle);
	int retval;
	for (;;) {
		memset(&clinet, 0, sizeof clinet);
		int conn = accept(listnfd, (struct sockaddr *) &clinet, &len);
		char clientIp[17];
		inet_ntop(AF_INET, &(clinet.sin_addr), clientIp, sizeof clientIp);
		if (conn < 0) {
			perror("Error calling accept()\n");
			exit(EXIT_FAILURE);
		}
		if (fork() == 0) {
			retval = close(listnfd);
			if (retval < 0) {
				perror("Error closing listening socket, child process");
				exit(EXIT_FAILURE);
			}
			DoIt(conn, argv[0], clientIp);
			exit(0);
		}
		retval = close(conn);
		if (retval < 0) {
			perror("Error closing accept socket, parent process");
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}
