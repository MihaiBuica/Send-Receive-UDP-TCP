#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "helpers.h"

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_address server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, n, ret;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN];
	int fdmax;			// valoare maxima fd din multimea read_fds


	if (argc < 4) {
		usage(argv[0]);
	}
	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar

	FD_ZERO(&read_fds);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	//FD_SET(STDIN_FILENO, &read_fds);
	//fdmax = STDIN_FILENO;

	char newbuffer[BUFLEN];
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");

	FD_SET(0, &read_fds);
	FD_SET(sockfd, &read_fds);
	fdmax = sockfd;
	n = send(sockfd, argv[1], strlen(argv[1]), 0);
	DIE(n < 0, "send id");


	int i, ex = 0;
	while (1) {
  		// se citeste de la tastatura
		tmp_fds = read_fds; 
		
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if( i == fdmax){
					memset(newbuffer, 0, BUFLEN);
					ret = recv(sockfd, newbuffer, BUFLEN, 0);
					if (strncmp(newbuffer, "exit", 4) == 0) {
						ex = 1;
						break;
					}
					DIE(ret < 0, "recv");

					printf("%s", newbuffer);
				}
			
				else{
					memset(buffer, 0, BUFLEN);
					fgets(buffer, BUFLEN - 1, stdin);

					if (strncmp(buffer, "exit", 4) == 0) {
						ex = 1;
						break;
					}

					// se trimite mesaj la server

					n = send(sockfd, buffer, strlen(buffer), 0);
					DIE(n < 0, "send");

				}
			}
		}
		if (ex == 1){
			break;
		}
	}


	close(sockfd);

	return 0;
}
