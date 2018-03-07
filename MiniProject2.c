#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

/*
 * The value 256^3 is added to the numeric value of the IP address,
 * in order to get the value of the next IP,eg: 192.168.1.13 ---> 192.168.1.14.
 * The right most oct in IP presentation place is 265 to the power of 3.
 */
const int ADD = 256 * 256 * 256;

/*
 * These values of TimeOut entered By the user (if he choice) .
 * They are initially -1 .
 * if the user didn't change TimeOut values, the TimeOut values stays as default.
 */
int Seconds, MilliSeconds;

/*
 * Error messages printing function
 */
void ErrFun(char * Message) {
	fprintf(stderr, "%s\n", Message);
	exit(EXIT_FAILURE);
}

/*
 * This Function reset the TimeOut value if the user asked for specific values before
 */
void SetTimeOut(int sockfd) {
	struct timeval timeOut;
	timeOut.tv_sec = Seconds;
	timeOut.tv_usec = MilliSeconds;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeOut,
			sizeof(timeOut)) < 0)
		ErrFun("TimeOut rest failed\n");
	if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeOut,
			sizeof(timeOut)) < 0)
		ErrFun("TimeOut rest failed\n");
}

int main(int argc, char **argv) {
	/*
	 *  First, we will check if the user entered two command line arguments,
	 *  if not, we will call the ErrFun function to print error message to the user.
	 */

	if (argc != 3)
		ErrFun("Wrong number of command line arguments!!"); // call the ErrFun function to print error message to the user

	int choice, i, stat;
	struct sockaddr_in serv;
	clock_t startTime, endTime;
	uint32_t subnet_mask, net_addr, curr_addr, host_num;

	system("clear"); // Clear screen

	memset(&serv, 0, sizeof serv); //Clearing the address structure .
	/*** checking if IP is valid ****/

	/*
	 * inet_pton :
	 * This function converts a human readable IPv4 ( argv[1] )
	 * into an address family 32bit and stored it in ( serv.sin_addr )
	 * if the returned value is 1 -> success and if it is 0 -> error in converting.
	 */
	stat = inet_pton(AF_INET, argv[1], &serv.sin_addr);
	if (stat == 0)
		ErrFun("Invalid IP address Format!!"); // call the ErrFun function to print error message to the user

	/*** checking if subnet Mask is valid ****/

	/*
	 * The subnet mask will be valid if :
	 * - all the digits is numeric.
	 * - the mask value between 0 and 31.
	 */
	for (i = 0; argv[2][i]; ++i)
		if (!isdigit(argv[2][i]))
			ErrFun("Invalid subnetMask Format!!"); // call the ErrFun function to print error message to the user

	sscanf(argv[2], "%u", &subnet_mask);
	if (subnet_mask < 0 || subnet_mask > 31)
		ErrFun("Subnet Mask must be positive number less than 32!!"); // call the ErrFun function to print error message to the user

	printf(
			"\nWelcome =)\n This program will list all the computers that are online in this IP: %s with a subnet mask %u.\n\n",
			argv[1], subnet_mask);

	Seconds = MilliSeconds = -1;
	/*
	 * User have the choice to change TCP TimeOut value or can use the default values.
	 */

	puts(
			"If you want to set a TimeOut value for TCP packets please Insert 0, otherwise please insert -1 :");

	scanf("%d", &choice);
	if (!choice) {
		puts(
				"Please insert the value of TimeOut in Seconds and in Milliseconds: ");
		puts("TimeOut in Seconds :");
		scanf("%d", &Seconds);
		puts("TimeOut in Milliseconds :");
		scanf("%d", &MilliSeconds);
	}

	/*
	 *	Network Address is calculated as follow:
	 *	1- Set the right most ( 32 - n ) bits to 1's by subtracting 1 from the value of (2 ^ (32 - n) )
	 *	  (2 ^ (32 - n) ) is equivalent to (1 << (32 - n))
	 *	2- Invert this value to have the left most n bits as 1's and the right most bits as 0
	 *
	 *	3- Change this value into network Big Indean and perform logical and (&) between,
	 *		the given address and the resulted subnet mask
	 * ~~~~~~~~~
	 * The number of valid hosts = ( 2 ^ (hostid = (32 - n) ) ) - 2,
	 * to eliminate the first and last address.
	 *
	 */

	net_addr = (serv.sin_addr.s_addr
			& htonl(~((host_num = (1 << (32 - subnet_mask))) - 1)));
	host_num -= 2;
	curr_addr = net_addr;
	system("clear"); // Clear screen
	puts("Online machines:");
	while (host_num--) {
		curr_addr += ADD;
		/*
		 * Every time the parent process gets the value of the next host and delivers it,
		 * to a child process to test if there is an alive host holding this ip.
		 * after child process is done, it exits the program.
		 */
		if (fork() == 0) { // if the fork returns 0, we will be in child process
			int sockfd = socket(AF_INET, SOCK_STREAM, 0); // creating TCP socket
			if (sockfd < 0) // if socket creation is successful, sockfd must be >= 0
				ErrFun("Error, creating socket is unsuccessful"); // call the ErrFun function to print error message to the user

			if (Seconds != -1) //check if the user chooses to change the TimeOut value, if he dose we will change it for every sockfd
				SetTimeOut(sockfd); // Calling function to change TimeOut value.
			memset(&serv, 0, sizeof serv); //Clearing the address structure .
			serv.sin_family = AF_INET; //Fixing the address family.
			serv.sin_port = htons(INADDR_ANY); //Filling the POET_NUMBER (any port number is okay).
			serv.sin_addr.s_addr = curr_addr; //Filling the IP_ADDRESS.
			/*
			 * clock : returns the number of clock ticks.
			 * Calculating the time spent by substring the start time from the end time,
			 * We will get the time in seconds after dividing the value by CLOCKS_PER_SEC.
			 */
			startTime = clock(); // Save the value of the current time ( before connecting )
			int stat = connect(sockfd, (struct sockaddr *) &serv, sizeof(serv)); // Trying to connect with the server
			endTime = clock(); // save the value of the current time ( after connecting )
			double time_spent = (double) (endTime - startTime) / CLOCKS_PER_SEC;
			int err = errno;
			/*
			 * The host is UP in three cases :
			 * 1- if the connection to that IP and port number is successful.
			 * 2- if a Port unreachable error is returned.
			 * 3- if the port is active but the host sent rest signal.
			 */
			if (stat == 0 || err == ECONNREFUSED || err == ECONNRESET)
				printf("IP: %s, RTT=%.10lf sec\n", inet_ntoa(serv.sin_addr),
						time_spent);

			close(sockfd); // Close the socket.
			exit(0); // Exit from child.
		}
	}

	/*
	 * This loop will ensure that the parent process will terminate until all it's child,
	 * processes exits successfully
	 */
	pid_t pid;
	while ((pid = waitpid(-1, NULL, 0))) {
		if (errno == ECHILD) {
			break;
		}
	}
	puts("Done");
	return 0;
}
