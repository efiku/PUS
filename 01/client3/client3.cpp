/*
 * Data:            2009-02-10
 * Autor:           Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Uruchamianie:    > client2.exe <adres IP> <numer portu> <wiadomosc>
 */

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char** argv) {

    WSADATA             wsaData;    /* Struktura, ktora otrzyma informacje na temat implementacji 'Windows Sockets'. */
    SOCKET              sockfd;     /* Deskryptor gniazda. */
    int                 retval;     /* Wartosc zwracana przez funkcje. */
    struct sockaddr_in  remote_addr;/* Struktura adresowa dla adresu zdalnego (serwera). */
    int                 addr_len;   /* Dlugosc struktury adresowej w bajtach. */
    char                buff[256];  /* Bufor dla funkcji recvfrom(). */
	char message[256]; //bufor na wysylana wiadomosc


    if (argc != 3) {
        fprintf(
            stderr,
            "Invocation: %s <IPv4 ADDRESS> <PORT>\n", argv[0]
        );
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

    /* Wyzerowanie struktury adresowej dla adresu zdalnego (serwera): */
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;   /* Domena komunikacyjna (rodzina protokolow). */

    /* Konwersja adresu IP z postaci czytelnej dla czlowieka (kropkowo-dziesietnej): */
    remote_addr.sin_addr.s_addr = inet_addr(argv[1]);
    if (remote_addr.sin_addr.s_addr == INADDR_NONE) {
        fprintf(stderr, "inet_addr(): INADDR_NONE\n");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    remote_addr.sin_port = htons(atoi(argv[2]));    /* Numer portu. */
    addr_len = sizeof(remote_addr);                 /* Rozmiar struktury adresowej w bajtach. */

	while(1)
	{
		printf("Message:\n");
		fgets(message, sizeof(message), stdin);

	
		/* Przyciecie wiadomosci, jezeli jest za dluga: */
		if (strlen(message) > 255) {
			fprintf(stdout, "Truncating message.\n");
			message[255] = '\0';
		}

		/* sendto() wysyla dane na adres okreslony przez strukture 'remote_addr': */
		retval = sendto(
					 sockfd,
					 message, strlen(message),
					 0,
					 (struct sockaddr*)&remote_addr, addr_len
				 );

		if (retval == SOCKET_ERROR) {
			fprintf(stderr, "sendto() failed: %d\n", WSAGetLastError());
			WSACleanup();
			exit(EXIT_FAILURE);
		}

		if(message[0] == '\n')
		{
			break;
		}

		/* Oczekiwanie na odpowiedz. Nie interesuje nas adres, z ktorego wyslano odpowiedz: */
		retval = recvfrom(sockfd, buff, sizeof(buff), 0, NULL, NULL);
		if (retval == SOCKET_ERROR) {
			fprintf(stderr, "recvfrom() failed: %d\n", WSAGetLastError());
			WSACleanup();
			exit(EXIT_FAILURE);
		}

		buff[retval] = '\0';

		fprintf(stdout, "Server response: '%s'\n", buff);
	}

    if (WSACleanup() == SOCKET_ERROR) {
        fprintf(stderr, "WSACleanup failed: %d\n", WSAGetLastError());
    }

    exit(EXIT_SUCCESS);
}
