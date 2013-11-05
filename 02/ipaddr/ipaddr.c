/*
 * Data:                2009-02-18
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Uruchamianie:        > ipaddr.exe <adres IPv4 lub IPv6>
 */

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

char* ipv4_binary_string(unsigned int addr);

int main(int argc, char** argv) {

    /* Struktura, ktora otrzyma informacje na temat implementacji 'Windows Sockets': */
    WSADATA             wsaData;

    int                 i;
    int                 retval; /* Wartosc zwracana przez funkcje. */
    unsigned long       addr;   /* Zmienna dla adresu IPv4 zwracanego przez funkcje inet_addr(). */

    /*
     * Adres IPv4 lub IPv6 w postaci tekstowej. Rozmiar nie powinien przekroczyc
     * INET6_ADDRSTRLEN. Pomimo tego faktu kozystamy z NI_MAXHOST (okresla maksymalny
     * rozmiar bufora dla nazwy domenowej lub adresu IP w formie tekstowej):
     */
    char                address[NI_MAXHOST];

    /* Struktura zawierajaca wskazowki dla funkcji getaddrinfo(): */
    struct addrinfo     hints;

    /*
     * Wskaznik na liste zwracana przez getaddrinfo() oraz wskaznik uzywany do
     * poruszania sie po elementach listy:
     */
    struct addrinfo     *rp, *result;

    struct sockaddr_in6 *sockaddr_ptr; /* Wskaznik na strukture adresowa dla IPv6. */

    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <IPv4 OR IPv6 ADDRESS>\n", argv[0]);
        system("pause");
        exit(EXIT_FAILURE);
    }

    /* Inicjalizacja Windows Sockets: */
    retval = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (retval != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", retval);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    /* Pozwalamy na AF_INET or AF_INET6: */
    hints.ai_family         =   AF_UNSPEC;  /* AF_UNSPEC == 0 */
    /* Gniazdo typu SOCK_STREAM (TCP): */
    hints.ai_socktype       =   SOCK_STREAM;
    hints.ai_protocol       =   IPPROTO_TCP;

    /*
     * AI_NUMERICHOST - nazwy domenowe nie beda rozwiazywane. Pierwszym argumentem
     * funkcji getaddrinfo() nie moze byc nazwa domenowa.
     */
    hints.ai_flags          =   0; //AI_NUMERICHOST;

    retval = getaddrinfo(argv[1], NULL, &hints, &result);
    if (retval != 0) {
        fprintf(stderr, "getaddrinfo() failed: %d\n", retval);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    /* Przechodzimy kolejno przez elementy listy: */
    for (rp = result; rp != NULL; rp = rp->ai_next) {

        if (rp->ai_family == AF_INET) {

            /*************************************************************/
            /*  Konwersja adresu IPv4 z postaci czytelnej dla czlowieka  */
            /*               getaddrinfo(), inet_addr()                  */
            /*************************************************************/
            fprintf(stdout, "Internet host address to network address structure conversion (AF_INET):\n\n");

            /* Wypisanie adresu IPv4 w formie binarnej. Adres zostal uzyskany za pomoca funkcji getaddrinfo(): */
            fprintf(
                stdout, "getaddrinfo(): %s (binary)\n",
                ipv4_binary_string(((struct sockaddr_in*)(rp->ai_addr))->sin_addr.s_addr)
            );

            /* Konwersje adresu IPv4 mozna przeprowadzic rowniez za pomoca inet_addr().
             * Funkcja inet_addr() przeprowadza konwersje adresu IPv4 z postaci
             * kropkowo-dziesietnej (parametr funkcji) do postaci zrozumialej dla maszyny
             * (wartosc zwracana).
             */
            addr = inet_addr(argv[1]);
            if (addr == INADDR_NONE) {
                fprintf(stderr, "\ninet_addr(): String does not contain a legitimate Internet address!\n");
            } else {
                fprintf(stdout, "inet_addr():   %s (binary)\n", ipv4_binary_string(addr));
            }

            /*************************************************************/
            /* Konwersja adresu IPv4 do postaci czytelnej dla czlowieka  */
            /*               inet_ntoa(), getnameinfo()                  */
            /*************************************************************/
            fprintf(stdout, "\nNetwork address structure to internet host address conversion (AF_INET):\n\n");

            /*
             * Funkcja inet_ntoa() przeprowadza konwersje adresu IPv4 z postaci
             * zrozumialej dla maszyny do postaci tekstowej (wartosc zwracana).
             * String jest zwracany w statycznie zaalokowanym buforze. Kolejne wywolanie
             * funkcji spowoduje jego nadpisanie.
             */
            fprintf(stdout, "inet_ntoa():   %s\n", inet_ntoa(((struct sockaddr_in*)rp->ai_addr)->sin_addr));

            /* Podobna konwersja za pomoca funkcji getnameinfo(): */
            if ((retval = getnameinfo(rp->ai_addr, rp->ai_addrlen, address, NI_MAXHOST, NULL, 0, NI_NUMERICHOST)) != 0) {
                fprintf(stderr, "getnameinfo() failed: %d\n", WSAGetLastError());
            } else {
                fprintf(stdout, "getnameinfo(): %s\n", address);
            }

        } else if (rp->ai_family == AF_INET6) {

            /*************************************************************/
            /*  Konwersja adresu IPv6 z postaci czytelnej dla czlowieka  */
            /*                       getaddrinfo()                       */
            /*************************************************************/
            fprintf(stdout, "Internet host address to network address structure conversion (AF_INET6):\n\n");

            /* Wypisanie adresu IPv6 w formie heksadecymalnej.
             * Adres zostal uzyskany za pomoca funkcji getaddrinfo(): */
            fprintf(stdout, "getaddrinfo(): ");
            i = 0;
            sockaddr_ptr = (struct sockaddr_in6*)rp->ai_addr;
            for (;;) {
                fprintf(stdout, "%.2x", sockaddr_ptr->sin6_addr.s6_addr[i]);
                ++i;
                if (i == 16) {
                    break;
                }

                if (i%2 == 0) {
                    fprintf(stdout, ":");
                }

            }
            fprintf(stdout, " (hex)\n");

            /*************************************************************/
            /* Konwersja adresu IPv6 do postaci czytelnej dla czlowieka  */
            /*                       getnameinfo()                       */
            /*************************************************************/
            fprintf(stdout, "\nNetwork address structure to internet host address conversion (AF_INET6):\n\n");

            if ((retval = getnameinfo(rp->ai_addr, rp->ai_addrlen, address, NI_MAXHOST, NULL, 0, NI_NUMERICHOST)) != 0) {
                fprintf(stderr, "getnameinfo() failed: %d\n", WSAGetLastError());
            } else {
                fprintf(stdout, "getnameinfo(): %s\n", address);
            }

        }

    }

    if (result != NULL) freeaddrinfo(result);

    if (WSACleanup() != 0) {
        fprintf(stderr, "WSACleanup failed.\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}


/*
 * Funkja odpowiedzialna za konwersje adresu IPv4 z postaci zrozumialej dla
 * maszyny do stringu reprezentujacego adres w formie binarnej.
 */
char* ipv4_binary_string(unsigned int addr) {
    int i, j;
    static char buff[36];
    const unsigned int pos = 0x80000000;
    addr = ntohl(addr);

    for (i=1, j=0; i < 32; ++i, ++j) {
        buff[j] = (pos & addr) ? '1' : '0';
        addr <<= 1;
        if (i%8 == 0) {
            buff[++j] = '.';
        }
    }

    buff[j++] = (pos & addr) ? '1' : '0';
    buff[j] = '\0';
    return buff;
}
