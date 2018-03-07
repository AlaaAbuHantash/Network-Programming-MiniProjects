#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define SA struct sockaddr *   /* define SA to be struct sockaddr , that is agenaric socket address structure . */
/*define some global variables */
uint16_t port;
double ratio;
double fRand(double fMin, double fMax) {
        /* function fRand : this function take two parameters refers to the range of the random number , 
          used to  allow  the server to  generates a random number . */
	double f = (double) rand() / RAND_MAX;
	return fMin + f * (fMax - fMin);
}
void Perror(char *message) {
     /* function Perror : C library function, take string as aparameter,
        used to prints a descriptive error message on the screen . */
	perror(message);
	exit(EXIT_FAILURE);
}
void DoIt(int sockfd) {
     /* function DoIt  : this function take UDP_SOCKET as aparameter ,, it generated random number, then compares the random number with
        the ratio entered as a command-line argument , If it  is less than the ratio, the server ignores the request & doesn't send anything,
        Otherwise the server echoes the contents of the request packet back to the client . */  
       
    /*Create structure variable  
      - serv : to storage the server IP address and  Port#
      - client :to storage the client IP address and  Port#*/
        struct sockaddr_in serv, client;
	socklen_t len = sizeof (serv);  
	char InBuffer[(1 << 10) + 1], ip[17];   /*(1 << 10) : means (2 ^ 10) */
   
 /*  Set all bytes in socket address structure to zero, and fill in the relevant data members   */
	memset(&serv, 0, len);
	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
 
   /* Trying to Bind our UDP_SOCKET :
       - int  ret : the socket file descriptor.
       - if it is successful, ret must be >= 0 ,
         otherwise call the perror to print error message to the user. */
	int ret = bind(sockfd, (SA) &serv, len);
	if (ret < 0)
		Perror("Bind problem:");
	puts("Server started listening right now...");
	for (;;) {
		len = sizeof client;
 
     /* Trying to receve packet from client :
       - int sz : the socket file descriptor.
       - if it is successful, sz must be >= 0 ,
         otherwise call the perror to print error message to the user. */
		int sz = recvfrom(sockfd, InBuffer, (1 << 10), 0, (SA) &client, &len);
		if (ret < 0)
			Perror("Reading problem:");
		InBuffer[sz] = 0;
		inet_ntop(AF_INET, &client.sin_addr, ip, sizeof (ip)); // fill client IP .
		int tmp;
		sscanf(InBuffer, "This is packet #%d", &tmp);
		if (tmp == 0)
			puts("");
		double random_val = fRand(0, 1); // Call fRand function to generate random value in the range [0,1] .
		if ((ratio - random_val) > 0.0) {
			printf("Packet #%d, from host with address: %s well be dropped,"
				"because it's luck with random values is Bad :P\n",
					tmp, ip);
			continue;
		}

        /* Trying to send the same request packet to the client :
       - int ret : the socket file descriptor.
       - if it is successful, ret must be >= 0 ,
         otherwise call the perror to print error message to the user. */
		ret = sendto(sockfd, InBuffer, sz, 0, (SA) &client, len);
		if (ret < 0)
			Perror("Sending problem!!");
                printf("Packet #%d, from host with address: %s is returned successfully :D\n", tmp,ip);

	}
}
int main(int argc, char **argv) {
 
      /*srand : C library function ,take  an integer value to be used as seed by
        the pseudo-random number generator algorithm . */ 
	srand(time(0)); 
	if (argc != 3) {
      /*First, we will check if the user has entered two command-line arguments ,
        if not, print error message to the user.*/
		puts("Error, please send 2 command line arguments , port# and ratio of requests will be dropped by the server .");
		exit(EXIT_FAILURE);
	}

	sscanf(argv[1], "%hu", &port); // fill port# from argv [1] .
	sscanf(argv[2], "%lf", &ratio);// fill  ratio of requests not to be answered by the server from rgv[2] .
	printf(
			"Our server will be bind to port #%d, and only a %.2lf%% of the incoming packets will be dropped\n",
			port, ratio * 100.0);
     
       /* Trying to creating UDP socket :
	 - int  sockfd : the socket file descriptor.
	 - if socket creation is successful, sockfd must be >= 0 ,
	   otherwise call the perror to print error message to the user. */
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
		Perror("Socket problem: ");
       /*calling  DoIt function */ 
	DoIt(sockfd);
	return 0;
}
