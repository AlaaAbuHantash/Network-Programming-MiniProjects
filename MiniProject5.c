#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#define SA struct sockaddr *
#define ll long long
const int MOD = 1e9 + 7;
const int SZ = (10 << 10);
typedef enum {
	false, true
} bool;


ll hash(char *s) {
	/*
	 * This function is used to calculate the hash value of requested url,
	 * using rabin karp algorithm.
	 */
	ll res = 0;
	for (; *s; ++s)
		res = ((res * 257LL) % MOD + *s) % MOD;
	return res;
}
void Perror(char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}
void* DOit(void *cc) {
	int confd = *((int*) cc);
	free(cc);
	pthread_detach(pthread_self());
	int n, retval;
	char Request[SZ + 1], Response[SZ + 1], tmp[(1 << 10)], host[1000], url[(1
			<< 10)], File[32];

	// read from client
	while ((n = read(confd, Request, SZ)) > 0) {

		Request[n] = 0;
		sscanf(Request, "%s", tmp);
		bool Rflag = (strcmp(tmp, "GET") == 0 ? true : false); // check if its GET method or not

		//replace (Connection: keep-alive) to (Connection: close);
		char *sptr = strstr(Request, "Connection: keep-alive"), *ssptr;
		if (sptr != NULL) {
			ssptr = sptr + 22;
			sprintf(sptr, "Connection: close%s", ssptr);
			n = strlen(Request);
		}

		// get the FULL URL and HOST
		sscanf(Request, "%*s %s %*[^:]: %s", url, host);

		/*
		 * calculate hash value of the URL we could get unique number for each URL
		 * we use this value to get a unique name for cached files.
		 */
		int hash_val = hash(url);
		memset(File, 0, sizeof File);
		sprintf(File, "%d.out", hash_val);

		// check if this request is cached !

		if (Rflag == true && access(File, F_OK) == 0) {
			// open the cached file
			FILE *fp = fopen(File, "r");
			while ((n = read(fileno(fp), Response, SZ)) > 0) {
				Response[n] = 0;
				retval = write(confd, Response, n);
				if (retval < 0) {
					perror("Writing Error!!");
					close(confd);
					pthread_exit(NULL);
				}
			}
			fclose(fp);
			continue;
		}

		/*
		 * if the file not cached it will be the first time to
		 * request it we will return it to the client and save
		 * it ( if its an 200 okay response) in a file named by its hash
		 * value of the requested url.
		 */

		// change the GET request
		sptr = strstr(url, host), ssptr = strstr(Request, "HTTP");
		if (sptr && ssptr) {
			sptr += strlen(host);
			sprintf(Request, "%s %s %s", tmp, sptr, ssptr);
			n = strlen(Request);

		}

		// get the IP address for the requested host.
		struct hostent *h = gethostbyname(host);
		if (h == NULL) {
			herror("Converting Error!!");
			close(confd);
			pthread_exit(NULL);
		}

		struct sockaddr_in serv2;
		memset(&serv2, 0, sizeof(serv2));
		memcpy(&serv2.sin_addr, h->h_addr_list[0], h->h_length);
		serv2.sin_family = AF_INET;
		serv2.sin_port = htons(80);

		int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0) {
			perror("Socket problem: ");
			close(confd);
			pthread_exit(NULL);
		}

		retval = connect(sockfd, (SA) &serv2, sizeof(serv2));
		if (retval < 0) {
			perror("Connect problem: ");
			close(confd);
			pthread_exit(NULL);
		}

		// send the request to the server.
		if (write(sockfd, Request, n) < 0) {
			perror("Writing problem: ");
			close(confd);
			close(sockfd);
			pthread_exit(NULL);
		}

		bool flage = false;
		FILE *fp;
		//receive Response from server
		while ((n = read(sockfd, Response, SZ)) > 0) {
			Response[n] = 0;
			// if we want to cache the request, it must have a 200 ok response.
			if (Rflag
					== true&& flage == false && strstr(Response, "HTTP/1.1 200 OK") != NULL) {
				flage = true;
				fp = fopen(File, "w");
			}

			if (flage == true) // if its a 200 ok response, we will write the response on the file.
				write(fileno(fp), Response, n);

			//if its a 200 ok response  or not, we will return the response to the client.
			if (write(confd, Response, n) < 0) {
				perror("Writing problem: ");
				close(confd);
				close(sockfd);
				pthread_exit(NULL);
			}

		}
		if (flage == true)
			fclose(fp);
		if (n < 0) {
			perror("Reading problem: ");
			close(confd);
			close(sockfd);
			pthread_exit(NULL);
		}
		retval = close(sockfd);
		if (retval < 0) {
			perror("Closing Socket problem");
			close(confd);
			pthread_exit(NULL);
		}

	}
	retval = close(confd);
	if (retval < 0) {
		perror("Closing Accept socket problem");
		pthread_exit(NULL);
	}
	return (NULL);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		puts("Please insert one command line argument!!");
		exit(EXIT_FAILURE);
	}
	pthread_t tid;
	int *confd, retval;
	int listnfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listnfd < 0)
		Perror("Socket problem: ");

	struct sockaddr_in serv;
	memset(&serv, 0, sizeof(serv));
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
	serv.sin_family = AF_INET;
	serv.sin_port = htons(atoi(argv[1]));

	retval = bind(listnfd, (SA) &serv, sizeof(serv));
	if (retval < 0)
		Perror("Bind problem : ");

	retval = listen(listnfd, (1 << 10));
	if (retval < 0)
		Perror("Listen problem : ");

	while (1) {
		confd = malloc(sizeof(int));
		*confd = accept(listnfd, NULL, NULL);

		if (*confd < 0) {
			Perror("Accept problem : ");
			free(confd);
			pthread_exit(NULL);
		}
		pthread_create(&tid, NULL, DOit, confd);
	}

	retval = close(listnfd);
	if (retval < 0)
		Perror("Closing listening socket problem");

	return 0;
}
