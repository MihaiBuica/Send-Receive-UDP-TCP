#ifndef _HELPERS_H
#define _HELPERS_H 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <map>

#define MAX_ID 10
#define BUFLEN		1551	// dimensiunea maxima a calupului de date
#define MAX_CLIENTS	1000	// numarul maxim de clienti in asteptare
#define MESAJFINAL 2000
using namespace std;
	
//structura ce reprezinta clientul
struct id_socket{
	char id[MAX_ID];
	int sock;
	int online;
}id_sock;

//functie pentru parsarea comenzilor in functie de spatiu
vector<string> split (char array[BUFLEN]){
	vector<string> rezultat;
	string s = array;
	string delimiter = " ";
	size_t pos = 0;
	string token;
	while((pos = s.find(delimiter)) != string::npos){
		token = s.substr(0, pos);
		rezultat.push_back(token);
		s.erase(0, pos + delimiter.length());
	}
	rezultat.push_back(s);
	return rezultat;
}

/*
 * Macro de verificare a erorilor
 * Exemplu:
 *     int fd = open(file_name, O_RDONLY);
 *     DIE(fd == -1, "open failed");
 */
#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

#endif
