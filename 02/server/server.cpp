/*
 * Data:            2009-02-10
 * Autor:           Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Uruchamianie:    > server1.exe <numer portu>
 */

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char** argv) {

    /* Struktura, ktora otrzyma informacje na temat implementacji 'Windows Sockets': */
    WSADATA             wsaData;

    int                 retval;     /* Wartosc zwracana przez funkcje. */

    /* Deskryptory dla gniazda nasluchujacego i polaczonego: */
    SOCKET              listenfd, connfd;

    struct addrinfo hints;
	struct addrinfo* result;

    /* Rozmiar struktur w bajtach: */
    int                 client_addr_len, server_addr_len;

    /* Bufor wykorzystywany przez funkcje send() i recv(): */
    char                buff[256];

    time_t              rawtime;
    struct tm*          timeinfo;

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
    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if (listenfd == INVALID_SOCKET) {
        fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    /* Wyzerowanie struktury adresowej serwera: */
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family      =   AF_UNSPEC;
    hints.ai_socktype =   SOCK_STREAM;
	hints.ai_protocol        =   IPPROTO_TCP;
	hints.ai_flags             =   AI_PASSIVE;

	retval = getaddrinfo(NULL, argv[1], &hints, &result);

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

    /* Funkcja pobiera polaczenie z kolejki polaczen oczekujacych na zaakceptowanie
     * i zwraca deskryptor dla gniazda polaczonego: */
    connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_len);
    if (connfd == INVALID_SOCKET) {
        fprintf(stderr, "accept() failed: %d\n", WSAGetLastError());
        closesocket(listenfd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    fprintf(
        stdout, "TCP connection accepted from %s:%d\n",
        inet_ntoa(client_addr.sin_addr),
        ntohs(client_addr.sin_port)
    );

    Sleep(6000);

    fprintf(stdout, "Sending current date and time...\n");

    Sleep(2000);

    /* Zapisanie w buforze 'buff' aktualnego czasu: */
    time(&rawtime);

#pragma warning(disable : 4996)
    timeinfo = localtime(&rawtime);
#pragma warning(default : 4996)

    strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", timeinfo);

    /* Wyslanie aktualnego czasu do klienta: */
    retval = send(connfd, buff, strlen(buff), 0);

    /* Funkcja oczekuje na segment TCP (od klienta) z ustawiona flaga FIN: */
    retval = recv(connfd, buff, sizeof(buff), 0);
    if (retval == 0) {
        Sleep(4000);
        fprintf(stdout, "Connection terminated by client "
                "(received FIN, entering CLOSE_WAIT state on connected socked)...\n");
    }

    Sleep(12000);

    fprintf(stdout, "Closing connected socket (sending FIN to client)...\n");
    if (closesocket(connfd) == SOCKET_ERROR) {
        fprintf(stderr, "closesocket() failed: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    Sleep(5000);

    fprintf(stdout, "Closing listening socket and terminating server...\n");
    if (closesocket(listenfd) == SOCKET_ERROR) {
        fprintf(stderr, "closesocket() failed: %d\n", WSAGetLastError());
    }

    Sleep(3000);

    if (WSACleanup() == SOCKET_ERROR) {
        fprintf(stderr, "WSACleanup failed: %d\n", WSAGetLastError());
    }
    exit(EXIT_SUCCESS);
}
