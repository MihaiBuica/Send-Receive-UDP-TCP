============== TEMA 2 - PC ==============
Buica Mihai-Victoras, 322CB
Continut arhiva: README, server.cpp, subscriber.cpp, helpers.h.
Schelet: Laboratorul 8.

==== DETALII IMPLEMENTARE ====

-Server:
	Serverul primeste mesaje prin intermediul udp (de la clientul udp), pe care le proceseaza si le clasifica in functie de topic, dupa care le trimite prin intermediul tcp(catre clientii tcp). Poate primii si mesaje de la clientii tcp, cum ar fi subscribe/unsubscribe sau atunci cand un client se deconecteaza sau mesaje de la stdin, comanda exit. Pentru functionalitatea ceruta in enunt, serverul deschide doi socketi (unul TCP si unul UDP) pe care ii adauga in multimea folosita de select, impreuna cu socketul 0, pentru stdin. 
	Asupra multimii de socketi se executa select, iar apoi se verifica fiecare socket si se detecteaza cel pe care s-a primit un mesaj.
		1. socketul 0 => inseamna ca s-a primit un mesaj de la stdin. Acest mesaj se va procesa doar daca este comanda exit. In caz afirmativ, fiecarui client ce este conectat in acel moment la server, i se trimite mesajul exit(pe care si el il va procesa) si se inchide conexiunea cu acesta(socket-ul este inchis si este scos din multimea pentru select). La final se inchid si socketii pentru conexiuni noi TCP si pentru UDP, iar programul se termina. Pentru a retine clientii conectati la server, se stocheaza un vector de clienti. 
		
		2. socketul sockfd_udp => inseamna ca s-a primit un mesaj pe socket-ul dedicat clientilor UDP. Dupa ce mesajul este primit, acesta trebuie parsat in functie de tipul lui. In prima faza, se extrage topicul mesajului si se adauga la payload(care deja are ip-ul si portul clientului udp). Apoi, in functie de tipul primit:
			a) tip 0 => INT. Se extrage din zona specifica de memorie un uint32_t(de la pozitia 52 din buffer) si se extrage si bitul de semn (pozitia 51). Avand aceste doua componente(numarul si bitul de semn), se construieste numarul si se adauga in payload impreuna cu tipul.
			b) tip 1 => SHORT_REAL. Se extrage din zona de memorie (pozitia 51 pentru ca nu are bit de semn) un uint16_t, ce se imparte la 100(pentru a-i afla si partea zecimala) si se adauga in payload.
			c) tip 2 => FLOAT. In acest caz, sunt preluate: bitul de semn, numarul si numarul de zecimale. Se construieste numarul find inmultit cu 10 la puterea (- nr de zecimale), dupa care se adauga la payload acest numar folosind sprintf si %.*f pentru numarul de zecimale.
			d) tip 3 => STRING.  Este copiat string-ul primit si adaugat in payload.
			Fiecare numar este transformat folosind ntohl/ntohs din network in host order.

		Dupa ce string-ul a fost parsat, se verifica daca topicul primit exista. In caz afirmativ, mesajul este trimis carte toti abonatii la acel topic, altfel, topicul se creeaza. Pentru aceasta utilitate, am folosit map. Fiecare topic are un vector de clienti ce sunt abonati la acesta. Atunci cand se face trimiterea, se verifica daca clientul este online, iar in caz afirmativ, mesajul ii este trimis. (pentru a fi actualizate datele, acestea se preiau in functie de id)
		
		Payload = mesajul ce va fi trimis la clienti. Se constuieste exact sub forma ceruta in enunt, clientii trebuind doar sa il afiseze. Acest lucru permite ca un client sa parseze mesajul doar daca are nevoie de anumite date, altfel (si in cazul nostru) trebuie doar sa afiseze ceea ce primeste de la server. 

		3. socketul sockfd => socketul pe care se conecteaza noi clienti. Daca este primit un mesaj pe acest socket, inseamna ca un nou client se conecteaza. Acesta este acceptat iar socketul pe care a fost primit, se adauga in multimea de socketi. 

		4.mesajul vine de pe un alt socket, atunci este unul primit de la clientii tcp. Daca se intoarce 0 la primirea unui mesaj, inseamna ca acel client se deconecteaza => este marcat ca offline(online = 0), se inchide conexiunea cu acel socket si se elimina din multimea socketilor. Altfel, daca mesajul primit contine cuvantul "subscribe", se face split mesajului dupa spatii, se identifica topicul la care se doreste sa se aboneze, iar in caz ca acest topic exista, clientul este adaugat in vectorul specific topicului respectiv, altfel se creeaza acel topic. Daca mesajul primit contine cuvantul "unsubscribe", atunci clientul ce a trimis acest mesaj, este scos din vectorul pentru topicul de la care se dezaboneaza (topicul specificat de unsubscribe). Daca mesajul nu contine niciunul dintre aceste doua cuvinte, atunci clientul isi trimite id-ul. Se verifica daca acest id a mai existat (daca s-a mai conectat inainte). In caz afirmativ, in structura veche ii sunt actualizate online (ce era 0 si acum devine 1) si noul socket de pe care s-a conectat. In caz negativ, noul client este adaugat vectorului de clienti.

	Pentru server, un client = o structura ce retine id, socket si online. Online reprezinta daca clientul este conectat sau nu in acel moment la server.  

-Subscriber:
	Clientul TCP, deschide un socket pe care poate primii date de la server si se conecteaza la server prin ip-ul si portul primite ca parametrii.
	1. Mesaje primite de la server: acestea pot fi: comanda exit, cand o primeste pe aceasta, clientul se inchide; sau un string de afisat, daca nu este comanda exit, atunci clientul trebuie sa afiseze ceea ce a primit de la server, deoarece string-ul este deja parsat.
	2. Mesaje de la stdin. Poate fi: exit => clientul se inchide, subscribe / unsubscribe => se trimite aceasta comanda catre server, dupa care se afiseaza un mesaj caracteristic,

-Helpers:
	Contine functia de parsare, constante si biblioteci. (parsarea este folosita atat de client cat si de server).

Ideea de implementare a constat in a mentine un client cat mai simplu. Serverul face parsarea si construirea numerelor, iar clientul doar afiseaza, deoarece in acest caz mi s-a parut mai eficient acest mod(daca sunt 10 clienti, se va face o parsare si 10 afisari / daca clientii parsau => 10 parsari si 10 afisari).

==== CAZ NETRATAT ====
SF = 1 => acest caz nu este tratat de catre program.

==== RULARE ====
make
./server <port>
./subscriber <id> <ip_server> <port_server>
./client_udp <....>
dupa se pot efectua comenzi de subscribe, iar clientul udp sa trimita mesaje.