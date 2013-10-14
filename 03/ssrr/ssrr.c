/*
 * Data:            2009-02-27
 * Autor:           Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Uruchamianie:    > ssrr.exe <adres IP lub nazwa domenowa>
 */

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <lib.h>

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
#pragma warning(disable : 4701)

int main(int argc, char **argv) {

    WSADATA             wsaData;                /* Struktura, ktora otrzyma informacje na temat implementacji 'Windows Sockets'. */
    SOCKET              sockfd;                 /* Deskryptor gniazda. */
    struct icmphdr      icmpheader  =   {0};    /* Naglowek ICMP. */
    struct addrinfo     hints;                  /* Struktura zawierajaca wskazowki dl funkcji getaddrinfo(). */
    struct addrinfo     *rp, *result;           /* Wskaznik na liste zwracana przez getaddrinfo() oraz wskaznik uzywany do poruszania sie po elementach listy. */
    int                 retval;                 /* Wartosc zwracana przez funkcje. */

    /*
     * Opcje IP:
     * NOP, SSRR, len, ptr, addr_IP_1, addr_IP_2, addr_IP_3
     */
    unsigned char       ip_options[16] = {1, 0x89,15,4, 192,168,1,1, 192,168,5,1, 192,168,0,1};

    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <HOSTNAME OR IP ADDRESS>\n", argv[0]);
        system("pause");
        exit(EXIT_FAILURE);
    }

    /* Inicjalizacja Winsock DLL przez proces: */
    retval = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (retval != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", retval);
        exit(EXIT_FAILURE);
    }

    /* Wskazowki dla getaddrinfo(): */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family     =   AF_INET; /* Domena komunikacyjna (rodzina protokolow). */
    hints.ai_socktype   =   SOCK_RAW; /* Typ gniazda. */
    hints.ai_protocol   =   IPPROTO_ICMP; /* Protokol. */

    /* Pierwszy argument to adres IP lub nazwa domenowa: */
    retval = getaddrinfo(argv[1], NULL, &hints, &result);
    if (retval != 0) {
        fprintf(stderr, "getaddrinfo() failed: %d\n", retval);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    /* Przechodzimy kolejno przez elementy listy: */
    for (rp = result; rp != NULL; rp = rp->ai_next) {

        /* Utworzenie gniazda dla protokolu ICMP: */
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == INVALID_SOCKET) {
            fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
            continue;
        }

        /* Ustawienie opcji IP (beda zawarte w kazdym wysylanym datagramie IP): */
        retval = setsockopt(sockfd, IPPROTO_IP, IP_OPTIONS, (const char*)ip_options, sizeof(ip_options));
        if (retval == SOCKET_ERROR) {
            fprintf(stderr, "setsockopt() failed: %d\n", WSAGetLastError());
            exit(EXIT_FAILURE);
        } else {
            /* Jezeli gniazdo zostalo poprawnie utworzone i opcje IP ustawione: */
            break;
        }
    }

    /* Jezeli lista jest pusta (nie utworzono gniazda): */
    if (rp == NULL) {
        fprintf(stderr, "Could not create socket.\n");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    /* Wypelnienie pol naglowka ICMP: */
    srand((unsigned int)time(NULL));
    /* Typ komunikatu: */
    icmpheader.type             =   ICMP_ECHO;
    /* Kod komunikatu: */
    icmpheader.code             =   0;  /* mozna pominac ze wzgledu na: struct icmphdr icmpheader = {0}; */
    /* Identyfikator: */
    icmpheader.un.echo.id       =   htons((unsigned short)GetCurrentProcessId());
    /* Numer sekwencyjny: */
    icmpheader.un.echo.sequence =   htons((unsigned short)rand());
    /* Suma kontrolna (plik lib.h): */
    icmpheader.checksum         =   internet_checksum((unsigned short*)&icmpheader, sizeof(icmpheader));

    fprintf(stdout, "Sending ICMP Echo...\n");

    /* Wysylanie komunikatu ICMP Echo: */
    retval = sendto(sockfd, (const char*)&icmpheader, sizeof(icmpheader), 0, rp->ai_addr, rp->ai_addrlen);
    if (retval == SOCKET_ERROR) {
        fprintf(stderr, "sentdo() failed: %d\n", WSAGetLastError());
    }

    /* Zwalniamy liste zaalokowana przez funkcje getaddrinfo(): */
    freeaddrinfo(result);

    if (closesocket(sockfd) == SOCKET_ERROR) {
        fprintf(stderr, "closesocket() failed: %d\n", WSAGetLastError());
    }

    if (WSACleanup() != 0) {
        fprintf(stderr, "WSACleanup failed.\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
