#include "helpers.h"

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}
//comparator pentru map
struct cmpByKey{
	bool operator()(const string &a, const string &b) const{
		return a < b;
	}
};
//functie pentru crearea unui numar de tip float
//se imparte nr la 10^(zecimale)
float transform_number(uint32_t nr, uint8_t zecimale){
	float zec = 1.00;
	while(zecimale){
		zec *= 10;
		zecimale--;
	}
	return nr / zec ;
}
//functie pentru dezabonarea unui client => stergere din vector
void unsubs(vector<struct id_socket>& vect, int sock){
	size_t i;
	for(i = 0; i < vect.size(); i++){
		if (vect[i].sock == sock){
			vect.erase(vect.begin() + i - 1);
			break;
		}
	}
}
//functie verificare daca un id este online
bool is_online (vector<struct id_socket> &vect, char* id){
	size_t i;
	for(i = 0; i < vect.size(); i++){
		if (strcmp(vect[i].id, id) == 0){
			if(vect[i].online == 1)
				return true;
			else
				return false;
		}
	}
	return false;
}
//functie pentru a deconecta un client.
void off(vector<struct id_socket>& vect, int sock){
	size_t i;
	for(i = 0; i < vect.size(); i++){
		if (vect[i].sock == sock){
			vect[i].online = 0;
			break;
		}
	}
}
//functie pentru a actualiza informatiile despre un id
bool actualizare_status (vector<struct id_socket> &vect, char* id, int sock){
	size_t i;
	for(i = 0; i < vect.size(); i++){
		if (strcmp(vect[i].id, id) == 0){
			vect[i].sock = sock;
			vect[i].online = 1;
			return true;
		}
	}
	return false;
}
//functie pentru identificarea id-ului in functie de socket
char* find_id_by_sock(vector<struct id_socket>& vect, int sock){
	size_t i;
	for(i = 0; i < vect.size(); i++){
		if (vect[i].sock == sock){
			return vect[i].id;
		}
	}
	return NULL;
}
//functie pentru identificarea socket-ului in functie de id
int find_sock_by_id(vector<struct id_socket>& vect, char* id){
	size_t i;
	for(i = 0; i < vect.size(); i++){
		if(vect[i].online == 1){
			return vect[i].sock;
		}
		else{
			return -1;
		}
	}
	return -1;
}
//functie pentru gasirea clientului in functie de socket
struct id_socket find_elem_by_sock(vector<struct id_socket>& vect, int sock){
	size_t i;
	struct id_socket aux;
	memset(&aux, 0, sizeof(struct id_socket));
	for(i = 0; i < vect.size(); i++){
		if (vect[i].sock == sock){
			return vect[i];
		}
	}
	return aux;
}

