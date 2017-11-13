/*--------------------------------------------------------------------*/
/* functions to connect clients and server */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <time.h> 
#include <errno.h>

#include <stdlib.h>

#define MAXNAMELEN 256
/*--------------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/* prepare server to accept requests
 returns file descriptor of socket
 returns -1 on error
 */
int startserver() {
	int sd; /* socket descriptor */

	char * servhost; /* full name of this host */
	ushort servport; /* port assigned to this server */

	struct sockaddr_in my_addr; /*used to initialize addr family in bind() */

	/*
	 FILL HERE (----------DONE---------)
	 create a TCP socket using socket()  -->provides reliable comm. b/w 2 sockets
	 */
	 /*socket(int domain, int type, int protocol) 
		domain = selects protocol family to be used
		type = socket type
		protocol = protocol to be used with socket (usually only 1 exists per type)
	*/
	sd = socket(AF_INET, SOCK_STREAM, 0);
	/*makes socket NON-BLOCKING 
	if(fcntl(sd, F_SETFL, fcntl(sd, F_GETFL,0) | O_NONBLOCK) == -1){
		fprintf(stderr, "%s\n", "unable to make socket non-blocking");
		return -1;
	}
	*/
	if(sd == -1){
		perror("socket");
		return -1;
	}
	/*
	 FILL HERE (-----DONE-----)
	 bind the socket to some port using bind()
	 let the system choose a port
	 */
		/*bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
			-->assigns addr of size addrlen(bytes) to sockfd */
	
	
	memset(&my_addr, 0, sizeof(struct sockaddr_in)); /*clear struct*/
	my_addr.sin_family = AF_INET; /*domain used for TCP sockets */
	my_addr.sin_port = htons(0); /*only root can bind to other ports, use 0 */
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY); /*set address to any interface*/

	if(bind(sd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1){
		perror("bind");
		return -1;
	}
	/* we are ready to receive connections */
	listen(sd, 5);

	/*
	 FILL HERE   (-------DONE ---------)
	 figure out the full host name (servhost)
	 use gethostname() and gethostbyname()
	 full host name is remote**.cs.binghamton.edu
	 */
	
	struct hostent *hostEntry; 
	if(gethostname(servhost, sizeof(servhost)) == -1){	
		perror("gethostname()");
		return -1;
	}
	/*full name provided in h_name field of hostent struct */
	hostEntry = gethostbyname(servhost);
	if(hostEntry == NULL){
		perror("gethostbyname()");
		return -1;
	}
	servhost = hostEntry->h_name;
	
	/*
	 FILL HERE  (----------DONE----------)
	 figure out the port assigned to this server (servport)
	 use getsockname()
	 */
	/*getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
		returns current addr of sockfd is bound to
	*/
	if(getsockname(sd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1){
		perror("getsockname");
		return -1;
	}
	servport = my_addr.sin_port; /*servport assigned to port found */
	/*printf("printing assigned port: %d\n", ntohs(servport)); ntohs used for shorts*/

	/* ready to accept requests */
	printf("admin: started server on '%s' at '%hu'\n", servhost, servport);
	free(servhost);
	return (sd);
}
/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/*
 establishes connection with the server
 returns file descriptor of socket
 returns -1 on error
 */
int hooktoserver(char *servhost, ushort servport) {
	int sd; /* socket descriptor */

	ushort clientport; /* port assigned to this client */

	/*
	 FILL HERE  (------DONE-------)
	 create a TCP socket using socket()
	 */
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if(sd == -1){
		perror("socket");
		return -1;
	}
	
	/*makes socket NON-BLOCKING
	if(fcntl(sd, F_SETFL, fcntl(sd, F_GETFL, 0) | O_NONBLOCK) == -1){
		fprintf(stderr, "%s\n", "failed to make socket non-blocking");
		return -1;
	}
	*/

	/*
	 FILL HERE (------DONE------)
	 connect to the server on 'servhost' at 'servport'
	 use gethostbyname() and connect()
	 */
	struct hostent *hostEntry;
	hostEntry = gethostbyname(servhost);
	if(!hostEntry){
		perror("gethostbyname");
		return -1;
	}
	struct sockaddr_in my_addr;
	memset(&my_addr, 0, sizeof(struct sockaddr_in));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = servport;  /*port given as parameter */
	/*TODO:CAST MIGHT BE UNNECESSARY*/
	my_addr.sin_addr.s_addr = (*(long*))hostEntry->h_addr_list[0]; /*find addr in list*/

	/*connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
		connect sockfd to specified address (addr)
	*/
	if(connect(sd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_in)) == -1){
		perror("connect");
		return -1;
	}
	
	/*
	 FILL HERE  (--------DONE--------)
	 figure out the port assigned to this client
	 use getsockname()
	 */
	if(getsockname(sd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_in)) == -1){
		perror("getsockname");
		return -1;
	}
	clientport = my_addr.sin_port;

	/* succesful. return socket descriptor */
	printf("admin: connected to server on '%s' at '%hu' thru '%hu'\n", servhost,
			servport, clientport);
	printf(">");
	fflush(stdout);
	return (sd);
}
/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
int readn(int sd, char *buf, int n) {
	int toberead;
	char * ptr;

	toberead = n;
	ptr = buf;
	while (toberead > 0) {
		int byteread;

		byteread = read(sd, ptr, toberead);
		if (byteread <= 0) {
			if (byteread == -1)
				perror("read");
			return (0);
		}

		toberead -= byteread;
		ptr += byteread;
	}
	return (1);
}

char *recvtext(int sd) {
	char *msg;
	long len;

	/* read the message length */
	if (!readn(sd, (char *) &len, sizeof(len))) {
		return (NULL);
	}
	len = ntohl(len);

	/* allocate space for message text */
	msg = NULL;
	if (len > 0) {
		msg = (char *) malloc(len);
		if (!msg) {
			fprintf(stderr, "error : unable to malloc\n");
			return (NULL);
		}

		/* read the message text */
		if (!readn(sd, msg, len)) {
			free(msg);
			return (NULL);
		}
	}

	/* done reading */
	return (msg);
}

int sendtext(int sd, char *msg) {
	long len;

	/* write lent */
	len = (msg ? strlen(msg) + 1 : 0);
	len = htonl(len);
	write(sd, (char *) &len, sizeof(len));

	/* write message text */
	len = ntohl(len);
	if (len > 0)
		write(sd, msg, len);
	return (1);
}
/*----------------------------------------------------------------*/

