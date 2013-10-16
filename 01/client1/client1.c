/*
 * Data:            2009-02-10
 * Autor:           Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Uruchamianie:    > client1.exe <adres IP> <numer portu>
 */

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* MSDN:
 * Implicit linking is sometimes referred to as static load or load-time dynamic linking.
 *
 * With implicit linking, the executable using the DLL
 * links to an import library (.lib file) provided by the maker of the DLL.
 * (...)
 * The linker creates the import library when the DLL is built.
 * (...)
 * The operating system loads the DLL when the executable using it is loaded.
 * The client executable calls the DLL's exported functions just as
 * if the functions were contained within the executable.
 */
#pragma comment(lib, "ws2_32.lib")

int main(int argc, char** argv) {

    WSADATA             wsaData;    /* Struktura, ktora otrzyma informacje na temat implementacji 'Windows Sockets'. */
    SOCKET              sockfd;     /* Deskryptor gniazda. */
    int                 retval;     /* Wartosc zwracana przez funkcje. */
    struct sockaddr_in  remote_addr;/* Struktura adresowa dla adresu zdalnego (serwera). */
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

    /* Stosowanie funkcji Sleep() pozwala lepiej zaobserwowac three-way handshake w snifferze. */
    Sleep(1000);

    /* Nawiazanie polaczenia (utworzenie asocjacji): */
    if (connect(sockfd, (const struct sockaddr*) &remote_addr, addr_len) == INVALID_SOCKET) {
        fprintf(stderr, "connect() failed: %d\n", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    }


    Sleep(3000);

    fprintf(stdout, "After three-way handshake. Waiting for server response...\n");

    /* Odebranie danych: */
    retval = recv(sockfd, buff, sizeof(buff), 0);
    Sleep(1000);
    fprintf(stdout, "Received server response: %s\n", buff);

    Sleep(4000);

    /* Zamkniecie polaczenia TCP: */
    fprintf(stdout, "Closing socket (sending FIN to server)...\n");
    if (closesocket(sockfd) == SOCKET_ERROR) {
        fprintf(stderr, "closesocket() failed: %d\n", WSAGetLastError());
    }

    Sleep(9000);

    /* Po zakonczeniu aplikacji, gniazdo przez okreslony czas (2 * MSL) bedzie w stanie TIME_WAIT: */
    fprintf(stdout, "Terminating application. After receiving FIN from server, "
            "TCP connection will go into TIME_WAIT state.\n");

    Sleep(4000);

    if (WSACleanup() == SOCKET_ERROR) {
        fprintf(stderr, "WSACleanup failed: %d\n", WSAGetLastError());
    }

    exit(EXIT_SUCCESS);
}