int main(int argc, char *argv[])
{
	vector<struct id_socket> vector_id_subs;
	map<string, vector<struct id_socket> , cmpByKey > topics; //topicurile
	map<string, vector<struct id_socket> , cmpByKey>::iterator it; //iterator pentru find
	topics["clients"] = vector<struct id_socket> (); //un topic la care vor fi abonati toti clientii default

	int sockfd, newsockfd, portno, sockfd_udp; 
	char recv_buffer[BUFLEN], buffer[BUFLEN]; //mesaje trimise / primite
	struct sockaddr_in serv_addr, cli_addr, udp_sockaddr, client_udp;

	int n, i, ret;
	size_t j; 

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
	FD_SET(0, &read_fds); //pentru stdin

	portno = atoi(argv[1]); //citire port
	DIE(portno == 0, "atoi");

	//deschidere socket UDP
	sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(sockfd_udp < 0, "socket UDP");
	memset((char *) &udp_sockaddr, 0, sizeof(udp_sockaddr));
	udp_sockaddr.sin_family = AF_INET;
	udp_sockaddr.sin_port = htons(portno);
	udp_sockaddr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(sockfd_udp, (struct sockaddr *) &udp_sockaddr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind UDP");

	FD_SET(sockfd_udp, &read_fds); //adaugare socket udp in multime
	fdmax = sockfd_udp;


	//dechidere socket TCP
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket TCP");

	int flag = 1;
	ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
	DIE(ret < 0, "setsockopt");

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

	struct timeval tmp; //daca dorim ca select sa nu mai fie blocant, putem folosi tmp(setat pe 0)
	memset(&tmp, 0, sizeof(struct timeval));
	while (1) {
		tmp_fds = read_fds; 
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");
		if(ret > 0){ //daca s-a primit ceva pe un socket
			for(i = 0; i <= fdmax; i++){
				if (FD_ISSET(i, &tmp_fds)) {
					if(i == 0){
					//Am primit pe socket-ul dedicat stdin => posibila comanda de exit
					//programul se termina cu aceasta comanda
						memset(&buffer, 0, BUFLEN);
						fgets(buffer, BUFLEN, stdin);
						if(strncmp(buffer, "exit", 4) == 0){
							size_t p = 0;
							//trimit comanda de exit tuturor clientilor
							for(p = 0; p < vector_id_subs.size(); p++){
								if (vector_id_subs[p].online == 1){
									n = send(vector_id_subs[p].sock, "exit", 4, 0);
									DIE(n < 0, "send exit");		
									close(vector_id_subs[p].sock);
									FD_CLR(vector_id_subs[p].sock, &read_fds);
								}
							}
							close(sockfd);
							close(sockfd_udp);
							return 0;
						}
					}
					else if (i == sockfd_udp){
						//Am primit mesaj pe socket-ul pentru UDP.
						char send_string[MESAJFINAL];
						memset(recv_buffer, 0, BUFLEN);
						n = recvfrom(sockfd_udp, recv_buffer, BUFLEN, 0, 
							(struct sockaddr*) &client_udp, (socklen_t *)&len_client);
						DIE(n < 0, "recvfrom");
						recv_buffer[n] = '\0';

						hostaddrp = inet_ntoa(client_udp.sin_addr);
						DIE(hostaddrp == NULL, "inet_ntoa");

						//Urmeaza parsarea string-ului si construirea mesajului de trimis.
						strcpy(send_string, hostaddrp); //adresa clientului udp
						strcat(send_string, ":");
						sprintf(send_string + strlen(send_string), "%d", ntohs(client_udp.sin_port)); //portul clientului udp
						strcat(send_string, " - ");
						char topic[50];
						memcpy(&topic, &recv_buffer, 50); //topicul mesajului
						strcat(send_string, topic);
						strcat(send_string, " - ");
	
						if (recv_buffer[50] == 0){
							strcat(send_string, "INT - "); //tipul mesajului
							uint32_t nr;
							memcpy(&nr, &recv_buffer[52], sizeof(uint32_t));
							if(recv_buffer[51] == 0){
								sprintf(send_string + strlen(send_string), "%d", ntohl(nr)); //mesajul
							}
							else{ //caz numar negativ
								strcat(send_string, "-");
								sprintf(send_string + strlen(send_string), "%d", ntohl(nr)); //mesajul
							}
						}
						else if (recv_buffer[50] == 1){
							strcat(send_string, "SHORT_REAL - "); //tipul mesajului
							uint16_t nr;
							memcpy(&nr, &recv_buffer[51], sizeof(uint16_t));
							sprintf(send_string + strlen(send_string), "%.2f", ntohs(nr)/100.0); //mesajul
						}
						else if (recv_buffer[50] == 2){
							strcat(send_string, "FLOAT - "); //tipul mesajului
							uint32_t nr;
							memcpy(&nr, &recv_buffer[52], sizeof(uint32_t));
							uint8_t nr_zecimale;
							memcpy(&nr_zecimale, &recv_buffer[52] + sizeof(uint32_t), sizeof(uint8_t));
							float numar = transform_number(ntohl(nr), nr_zecimale); //obtinerea numarului
							if(recv_buffer[51] == 0){
								sprintf(send_string + strlen(send_string), "%.*f", nr_zecimale, numar); //mesajul
							}
							else{ //numar negativ
								strcat(send_string, "-");
								sprintf(send_string + strlen(send_string), "%.*f", nr_zecimale, numar);
							}						
						}
						else if (recv_buffer[50] == 3){
							strcat(send_string, "STRING - "); //tipul mesajului
							char stg[1500];
							memcpy(&stg, &recv_buffer[51], 1500);
							strcat(send_string, stg); //mesajul
						}
						strcat(send_string, "\n");
						//Verificare existenta topic.
						it = topics.find(topic);
						if(it != topics.end()){
							//topicul deja exista => se trimite mesajul la toti subscriberii conectati
							vector<struct id_socket> print = it->second;
						 	for(j = 0; j < print.size(); j++){
						 		//daca e online intoarce socketu-ul
								int so = find_sock_by_id(vector_id_subs, print[j].id); 
								if(so > 0){
									n = send(so, send_string, strlen(send_string), 0);
									DIE(n < 0, "send");			
								}
							}
						}
						else{
							//topicul nu exista => am creat topicul
							string a = topic;
							topics[a] = vector<struct id_socket> ();
						}

					}

					else if (i == sockfd){
						// a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
						// pe care serverul o accepta
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
						//mesajul este de la unul din clientii TCP
						//poate fi subscribe / unsubscribe sau disconnect
						memset(recv_buffer, 0, BUFLEN);
						n = recv(i, recv_buffer, sizeof(recv_buffer), 0);
						DIE(n < 0, "recv");
						
						if (n == 0){
							//clientul s-a deconectat
							char* id = find_id_by_sock(vector_id_subs, i);
							DIE(id == NULL, "find_id_by_sock");
							printf("Client (%s) disconnected.\n", id);
							off(vector_id_subs, i);
							close(i);
							FD_CLR(i, &read_fds);
						}
						else{
							//clientul a trimis o comanda de subscribe / unsubscribe
							if (strncmp(recv_buffer, "subscribe", 9) == 0){
								vector<string> elem = split(recv_buffer);
								if(elem.size() == 3){
									it = topics.find(elem[1]);
									if(it != topics.end()){
										//topicul deja exista => abonam clientul la topic
										it->second.push_back(find_elem_by_sock(vector_id_subs,i));
									}
									else{
										//topicul nu exista
										string a = elem[1];
										vector<struct id_socket> newVct;
										newVct.push_back(find_elem_by_sock(vector_id_subs,i));
										topics[a] = newVct;
									}
								}
								break;
							}
							else if (strncmp(recv_buffer, "unsubscribe", 11) == 0){
								recv_buffer[strlen(recv_buffer) - 1] = '\0';
								vector<string> elem = split(recv_buffer);
								it = topics.find(elem[1]);
								if(it != topics.end()){
									unsubs(it->second,i);
								}
								break;
							}
							else{
								//clientul si-a trimis id-ul, iar acesta este procesat
								struct id_socket aux;
								aux.sock = i;
								strncpy(aux.id, recv_buffer, MAX_ID);
								//afisare conectare nou client
								printf("New client (%s) conncted from %s:%d.\n", 
								aux.id, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
								
								aux.online = 1;

								if( !actualizare_status(vector_id_subs, aux.id, aux.sock) ){
									//este prima aparitie a acestui id => prima conectare
									vector_id_subs.push_back(aux);
									topics["clients"].push_back(aux);
								}
							}
						}
					}

				}

			}

		}	
	}

	close(sockfd);
	close(sockfd_udp);

	return 0;
}