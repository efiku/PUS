

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WS2tcpip.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char** argv) {

    /* Struktura, ktora otrzyma informacje na temat implementacji 'Windows Sockets': */
    WSADATA             wsaData;

    int                 retval;     /* Wartosc zwracana przez funkcje. */

    /* Deskryptory dla gniazda nasluchujacego i polaczonego: */
    SOCKET              listenfd, connfd;

    /* Gniazdowe struktury adresowe (dla klienta i serwera): */
    struct sockaddr_in6  server_addr, client_addr;

    /* Rozmiar struktur w bajtach: */
    int                 client_addr_len, server_addr_len;

    /* Bufor wykorzystywany przez funkcje send() i recv(): */
    char                buff[256];

    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <PORT>\n", argv[0]);
        system("pause");
        exit(EXIT_FAILURE);
    }

    /* Inicjalizacja 'Windows Sockets' DLL: */
    retval = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (retval != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", retval);
        exit(EXIT_FAILURE);
    }

    /* Utworzenie gniazda dla protokolu TCP: */
	listenfd = socket(PF_INET6, SOCK_STREAM, 0);
    if (listenfd == INVALID_SOCKET) {
        fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    /* Wyzerowanie struktury adresowej serwera: */
    memset(&server_addr, 0, sizeof(server_addr));
    /* Domena komunikacyjna (rodzina protokolow): */
    server_addr.sin6_family      =   AF_INET6;
    /* Adres nieokreslony (ang. wildcard address): */
	server_addr.sin6_addr =   in6addr_any;
    /* Numer portu: */
    server_addr.sin6_port        =   htons(atoi(argv[1]));
    /* Rozmiar struktury adresowej serwera w bajtach: */
    server_addr_len             =   sizeof(server_addr);

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

    client_addr_len = sizeof(client_addr); /* Rozmiar struktury adresowej dla adresu klienta. */

	while(1)
	{
		/* Funkcja pobiera polaczenie z kolejki polaczen oczekujacych na zaakceptowanie
		 * i zwraca deskryptor dla gniazda polaczonego: */
		connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_len);
		if (connfd == INVALID_SOCKET) {
			fprintf(stderr, "accept() failed: %d\n", WSAGetLastError());
			closesocket(listenfd);
			WSACleanup();
			exit(EXIT_FAILURE);
		}
		char clienthost [NI_MAXHOST];
		char clientservice [NI_MAXSERV];
		getnameinfo((struct sockaddr*)&client_addr, client_addr_len, clienthost, sizeof(clienthost), clientservice, sizeof(clientservice), NI_NUMERICHOST);

		fprintf(
			stdout, "TCP connection accepted from %s:%d\n",
			clienthost,
			clientservice
		);

		char buff [256];
		strcpy(buff, "Laboratorium PUS");
		//wys³anie wiadomoœci do klienta
		retval = send(connfd, buff, strlen(buff), 0);
		if (closesocket(connfd) == SOCKET_ERROR) {
        fprintf(stderr, "closesocket() failed: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    if (closesocket(listenfd) == SOCKET_ERROR) {
        fprintf(stderr, "closesocket() failed: %d\n", WSAGetLastError());
    }

    if (WSACleanup() == SOCKET_ERROR) {
        fprintf(stderr, "WSACleanup failed: %d\n", WSAGetLastError());
    }
    exit(EXIT_SUCCESS);
}
