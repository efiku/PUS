

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char** argv) {

    WSADATA             wsaData;    /* Struktura, ktora otrzyma informacje na temat implementacji 'Windows Sockets'. */
    SOCKET              sockfd;     /* Deskryptor gniazda. */
    int                 retval;     /* Wartosc zwracana przez funkcje. */
	struct addrinfo hints, *res;
    int                 addr_len;   /* Dlugosc struktury adresowej w bajtach. */
    char                buff[256];  /* Bufor dla funkcji recv(). */


    if (argc != 3) {
        fprintf(stderr, "Invocation: %s <IPv4 ADDRESS> <PORT>\n", argv[0]);
        system("pause");
        exit(EXIT_FAILURE);
    }

    /* Inicjalizacja Winsock DLL przez proces: */
    retval = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (retval != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", retval);
        exit(EXIT_FAILURE);
    }

    /* Utworzenie gniazda dla protokolu TCP: */
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    /* Wyzerowanie struktury adresowej dla adresu zdalnego (serwera): */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET6;   /* Domena komunikacyjna (rodzina protokolow). */
	hints.ai_socktype = SOCK_STREAM;

	addr_len = getaddrinfo(argv[1], argv[2], &hints, &res);

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    /* Nawiazanie polaczenia (utworzenie asocjacji): */
	if (connect(sockfd, res->ai_addr, res->ai_addrlen) == INVALID_SOCKET) {
        fprintf(stderr, "connect() failed: %d\n", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    }

	struct sockaddr_storage addr_storage;

	// Zerujemy strukture
	memset(&addr_storage, 0, sizeof(addr_storage));

	int len = sizeof(struct sockaddr_storage);		
	retval = getsockname(sockfd, (struct sockaddr*) &addr_storage, &len);
	if(retval != 0) {
		fprintf(stderr, "getnameinfo() failed: %d\n", WSAGetLastError());
	}

	char host[NI_MAXHOST] = {0}, serv[NI_MAXSERV] = {0};
	retval = getnameinfo((struct sockaddr*) &addr_storage, len, 
		host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST);

	if(retval != 0) {
		fprintf(stderr, "getnameinfo() failed: %d\n", WSAGetLastError());
	}

	// Wypisujemy informacje o gnieüdzie
	printf("HOST: %s SERV: %s\n", host, serv);

	memset(&buff, 0, sizeof(buff));
    retval = recv(sockfd, buff, sizeof(buff), 0);

	printf("%s", buff);

    if (closesocket(sockfd) == SOCKET_ERROR) {
        fprintf(stderr, "closesocket() failed: %d\n", WSAGetLastError());
    }

    if (WSACleanup() == SOCKET_ERROR) {
        fprintf(stderr, "WSACleanup failed: %d\n", WSAGetLastError());
    }

    exit(EXIT_SUCCESS);
}
