/*
 * Data:            2009-02-10
 * Autor:           Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Uruchamianie:    > server2.exe <numer portu>
 */

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
	#include "libpalindrome.h"
}

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char** argv) {

    /* Struktura, ktora otrzyma informacje na temat implementacji 'Windows Sockets': */
    WSADATA             wsaData;

    int                 retval;     /* Wartosc zwracana przez funkcje. */

    SOCKET              sockfd;     /* Deskryptor gniazda */

    /* Gniazdowe struktury adresowe (dla klienta i serwera): */
    struct sockaddr_in  server_addr, client_addr;

    /* Rozmiar struktur w bajtach: */
    int                 client_addr_len, server_addr_len;

    /* Bufor wykorzystywany przez funkcje recvfrom() i sendto(): */
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

    /* Utworzenie gniazda dla protokolu UDP: */
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd == INVALID_SOCKET) {
        fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    /* Wyzerowanie struktury adresowej serwera: */
    memset(&server_addr, 0, sizeof(server_addr));
    /* Domena komunikacyjna (rodzina protokolow): */
    server_addr.sin_family      =   AF_INET;
    /* Adres nieokreslony (ang. wildcard address): */
    server_addr.sin_addr.s_addr =   htonl(INADDR_ANY);
    /* Numer portu: */
    server_addr.sin_port        =   htons(atoi(argv[1]));
    /* Rozmiar struktury adresowej serwera w bajtach: */
    server_addr_len             =   sizeof(server_addr);

    /* Powiazanie lokalnego adresu IP i numeru portu z gniazdem: */
    if (bind(sockfd, (struct sockaddr*) &server_addr, server_addr_len) == SOCKET_ERROR) {
        fprintf(stderr, "bind() failed: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Server is listening for incoming connection...\n");

    client_addr_len = sizeof(client_addr);

	while(1)
	{
		/* Oczekiwanie na dane od klienta: */
		retval = recvfrom(
					 sockfd,
					 buff, sizeof(buff),
					 0,
					 (struct sockaddr*)&client_addr, &client_addr_len
				 );

		if (retval == SOCKET_ERROR) {
			fprintf(stderr, "recvfrom() failed: %d\n", WSAGetLastError());
			WSACleanup();
			exit(EXIT_FAILURE);
		}

		fprintf(stdout, "UDP datagram received from %s:%d. Echoing message...\n",
				inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)
			   );

		if(buff[0] == '\n')
		{
			break;
		}
		char resp[256];

		if(is_palindrome(buff, retval) == -1)
		{
			strcpy(resp, "nieprawidlowy ciag");
		}
		else if(is_palindrome(buff, retval) == 0)
		{
			strcpy(resp, "ciag nie jest palindromem");
		}
		else
		{
			strcpy(resp, "ciag nie jest palindromem");
		}

		/* Wyslanie odpowiedzi (echo): */
		retval = sendto(
					 sockfd,
					 resp, strlen(resp),
					 0,
					 (struct sockaddr*)&client_addr, client_addr_len
				 );

		if (retval == SOCKET_ERROR) {
			fprintf(stderr, "sendto() failed: %d\n", WSAGetLastError());
			WSACleanup();
			exit(EXIT_FAILURE);
		}

	}

    if (closesocket(sockfd) == SOCKET_ERROR) {
        fprintf(stderr, "closesocket() failed: %d\n", WSAGetLastError());
    }

    if (WSACleanup() == SOCKET_ERROR) {
        fprintf(stderr, "WSACleanup failed: %d\n", WSAGetLastError());
    }

    exit(EXIT_SUCCESS);
}
