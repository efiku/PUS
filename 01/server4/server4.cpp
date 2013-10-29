#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

// Typ reprezentuj¹cy ID wiadomoœci
typedef unsigned int msgid_t;

// Maksymalny rozmiar wiadomoœci
const size_t MAX_MESSAGE_SIZE = 1024;

/**
 * Struktura reprezentuj¹ca pojedyñczego po³aczonego klienta
 */
struct Client {
	SOCKET fd;	
	// ID ostatnio odebranej wiadomoœci 
	msgid_t lastId;
	// Bufor na wiadomoœci
	char buff[MAX_MESSAGE_SIZE];
	// Rozmiar danych w buforze
	size_t len;

	Client(SOCKET fd) : fd(fd), lastId(0), len(0) {};

	// Czyœci bufor klienta 
	void clear() { memset(buff, 0, len); len = 0; }
};

typedef std::vector<Client> ClientsList;

struct Message {
	// Identyfikator wiadomoœci
	msgid_t id;
	// Identyfikator authora
	int author;
	// Czas odebrania wiadomoœci
	time_t recived;
	// D³ugoœæ wiadomoœci
	size_t len;
	// Treœæ wiadomoœci
	char message[MAX_MESSAGE_SIZE];
};

class MessagesManager {
	// Otrzymane wiadomoœci
	std::vector<Message> messages;
	// Odstatnio wygenerowane ID
	msgid_t lastId;

public:	
	MessagesManager() : lastId(0) {};

	// Dodaje wiadomoœæ
	msgid_t push(Message msg) {
		// Generujemy ID wiadomoœci 
		msg.id = getNextId();
		// Zapisujemy czas odebrania wiadomoœci
		msg.recived = time(NULL);
		// Dodajemy wiadomoœæ
		messages.push_back(msg);

		return msg.id;
	}

	const Message* getNextMessage(int author, msgid_t lastId) const {
		for(int i = 0; i < messages.size(); i++) {
			if(lastId < messages[i].id && author != messages[i].author) {
				return &messages[i];
			}
		}

		return NULL;
	}

private:
	msgid_t getNextId() { return ++lastId; }; 
};

int main(int argc, char* argv[]) {
	int retval = 0;

	if(argc < 2) {
		fprintf(stderr, "Uzycie: %s <port>", argv[0]);
		system("pause");
		return EXIT_FAILURE;
	}

	WSADATA wsaData;
    /* Inicjalizacja 'Windows Sockets' DLL: */
    retval = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (retval != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", retval);
        exit(EXIT_FAILURE);
    }

    /* Utworzenie gniazda dla protokolu TCP: */
    SOCKET listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if (listenfd == INVALID_SOCKET) {
        fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    }

	struct sockaddr_in server_addr;
    /* Wyzerowanie struktury adresowej serwera: */
    memset(&server_addr, 0, sizeof(server_addr));
    /* Domena komunikacyjna (rodzina protokolow): */
    server_addr.sin_family      =   AF_INET;
    /* Adres nieokreslony (ang. wildcard address): */
    server_addr.sin_addr.s_addr =   htonl(INADDR_ANY);
    /* Numer portu: */
    server_addr.sin_port        =   htons(atoi(argv[1]));
    /* Rozmiar struktury adresowej serwera w bajtach: */
    int server_addr_len         =   sizeof(server_addr);

    /* Powiazanie lokalnego adresu IP i numeru portu z gniazdem: */
    if (bind(listenfd, (struct sockaddr*) &server_addr, server_addr_len) == SOCKET_ERROR) {
        fprintf(stderr, "bind() failed: %d\n", WSAGetLastError());
        closesocket(listenfd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    /* Przeksztalcenie gniazda w gniazdo nasluchujace: */
    if (listen(listenfd, SOMAXCONN) == SOCKET_ERROR) {
        fprintf(stderr, "listen() failed: %d\n", WSAGetLastError());
        closesocket(listenfd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Server is listening for incoming connection...\n");

	/* Klienci */
	ClientsList clients;
	/* Kolejka wiadomoœci */
	MessagesManager msgs;
		 
	/*  */
	struct fd_set rfds, wfds, efds;

	while(true) {
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		FD_ZERO(&efds);
		
		FD_SET(listenfd, &rfds);
		FD_SET(listenfd, &wfds);
		FD_SET(listenfd, &efds);

		for(int i = 0; i < clients.size(); i++) {
			Client& client = clients[i];
			
			FD_SET(client.fd, &rfds);
			FD_SET(client.fd, &wfds);
			FD_SET(client.fd, &efds);
		}

		if(select(0, &rfds, &wfds, &efds, 0) > 0) {
			if(FD_ISSET(listenfd, &rfds)) {
				/* Próba nawi¹zania nowego po³¹czenia z serwerem */
				struct sockaddr_in client_addr;
				/* Rozmiar struktury adresowej dla adresu klienta. */
				int client_addr_len = sizeof(client_addr); 

				/* Funkcja pobiera polaczenie z kolejki polaczen oczekujacych na zaakceptowanie
				 * i zwraca deskryptor dla gniazda polaczonego: */
				SOCKET connfd = accept(listenfd, (struct sockaddr*) &client_addr, &client_addr_len);
				if (connfd == INVALID_SOCKET) {
					fprintf(stderr, "accept() failed: %d\n", WSAGetLastError());
					closesocket(listenfd);
					WSACleanup();
					exit(EXIT_FAILURE);
				}

				printf("TCP connection accepted from %s:%d\n", 
					inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));	
				
				clients.push_back(Client(connfd));
			}

			for(int i = 0; i < clients.size(); i++) {
				Client& client = clients[i];

				if(FD_ISSET(client.fd, &rfds)) {					
					/* Odczytujemy wiadomoœæ od klienta */				
					if((retval = recv(client.fd, client.buff + client.len, 
						MAX_MESSAGE_SIZE - client.len, 0)) != SOCKET_ERROR) 
					{
						client.len += retval;

						// Sprawdzamy czy w buforze znajduje siê znak koñca linii
						if(client.buff[client.len - 1] == '\n') {
							Message msg;

							msg.author = i;
							msg.len = client.len;
							memcpy(msg.message, client.buff, client.len);														

							/* Dodajemy wiadomoœæ do wys³ania */
							msgs.push(msg);
							/* Czyœcimy bufor klienta */
							client.clear();
						}
					}
					else {
						// Koniec po³¹czenia 
						closesocket(client.fd);
						client.fd = INVALID_SOCKET;
						
						fprintf(stderr, "Klient sie rozlaczyl!\n");
					}
				}

				if(FD_ISSET(client.fd, &wfds)) {					
					const Message* msg = NULL;
					
					if((msg = msgs.getNextMessage(i, client.lastId)) != NULL) {
						/* Wysy³amy nastêpn¹ wiadomoœæ do klienta */
						if((retval = send(client.fd, msg->message, msg->len, 0)) == SOCKET_ERROR) {
							// B³¹d podczas wysy³ania wiadomoœci
							fprintf(stderr, "B³¹d podczas wysy³ania wiadomosci\n");
						}

						client.lastId = msg->id;
					}					
				}
			}
		}
	}

    if (WSACleanup() == SOCKET_ERROR) {
        fprintf(stderr, "WSACleanup failed: %d\n", WSAGetLastError());
    }
	
	system("PAUSE");

	return 0;
}
