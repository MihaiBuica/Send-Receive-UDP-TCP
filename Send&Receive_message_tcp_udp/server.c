#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "helpers.h"

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[])
{
	struct id_socket vector_id[MAX_CLIENTS];
	int vector_sks[MAX_CLIENTS];
	int max_len = 0;
	int sockfd, newsockfd, portno, sockfd_udp;
	char buffer[BUFLEN], recv_buffer[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr, udp_sockaddr, client_udp;
	int n, i, ret, j;
	socklen_t clilen, len_client = sizeof(client_udp);
	char *hostaddrp;

	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax;			// valoare maxima fd din multimea read_fds

	if (argc < 2) {
		usage(argv[0]);
	}

	// se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);
	FD_SET(0, &read_fds);

	portno = atoi(argv[1]);
	DIE(portno == 0, "atoi");

	//socket UDP
	sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(sockfd_udp < 0, "socket UDP");
	memset((char *) &udp_sockaddr, 0, sizeof(udp_sockaddr));
	udp_sockaddr.sin_family = AF_INET;
	udp_sockaddr.sin_port = htons(portno);
	udp_sockaddr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(sockfd_udp, (struct sockaddr *) &udp_sockaddr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind UDP");

	FD_SET(sockfd_udp, &read_fds);
	fdmax = sockfd_udp;


	//socket TCP
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket TCP");

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind TCP");

	ret = listen(sockfd, MAX_CLIENTS);
	DIE(ret < 0, "listen");

	// se adauga noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
	FD_SET(sockfd, &read_fds);
	fdmax = sockfd;

	struct timeval tmp;
	memset(&tmp, 0, sizeof(struct timeval));
	while (1) {
		tmp_fds = read_fds; 
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL); //pentru ca select sa nu se mai blocheze
		DIE(ret < 0, "select");
		if(ret > 0){
			for(i = 0; i <= fdmax; i++){
				if (FD_ISSET(i, &tmp_fds)) {
					if (i == sockfd_udp){
						//suntem pe udp
						char send_string[MESAJFINAL];
						memset(recv_buffer, 0, BUFLEN);
						n = recvfrom(sockfd_udp, recv_buffer, BUFLEN, 0, 
							(struct sockaddr*) &client_udp, (socklen_t *)&len_client);
						DIE(n < 0, "recvfrom");
						recv_buffer[n] = '\0';

						hostaddrp = inet_ntoa(client_udp.sin_addr);
						DIE(hostaddrp == NULL, "inet_ntoa");
						
						strcpy(send_string, hostaddrp);
						strcat(send_string, ":");
						sprintf(send_string + strlen(send_string), "%d", ntohs(client_udp.sin_port));
						strcat(send_string, " - ");
						char topic[50];
						memcpy(&topic, &recv_buffer, 50);
						strcat(send_string, topic);
						strcat(send_string, " - ");
	
						if (recv_buffer[50] == 0){
							strcat(send_string, "INT - ");
							uint32_t nr;
							memcpy(&nr, &recv_buffer[52], sizeof(uint32_t));
							if(recv_buffer[51] == 0){
								sprintf(send_string + strlen(send_string), "%d", ntohl(nr));
								//printf("nr = %d\n", ntohl(nr));
							}
							else{
								strcat(send_string, "-");
								sprintf(send_string + strlen(send_string), "%d", ntohl(nr));
								//printf("nr = -%d\n", ntohl(nr));
							}
						}
						else if (recv_buffer[50] == 1){
							strcat(send_string, "SHORT_REAL - ");
							uint16_t nr;
							memcpy(&nr, &recv_buffer[51], sizeof(uint16_t));
							sprintf(send_string + strlen(send_string), "%.2f", ntohs(nr)/100.0);
							//printf("nr = %.2f\n",  ntohs(nr)/100.0);
						}
						else if (recv_buffer[50] == 2){
							strcat(send_string, "FLOAT - ");
							uint32_t nr;
							memcpy(&nr, &recv_buffer[52], sizeof(uint32_t));
							uint8_t nr_zecimale;
							memcpy(&nr_zecimale, &recv_buffer[52] + sizeof(uint32_t), sizeof(uint8_t));
							float numar = transform_number(ntohl(nr), nr_zecimale);
							if(recv_buffer[51] == 0){
								sprintf(send_string + strlen(send_string), "%.*f", nr_zecimale, numar);
								//printf("nr = %.*f\n", nr_zecimale ,numar);
							}
							else{
								strcat(send_string, "-");
								sprintf(send_string + strlen(send_string), "%.*f", nr_zecimale, numar);
								//printf("nr = -%.*f\n", nr_zecimale ,numar);
							}						
						}
						else if (recv_buffer[50] == 3){
							strcat(send_string, "STRING - ");
							char string[1500];
							memcpy(&string, &recv_buffer[51], 1500);
							strcat(send_string, string);
							//printf("String = %s\n", string);
						}
						printf("%s\n", send_string);
						printf("Numarul de socketi: %d\n", max_len);
					 	for(j = 0; j < max_len; j++){
							if(vector_id[j].online == 1){
								n = send(vector_sks[j], send_string, strlen(send_string), 0);
								DIE(n < 0, "send");			
							}
						}

						//procesez ce am primit pe udp
					}
					else if (i == sockfd){
						// a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
						// pe care serverul o accepta
						printf("New sock\n");
						clilen = sizeof(cli_addr);
						newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
						DIE(newsockfd < 0, "accept");

						// se adauga noul socket intors de accept() la multimea descriptorilor de citire
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax) { 
							fdmax = newsockfd;
						}

					}

					else{
						memset(recv_buffer, 0, BUFLEN);
						n = recv(i, recv_buffer, sizeof(recv_buffer), 0);
						DIE(n < 0, "recv");
						
						if (n == 0){
							printf("Client (%d) disconected.\n", i);
							close(i);
							FD_CLR(i, &read_fds);
						}
						else{

							if (strncmp(recv_buffer, "follow", 6) == 0){
								printf("am ajuns pe if\n");
								break;
							}
							else if (strncmp(recv_buffer, "unfollow", 8) == 0){
								printf("Am ajuns pe else if\n");
								break;
							}
							else{
								vector_sks[max_len] = i;
								vector_id[max_len].sock = i; 
								strncpy(vector_id[max_len].id, recv_buffer, MAX_ID);
								printf("New client (%s) conncted from %s:%d.\n", 
								vector_id[max_len].id, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
								vector_id[max_len].online = 1;
								max_len++;
							}
						//follow
						//unfollow
						}
					}

				}

			}	
		}

		 printf("=========am terminat un ciclu=============\n");
	}

	close(sockfd);
	close(sockfd_udp);

	return 0;
}