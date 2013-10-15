#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <lib.h>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4701)

#define SOURCE_ADDRESS "fd8e:55e0:a9d1::/48"
#define SOURCE_PORT 5050

int main(int argc, char** argv)
{
	WSADATA             wsaData;            /* Struktura, ktora otrzyma informacje na temat implementacji 'Windows Sockets'. */
    SOCKET              sockfd;             /* Deskryptor gniazda. */
    struct addrinfo     hints;              /* Struktura zawierajaca wskazowki dl funkcji getaddrinfo(). */
    struct addrinfo     *rp, *result;       /* Wskaznik na liste zwracana przez getaddrinfo() oraz wskaznik uzywany do poruszania sie po elementach listy. */
    int                 retval;             /* Wartosc zwracana przez funkcje. */
    int                 socket_option;      /* Zmienna wykorzystywana do ustawiania opcji gniazda. */
    unsigned short      checksum;           /* Zmienna wykorzystywana do obliczenia sumy kontrolnej. */

    /* Bufor na naglowek IP, naglowek UDP oraz pseudo-naglowek: */
    unsigned char       datagram[sizeof(struct ipv6) + sizeof(struct udphdr) + sizeof(struct phdrv6)];

    /* Wskaznik na naglowek IP (w buforze okreslonym przez 'datagram'): */
    struct ipv6           *ip_header          = (struct ipv6 *)datagram;

    /* Wskaznik na naglowek UDP (w buforze okreslonym przez 'datagram'): */
    struct udphdr       *udp_header         = (struct udphdr *)(datagram + sizeof(struct ip));

    /* Wskaznik na pseudo-naglowek (w buforze okreslonym przez 'datagram'): */
    struct phdrv6         *pseudo_header      = (struct phdrv6 *)(datagram + sizeof(struct ipv6) + sizeof(struct udphdr));

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
	hints.ai_family     =   AF_INET6;        /* Domena komunikacyjna (IPv6). */
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
	inet_pton(AF_INET6, SOURCE_ADDRESS, &ip_header->ip_src); //adres zrodlowy
	inet_pton(AF_INET6, argv[1], &ip_header->ip_dst); //adres docelowy
	ip_header->ip_v_tc_flow         =   6; /* Wersja protokolu (IPv6). */
	ip_header->ip_hop               =   128;
	ip_header->ip_pl               =   36;

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
    pseudo_header->ip_src    =   ip_header->ip_src;
    /* Docelowy adres IP: */
    pseudo_header->ip_dst    =   ip_header->ip_dst;
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
		printf("Send");
        Sleep(1000);
    }
}