/*--------------------------------------------------------------------*/
/* conference server */

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

extern char * recvtext(int sd);
extern int sendtext(int sd, char *msg);

extern int startserver();
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
int fd_isset(int fd, fd_set *fsp) {
	return FD_ISSET(fd, fsp);
}
/* main routine */
int main(int argc, char *argv[]) {
	int servsock; /* server socket descriptor */

	fd_set livesdset; /* set of live client sockets */
	/*typdef struct fd_set {
		u_int fd_count;  -->number of sockets
		SOCKET fd_array[FD_SETSIZE]; }fd_set;  --->array of sockets (default size=256)
	*/
	int livesdmax; /* largest live client socket descriptor */

	/* check usage */
	if (argc != 1) {
		fprintf(stderr, "usage : %s\n", argv[0]);
		exit(1);
	}

	/* get ready to receive requests */
	servsock = startserver();
	if (servsock == -1) {
		perror("Error on starting server: ");
		exit(1);
	}

	/*
	 FILL HERE: (------DONE------)
	 init the set of live clients
	 */
	 
	 /*FD_ZERO(&fdset) initializes file descriptor set to 0 bits (for all fd's) */
	livesdset.fd_count = livesdmax+1;
	FD_ZERO(&livesdset);
	/*check if all fd's are initialized 
	no reason to check if fdset has 0 elements*/
	if(livesdmax > 0){
		int i;
		for(i = 3; i <= livesdmax; i++){
			if(i == servsock) continue;  /* skip server socket */
			if(fd_isset(i, &livesdset) == 0){
				fprintf(stderr, "%s\n", "fdset isn't initialized.");
				exit(0);
			}
		}
	}
	/* receive requests and process them */
	while (1) {
		int frsock; /* loop variable */

		/*TODO: fdsets need to be reinitialized after each iteration */

		/*
		 FILL HERE  (------DONE------)
		 wait using select() for  
		 messages from existing clients and
		 connect requests from new clients
		 */
		
		/*struct timeval tv; wait for max of 2 min
		tv.tv_sec = 120;
		tv.tv_usec = 0;*/
		/*select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
				struct timeval *timeout)
		nfds->highest file descriptor
		_fds->monitored for changes in communication
		timeout -> how long to wait 
		*/
		/*TODO: might need to initialize writefds*/
		if(select(livesdmax+1, livesdset, NULL, NULL, NULL) == -1){
			perror("select()");
			exit(0);
		}

		/* look for messages from live clients */
		for (frsock = 3; frsock <= livesdmax; frsock++) {
			/* skip the listen socket */
			/* this case is covered separately */
			if (frsock == servsock)
				continue;
 		/* FILL HERE: message from client 'frsock'? (----DONE----)*/ 
			if (fd_isset(frsock, &livesdset)) {
				char * clienthost; /* host name of the client */
				ushort clientport; /* port number of the client */


				/*
				 FILL HERE:  (-----DONE-----)
				 figure out client's host name and port
				 using getpeername() and gethostbyaddr()
				 */
				/*getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
					returns peer connected to socket sockfd and stores it in addr
				
				gethostbyaddr(in_addr *hostaddr, socklen_t len, int type)
				takes in addr and returns hostname
				hostaddr->address (struct sockaddr_in.sin_addr)
				len = sizeof(struct in_addr), type = AF_INET

				*/
				struct hostent *hostEntry1;
				struct sockaddr_in my_addr1;
				memset(&my_addr1, 0, sizeof(struct sockaddr_in));

				/*peer name will be stored in my_addr1 */
				if(getpeername(frsock, (struct sockaddr*)&my_addr1, sizeof(my_addr1)) == -1){ 
					perror("getpeername()");
					exit(0);
				}
				/*ntohs when receiving data? */
				clientport = ntohs(my_addr1.sin_port); /*port number assigned*/
				
				hostEntry1 = gethostbyaddr(&(my_addr1.sin_addr), sizeof(struct in_addr, AF_INET));
				if(hostEntry1 == NULL){
					perror("gethostbyaddr()");
					exit(0);
				}

				clienthost = hostEntry1->h_name; /*hostname assigned */
				
				/* read the message */
				msg = recvtext(frsock);
				if (!msg) {
					/* disconnect from client */
					printf("admin: disconnect from '%s(%hu)'\n", clienthost,
							clientport);

					/*
					 FILL HERE:  (----DONE----)
					 remove this guy from the set of live clients
					 */
					FD_CLR(frsock, &livesdset);
					
					/* close the socket */
					close(frsock);
				} else {
					/*
					 FILL HERE (----DONE----)
					 send the message to all live clients
					 except the one that sent the message
					 */
					
					int j;
					/*cycle through all LIVE clients in fd_set */
					/*TODO: might have to start from fd 0 instead */
					for(j = 3; j <= livesdmax; j++){ 
						/*check if client is live and isn't sender */
						if(j  == servsock) continue; /*ignore server socket*/
						if(j != frsock && fd_isset(j, livesdset)){
							if(sendtext(j, msg) != 1){
								fprintf(stderr, "%s\n","live client didn't receive message.");
								exit(0);
							}
						}
					}

					/* display the message */
					printf("%s(%hu): %s", clienthost, clientport, msg);

					/* free the message */
					free(msg);
				}
			}
		}

		/* look for connect requests */
	/* FILL HERE: connect request from a new client? (---DONE---) */ 
		if (fd_isset(servsock, &livesdset) ) {

			/*
			 FILL HERE: (----DONE----)
			 accept a new connection request
			 */

			 /*accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
			   returns fd for new socket
 			   sockfd->listening socket, addr filled with specified size
			*/
			
			frsock = servsock;
			int csd;
			struct sockaddr_in client_addr;
			memset(&client_addr, 0, sizeof(client_addr));
			csd = accept(frsock, (struct sockaddr*)&client_addr, sizeof(struct sockaddr_in)); 

			/* if accept is fine? */
			if (csd != -1) {
				char * clienthost; /* host name of the client */
				ushort clientport; /* port number of the client */

				/*
				 FILL HERE: (----DONE----)
				 figure out client's host name and port
				 using gethostbyaddr() and without using getpeername().
				 */
								
				struct hostent *clientEntry;
				clientEntry = gethostbyaddr(&(client_addr.sin_addr), sizeof(struct in_addr), AF_INET);
				if(clientEntry == NULL){
					perror("gethostbyaddr()");
					exit(0);
				}
				clienthost = clientEntry->h_name;
				clientport = ntohs(client_addr.sin_port);
				printf("admin: connect from '%s' at '%hu'\n", clienthost,
						clientport);

				/*
				 FILL HERE: (----DONE----)
				 add this guy to set of live clients
				 */
				FD_SET(csd, &livesdset);

			} else {
				/*check for EAGAIN(11) or EWOULDBLOCK(EGAIN)
				***ADDED CODE***: errno, if statement, else braces
				*/
				extern int errno;
				if(errno == 11){
					fprintf(stderr, "%s\n","request queue is currently empty. accept() failed.");
					exit(0);
				}
				else{
					perror("accept");
					exit(0);
				}
			}
		}

		/*********RE-INITIALIZE socket set */
		FD_ZERO(&livesdset);
	
	}
	return 0;
}
/*--------------------------------------------------------------------*/

