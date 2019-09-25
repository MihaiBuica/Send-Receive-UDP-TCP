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
	char newbuffer[BUFLEN];


	if (argc < 4) {
		usage(argv[0]);
	}
	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar

	FD_ZERO(&read_fds);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");
	//CREARE SOCKET TCP + conexiunea cu server-ul
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");

	int flag = 1;
	ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
	DIE(ret < 0, "setsockopt");
	
	FD_SET(0, &read_fds); //socket pentru stdin
	FD_SET(sockfd, &read_fds); //socket-ul TCP
	fdmax = sockfd;

	n = send(sockfd, argv[1], strlen(argv[1]), 0); //trimitere id
	DIE(n < 0, "send id");


	int i, ex = 0;
	while (1) {
		tmp_fds = read_fds; 

		// verificare ce socket/fd este activ
		// apelul este blocat pana ce se realizeaza o conexiune: server sau stdin.
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if( i == fdmax){
					//fd pentru conexiuna cu serverul
					//mesaj primit de la server
					memset(newbuffer, 0, BUFLEN);
					ret = recv(sockfd, newbuffer, BUFLEN, 0);
					if (strncmp(newbuffer, "exit", 4) == 0) {
					//serverul se inchide	
						ex = 1; //clientul trebuie sa se inchida
						break;
					}
					DIE(ret < 0, "recv");

					printf("%s", newbuffer);
				}
			
				else{
					//fd pentru stdin
					//mesaj primit de la stdin
					memset(buffer, 0, BUFLEN);
					fgets(buffer, BUFLEN - 1, stdin);

					if (strncmp(buffer, "exit", 4) == 0) {
						ex = 1;
						break;
					}

					// se trimite mesaj la server

					n = send(sockfd, buffer, strlen(buffer), 0);
					
					// afisarea in urma comenzii trimise
					vector<string> elem = split(buffer);
					if(elem[0] == "subscribe"){
						printf("subscribed %s\n", elem[1].c_str());
					}
					else if (elem[0] == "unsubscribe") {
						printf("unsubscribed %s\n", elem[1].c_str());
					}

					DIE(n < 0, "send");

				}
			}
		}
		//in cazul in care clientul trebuie sa se inchida.
		if (ex == 1){
			break;
		}
	}


	close(sockfd);

	return 0;
}
