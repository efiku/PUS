/*
 * Data:            2009-02-27
 * Autor:           Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Uruchamianie:    > udp.exe <adres IP lub nazwa domenowa> <numer portu>
 */

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <lib.h> /* Project->Properties->Configuration Properties->C/C++->General->Additional Include Directories */

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4701)

#define SOURCE_PORT 5050
#define SOURCE_ADDRESS "192.168.0.100"

int main(int argc, char** argv) {

    WSADATA             wsaData;            /* Struktura, ktora otrzyma informacje na temat implementacji 'Windows Sockets'. */
    SOCKET              sockfd;             /* Deskryptor gniazda. */
    struct addrinfo     hints;              /* Struktura zawierajaca wskazowki dl funkcji getaddrinfo(). */
    struct addrinfo     *rp, *result;       /* Wskaznik na liste zwracana przez getaddrinfo() oraz wskaznik uzywany do poruszania sie po elementach listy. */
    int                 retval;             /* Wartosc zwracana przez funkcje. */
    int                 socket_option;      /* Zmienna wykorzystywana do ustawiania opcji gniazda. */
    unsigned short      checksum;           /* Zmienna wykorzystywana do obliczenia sumy kontrolnej. */

    /* Bufor na naglowek IP, naglowek UDP oraz pseudo-naglowek: */
    unsigned char       datagram[sizeof(struct ip) + sizeof(struct udphdr) + sizeof(struct phdr)];

    /* Wskaznik na naglowek IP (w buforze okreslonym przez 'datagram'): */
    struct ip           *ip_header          = (struct ip *)datagram;

    /* Wskaznik na naglowek UDP (w buforze okreslonym przez 'datagram'): */
    struct udphdr       *udp_header         = (struct udphdr *)(datagram + sizeof(struct ip));

    /* Wskaznik na pseudo-naglowek (w buforze okreslonym przez 'datagram'): */
    struct phdr         *pseudo_header      = (struct phdr *)(datagram + sizeof(struct ip) + sizeof(struct udphdr));

    /* Sprawdzenie argumentow wywolania: */
    if (argc != 3) {
        fprintf(stderr, "Invocation: %s <HOSTNAME OR IP ADDRESS> <PORT>\n", argv[0]);
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
    hints.ai_family     =   AF_INET;        /* Domena komunikacyjna (IPv4). */
    hints.ai_socktype   =   SOCK_RAW;       /* Typ gniazda. */
    hints.ai_protocol   =   IPPROTO_UDP;    /* Protokol. */

    retval = getaddrinfo(argv[1], NULL, &hints, &result);
    if (retval != 0) {
        fprintf(stderr, "getaddrinfo() failed: %d\n", retval);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    /* Opcja okreslona w wywolaniu setsockopt() zostanie wlaczona: */
    socket_option = 1;

    /* Przechodzimy kolejno przez elementy listy: */
    for (rp = result; rp != NULL; rp = rp->ai_next) {

        /* Utworzenie gniazda dla protokolu UDP: */
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == INVALID_SOCKET) {
            fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
            continue;
        }

        /* Ustawienie opcji IP_HDRINCL: */
        retval = setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, (const char*)&socket_option, sizeof(int));
        if (retval == SOCKET_ERROR) {
            fprintf(stderr, "setsockopt() failed: %d\n", WSAGetLastError());
            exit(EXIT_FAILURE);
        } else {
            /* Jezeli gniazdo zostalo poprawnie utworzone i opcja IP_HDRINCL ustawiona: */
            break;
        }
    }

    /* Jezeli lista jest pusta (nie utworzono gniazda): */
    if (rp == NULL) {
        fprintf(stderr, "Could not create socket.\n");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    /********************************/
    /* Wypelnienie pol naglowka IP: */
    /********************************/
    ip_header->ip_hl                =   5; /* 5 * 32 bity = 20 bajtow */
    ip_header->ip_v                 =   4; /* Wersja protokolu (IPv4). */
    ip_header->ip_tos               =   0; /* Pole TOS wyzerowane. */
    ip_header->ip_len               =   0; /* Dlugosc (naglowek + dane); pole zawsze wypelniane przez system. */
    ip_header->ip_id                =   0; /* Pole Identification; wypelniane przez systemFilled in when zero. */
    ip_header->ip_off               =   0; /* Pole Fragment Offset. */
    ip_header->ip_ttl               =   128; /* TTL */

    /* Identyfikator enkapsulowanego protokolu: */
    ip_header->ip_p                 =   IPPROTO_UDP;

    /* Adres zrodlowy: */
    ip_header->ip_src.s_addr        =   inet_addr(SOURCE_ADDRESS);

    /* Adres docelowy: */
    ip_header->ip_dst.s_addr        = ((struct sockaddr_in*)rp->ai_addr)->sin_addr.s_addr;

    /* ip_header->ip_sum            =   0; */ /* Zawsze wypelniane przez system. */

    /*********************************/
    /* Wypelnienie pol naglowka UDP: */
    /*********************************/

    /* Port zrodlowy: */
    udp_header->uh_sport            =   htons(SOURCE_PORT);
    /* Port docelowy: */
    udp_header->uh_dport            =   htons((unsigned short)atoi(argv[2]));
    /* Rozmiar naglowka UDP i danych. W tym przypadku tylko naglowka:  */
    udp_header->uh_ulen             =   htons(sizeof(struct udphdr));

    /************************************/
    /* Wypelnienie pol pseudo-naglowka: */
    /************************************/

    /* Zrodlowy adres IP: */
    pseudo_header->ip_src.s_addr    =   ip_header->ip_src.s_addr;
    /* Docelowy adres IP: */
    pseudo_header->ip_dst.s_addr    =   ip_header->ip_dst.s_addr;
    /* Pole wyzerowane: */
    pseudo_header->unused           =   0;
    /* Identyfikator enkapsulowanego protokolu: */
    pseudo_header->protocol         =   ip_header->ip_p;
    /* Rozmiar naglowka UDP i danych: */
    pseudo_header->length           =   udp_header->uh_ulen;
    /* Obliczenie sumy kontrolnej na podstawie naglowka UDP i pseudo-naglowka: */
    udp_header->uh_sum              =   0;
    checksum                        =   internet_checksum((unsigned short *)udp_header, sizeof(struct udphdr) + sizeof(struct phdr));
    udp_header->uh_sum              = (checksum == 0) ? 0xffff : checksum;

    fprintf(stdout, "Sending UDP...\n");

    /* Wysylanie datagramow co 1 sekunde: */
    for (;;) {

        /* Prosze zauwazyc ze pseudo-naglowek nie jest wysylany (ale jest on umieszczony w buforze za naglowkiem UDP dla wygodnego obliczania sumy kontrolnej): */
        retval = sendto(sockfd, (const char*)datagram, sizeof(struct ip) + sizeof(struct udphdr), 0, rp->ai_addr, rp->ai_addrlen);
        if (retval == SOCKET_ERROR) {
            fprintf(stderr, "sentdo() failed: %d\n", WSAGetLastError());
        }
        Sleep(1000);
    }

}
