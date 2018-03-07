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

typedef enum {
	false, true
} bool;

#define SA struct sockaddr *  /* define SA to be struct sockaddr , that is agenaric socket address structure . */

/* define some global variables */
char *ip;
uint16_t port;
double ratio;
int packet_num;

void Perror(char *message) {
      /* function Perror : C library function, take string as aparameter,
        used to prints a descriptive error message on the screen . */
	perror(message);
	exit(EXIT_FAILURE);
}

void GetTime(char *Buffer) {
    /* function GetTime : this function take string as aparameter , used to convert time to string  and store it in Buffer */  
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(Buffer, 80, "%d %b %X %p", timeinfo);
}

void DoIt(int sockfd) {
    /*Create structure variable  
      - client :to storage the client IP address and  Port#*/

	struct sockaddr_in client;
	double Avg = 0;
	socklen_t len = sizeof (client);
	char InBuffer[(1 << 10) + 1], OutBuffer[(1 << 10) + 1], TimeBuffer[80];
	fd_set rset;
	bool flag;
	struct timeval Time;// struct to reset the TimeOut value .

    /*  Set all bytes in socket address structure to zero, and fill in the relevant data members   */  
	memset(&client, 0, len);
	client.sin_family = AF_INET;
	client.sin_port = htons(port);
	inet_pton(AF_INET, ip, &client.sin_addr);
        int i, sz, TimeOutCnt = 0;  

 /* Trying to connect with server :
       - int  ret : the socket file descriptor.
       - if it is successful, ret must be >= 0 ,
         otherwise call the perror to print error message to the user. */
	int ret = connect(sockfd, (SA) &client, len) ; 
	if (ret < 0)
		Perror("Connect problem!!");
	for (i = 0; i < packet_num; ++i) {
  /* specify the descriptors the kernel tests for reading */ 
		FD_ZERO(&rset);// clears a descriptor set */
		FD_SET(sockfd, &rset);// adds aparticular descriptor sockfd in rset *
  /*fill timeout value */
        	Time.tv_sec = 2;
		Time.tv_usec = 0;
		GetTime(TimeBuffer);


  /*  %n : we doesn't use this format to print any thing ,, The argument (sz) must be a pointer to a signed int,
           where the number of characters written so far is stored , so we use it instead of strlen function */
		sprintf(InBuffer, "This is packet #%d, transmission time is: %s%n", i,
				TimeBuffer, &sz); 
		len = sizeof (client);
 
  /* we used write & read inested of sendto & recvfrom , because we use connect */
  /* Trying to send packet to the server :
       - int ret : the socket file descriptor.
       - if it is successful, ret must be >= 0 ,
         otherwise call the perror to print error message to the user. */
 
               ret = write(sockfd,InBuffer,sz);
		if (ret < 0)
			Perror("Sending problem!!");
		flag = true;
		while (1) {
			FD_ZERO(&rset);
			FD_SET(sockfd, &rset);
 
 /* select function : this function allows the process to instruct the kernal to wait for any one 
    of multiple events to occure & to wake up the process only when one or more of these events occurs 
    or when a specified amount of time has passed */   
			ret = select(sockfd + 1, &rset, NULL, NULL, &Time);
			if (ret == 0) {
				printf("seq #%d Request timeout\n", i);
				++TimeOutCnt;
				flag = false;
				break;
			}

      /* Trying to receve packet from server :
        - int ret: the socket file descriptor.
        - if it is successful, ret  must be >= 0 ,
         otherwise call the perror to print error message to the user. */
                        ret = read(sockfd , OutBuffer,(1 << 10));
			if (ret < 0)
				Perror("Reading problem!!");
			OutBuffer[ret] = 0;
			int tmp;
			sscanf(OutBuffer, "This is packet #%d,", &tmp);
			if (tmp < i)
				continue;
			break;
		}
		if (flag == false)
			continue;
    /* calculate Round Trip Time */
		double _RTT = (double) Time.tv_sec * 1000 + (double) Time.tv_usec / 1e3;
		Avg += (2000.0 - _RTT);
		printf("seq #%d rtt = %.10lf ms\n", i, (2000.0 - _RTT));
	}
	Avg /= (double) (packet_num - TimeOutCnt); /*calculate average round-trip time */
        printf(" --- Statistics ---\n");
	printf(
			"%d packets transmitted, %.2lf%% packet loss, average round-trip time = %.5lf ms\n",
			packet_num, ((double) TimeOutCnt / packet_num) * 100.0, Avg);

}
int main(int argc, char **argv) {
	if (argc != 4) {
      /* First, we will check if the user has entered three command-line arguments ,
        if not, print error message to the user. */

		puts("Error, please send 3 command line arguments : IP address , port# & #of request packet to be sent to the server .");
		exit(EXIT_FAILURE);
	}
	ip = argv[1]; // fill IP address from  argv[1] .
	sscanf(argv[2], "%hu", &port); // fill port# from argv[2] .
	sscanf(argv[3], "%d", &packet_num);// fill #of packet to be send to the server from argv[3] .
	printf(
			"Our client will try to ping the host with address: %s, port: %d, using %d packets.\n",
			ip, port, packet_num);
      
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
