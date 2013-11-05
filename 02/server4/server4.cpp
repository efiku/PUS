#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[]) {
    int retval = 0;

	WSADATA wsaData;	
    if ((retval = WSAStartup(MAKEWORD(2,2), &wsaData)) != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", retval);
        exit(EXIT_FAILURE);
    }	

    /* Utworzenie gniazda dla protokolu TCP: */
    SOCKET sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
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

    if (bind(sockfd, (struct sockaddr*) &server_addr, server_addr_len) == SOCKET_ERROR) {
        fprintf(stderr, "bind() failed: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    /* Przeksztalcenie gniazda w gniazdo nasluchujace: */
    if (listen(sockfd, SOMAXCONN) == SOCKET_ERROR) {
        fprintf(stderr, "listen() failed: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

	while(true) {
		/* Próba nawi¹zania nowego po³¹czenia z serwerem */
		struct sockaddr_in client_addr;
		/* Rozmiar struktury adresowej dla adresu klienta. */
		int client_addr_len = sizeof(client_addr); 

		/* Funkcja pobiera polaczenie z kolejki polaczen oczekujacych na zaakceptowanie
			* i zwraca deskryptor dla gniazda polaczonego: */
		SOCKET connfd = accept(sockfd, (struct sockaddr*) &client_addr, &client_addr_len);
		if (connfd == INVALID_SOCKET) {
			fprintf(stderr, "accept() failed: %d\n", WSAGetLastError());
			closesocket(sockfd);
			WSACleanup();
			exit(EXIT_FAILURE);
		}

		printf("TCP connection accepted from %s:%d\n", 
			inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));	

		const char* message = "Laboratorium PUS";

		if((retval = send(connfd, message, strlen(message), 0)) == SOCKET_ERROR) {
			// Nieuda³o siê dostarczenie wiadomoœci
			fprintf(stderr, "send() failed: %d\n", WSAGetLastError());
			closesocket(connfd);
			return 0;
		}

		if (closesocket(connfd) == SOCKET_ERROR) {
			fprintf(stderr, "closesocket() failed: %d\n", WSAGetLastError());
		}	
	}

    if (closesocket(sockfd) == SOCKET_ERROR) {
        fprintf(stderr, "closesocket() failed: %d\n", WSAGetLastError());
    }

    if (WSACleanup() == SOCKET_ERROR) {
        fprintf(stderr, "WSACleanup failed: %d\n", WSAGetLastError());
    }

	system("pause");

	return EXIT_SUCCESS;
}