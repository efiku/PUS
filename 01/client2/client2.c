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


    if (argc != 4) {
        fprintf(
            stderr,
            "Invocation: %s <IPv4 ADDRESS> <PORT> <MESSAGE>\n", argv[0]
        );
        system("pause");
        exit(EXIT_FAILURE);
    }

    /* Przyciecie wiadomosci, jezeli jest za dluga: */
    if (strlen(argv[3]) > 255) {
        fprintf(stdout, "Truncating message.\n");
        argv[3][255] = '\0';
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

    Sleep(1000);

    fprintf(
        stdout,
        "Sending message to %s.\nWaiting for server response...\n", argv[1]
    );

    Sleep(2000);
    /* sendto() wysyla dane na adres okreslony przez strukture 'remote_addr': */
    retval = sendto(
                 sockfd,
                 argv[3], strlen(argv[3]),
                 0,
                 (struct sockaddr*)&remote_addr, addr_len
             );

    if (retval == SOCKET_ERROR) {
        fprintf(stderr, "sendto() failed: %d\n", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
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

    if (WSACleanup() == SOCKET_ERROR) {
        fprintf(stderr, "WSACleanup failed: %d\n", WSAGetLastError());
    }

    exit(EXIT_SUCCESS);
}
