/*--------------------------------------------------------------------*/
/* conference client */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <time.h> 
#include <errno.h>

#include <stdlib.h>

#define MAXMSGLEN  1024

extern char * recvtext(int sd);
extern int sendtext(int sd, char *msg);

extern int hooktoserver(char *servhost, ushort servport);
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
int main(int argc, char *argv[]) {
	int sock;

	fd_set rfds;
	int retval;

	/* check usage */
	if (argc != 3) {
		fprintf(stderr, "usage : %s <servhost> <servport>\n", argv[0]);
		exit(1);
	}

	/* get hooked on to the server */
	sock = hooktoserver(argv[1], atoi(argv[2]));
	if (sock == -1) {
		perror("Error: ");
		exit(1);
	}

	FD_ZERO(&rfds);

	FD_SET(0, &rfds); /*used to specify stdin */
	FD_SET(sock, &rfds);  /*used to point to SERVER messages??***/
	/*struct timeval tv; used to have some sort of time limit */

	/* keep talking */
	/*TODO: might have to reinitialize rfds after every iteration */
	while (1) {

		/*
		 FILL HERE   (----------DONE----------)
		 use select() to watch simultaneously for
		 inputs from user and messages from server
		 */

		/*select(nfds, &readfds, &writefds, exceptfds, struct timeval*)   				
			nfds = highest file descriptor number + 1
			rdfs = set of file descriptors that are being watched
			returns the number of bits set in the file descriptors
		*/
		/*tv.tv_sec = 120; have the client wait for max of 2 minutes/
		tv.tv_usec = 0;
		*/
		retVal = select(sock+1, &rfds, NULL, NULL, NULL);

		
		if(retVal == -1){
			perror("select(): ");
			exit(1);
		}
		/*FILL HERE: message from server?  (---------DONE--------)*/
		/*checks if any bit was set and if sock's bit was set*/
		/*CHANGES: if->else if  */
		else if (retVal > 0 && FD_ISSET(sock, &rfds) ) {
			char *msg;
			msg = recvtext(sock);
			if (!msg) {
				/* server killed, exit */
				fprintf(stderr, "error: server died\n");
				exit(1);
			}

			/* display the message */
			printf(">>> %s", msg);
			/* free the message */
			free(msg);
		}

		/* FILL HERE: input from keyboard? (-------DONE---------)*/
		/*checks if any bit was set and if stdin's bit was set */
		/*CHANGES:: if->else if */
		else if (retVal > 0 && FD_ISSET(0, &rfds) ) {
			char msg[MAXMSGLEN];
			if (!fgets(msg, MAXMSGLEN, stdin))
				exit(0);
			sendtext(sock, msg);
		}
		else{
			printf("%s\n", "no messages or input within 2 minutes.");
		}

		printf(">");
		fflush(stdout);
		
		/*REINITIALIZE fd_set */
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		FD_SET(sock, &rfds);

	}
	return 0;
}
/*--------------------------------------------------------------------*/

