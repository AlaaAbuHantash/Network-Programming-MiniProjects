#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BingIP "204.79.197.200"  //Define constant IP
#define HTTP_PORT_NUB 80        //Define constant PORT_NUMBER

void ErrFun(char * Message) {
	/*function ErrFun : receives error message when it's called in the main ,
	 and print it on the screen. */
	fprintf(stderr, "%s\n", Message);
	exit(EXIT_FAILURE);
}
int main(int argc, char **argv) {
	/*First, we will check if the user has entered a single command line argument,
	  if not, call the ErrFun function  to print error message to the user.*/
	if (argc != 2)
		ErrFun("Missing command line arguments!! ,, please try again .\n");

	puts("\nWelcome to our program...\nthis program  will display the first TEN links for your given search pattern using www.bing.com web site serach .");
	printf("You are searching for : %s\n\n", argv[1]);

	/* Trying to creating TCP socket :
	 - int  sockfd : the socket file descriptor.
	 - if socket creation is successful, sockfd must be >= 0 ,
	   otherwise call the ErrFun function to print error message to the user. */
	puts("Creating TCP socket ...");
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		ErrFun("Error, creating socket is unsuccessful");

	puts("Socket created successfully");
	puts("********************\n\n");

	struct sockaddr_in servaddr; //Create structure variable  to storage the server IP address and  PORT_NUMBER .
	memset(&servaddr, 0, sizeof(servaddr)); //Clearing the address structure .
	servaddr.sin_family = AF_INET; //Fixing the address family.
	servaddr.sin_port = htons(HTTP_PORT_NUB); //Filling the POET_NUMBER (Http server).
	servaddr.sin_addr.s_addr = inet_addr(BingIP); //Filling  the IP_ADDRESS (Bing IP) .
	/* Trying to connect with the server :
	 -if the connection established successfully, connect function must be >= 0 ,
	 otherwise call the ErrFun function to print error message to the user. */
	puts("Connecting Bing server...");
	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
		ErrFun("Connection Error!!");

	puts("Connection established successfully");
	puts("********************\n\n");

	char *GET ="GET /search?q=%s HTTP/1.1\r\nHost: www.bing.com\r\nConnection: Close\r\n\r\n";
	/* GET: pointer to storage GET_METHOD format */
	char *sendline = malloc(strlen(GET) + strlen(argv[1]) + 100);
	/* sendline: pointer to store the response from the server, we make it pointer to control it's size using malloc function,
	   based on GET_METHOD size and search_term size */
	sprintf(sendline, GET, argv[1]);
	puts("Requesting search page...");
	/* Trying to send page request to the server :
	 - if the page request successful , write function  must return >= 0 ,
	   otherwise call the ErrFun function to print error message. */
	if (write(sockfd, sendline, strlen(sendline)) < 0)
		ErrFun("Request failed!!");

	puts("Page request successful");
	puts("********************\n\n");
	/* Trying to retrieve HTML page :
	 -reciveline : array of characters to store the response from the server.
	 -(1 << 20) : means (2 ^ 20)  , so we reserve big size.
	 -if the page retrieved successfully , read function  must return >= 0 ,
	  otherwise call the ErrFun function to print error message. */
	puts("Retrieving HTML page...");
	char reciveline[(1 << 20) + 1];
	FILE *file = fopen("file1", "w+"); //Open the file to store the response from the server - HTML page - on it .
	int size;
	while ((size = read(sockfd, reciveline, 1000)) > 0) {
		reciveline[size] = 0;
		fprintf(file, "%s", reciveline);
	}

	if (size < 0)
		ErrFun("Error retrieving HTML page!!");

	puts("Page retrieved successfully");
	puts("********************\n\n");
	/* Trying to find first TEN links and show it in the screen :
	 -SearchPatt : array of characters includes the common part between the links format in HTML page.
	 -using fscanf with this format specifier to skip all characters before this special character '<', then using fgetc function
	  we read from file character by character and compare the first term with the search pattern,
	  if we found 14 sequence characters similar to the SearchPatt,
	  then print and increase the counter by one, otherwise back to while loop . */
	int cnt = 10, num = 1 ;
	fclose(file); //Finished writing on the file, so we close it.
	file = fopen("file1", "r"); //Open the file to read from it.
	char SearchPatt[] = "<h2><a href=\"";
	printf("The first TEN links after searching for \"%s\":\n", argv[1]);
	puts("********************");
	while (cnt && fscanf(file, "%[^<]", reciveline) > 0) {
                    int i=0;
		for (; i < 13 && fgetc(file) == SearchPatt[i]; ++i)
	        	;
		if (i == 13) {
			fscanf(file, "%[^\"]", reciveline);
			printf("%d) %s\n", num, reciveline);
			--cnt;
			num++;
		}
	}
	fclose(file); //Finished reading from the file, so we close it .
	close(sockfd); //Close the socket .
	return 0;
}
