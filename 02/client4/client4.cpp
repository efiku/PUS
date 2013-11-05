#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[]) {
    int retval = 0;

	WSADATA wsaData;	
    if ((retval = WSAStartup(MAKEWORD(2,2), &wsaData)) != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", retval);
        exit(EXIT_FAILURE);
    }	

	/* Struktura zawierajaca wskazowki dl funkcji getaddrinfo(). */
    struct addrinfo hints;                  
    /* Wskaznik na liste zwracana przez getaddrinfo() oraz wskaznik uzywany do poruszania sie po elementach listy. */
	struct addrinfo *rp, *result;           

	/* Wskazowki dla getaddrinfo(): */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family     =   AF_INET; /* Domena komunikacyjna (rodzina protokolow). */
	hints.ai_socktype   =   SOCK_STREAM; /* Typ gniazda. */
	hints.ai_protocol   =   IPPROTO_TCP; /* Protokol. */

    /* Pierwszy argument to adres IP lub nazwa domenowa: */
    retval = getaddrinfo(argv[1], argv[2], &hints, &result);
    if (retval != 0) {
        fprintf(stderr, "getaddrinfo() failed: %d\n", retval);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    SOCKET sockfd;
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockfd == INVALID_SOCKET) {
			fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
			continue;
		}

		if(connect(sockfd, rp->ai_addr, rp->ai_addrlen) == SOCKET_ERROR) {
			fprintf(stderr, "connect() failed: %d\n", WSAGetLastError());
			continue;
		}

		struct sockaddr_storage addr_storage;

		// Zerujemy strukture
		memset(&addr_storage, 0, sizeof(addr_storage));

		int len = sizeof(struct sockaddr_storage);		
		retval = getsockname(sockfd, (struct sockaddr*) &addr_storage, &len);
		if(retval != 0) {
			fprintf(stderr, "getnameinfo() failed: %d\n", WSAGetLastError());
		}

		char host[NI_MAXHOST] = {0}, serv[NI_MAXSERV] = {0};
		retval = getnameinfo((struct sockaddr*) &addr_storage, len, 
			host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST);

		if(retval != 0) {
			fprintf(stderr, "getnameinfo() failed: %d\n", WSAGetLastError());
		}

		// Wypisujemy informacje o gnieŸdzie
		printf("HOST: %s SERV: %s\n", host, serv);

		break;
	}

    /* Jezeli lista jest pusta (nie utworzono gniazda): */
    if (rp == NULL) {
        fprintf(stderr, "Could not create socket.\n");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

	char message[1024];
	if((retval = recv(sockfd, message, sizeof(message), 0)) == SOCKET_ERROR) {
		// Nie uda³o siê odebranie wiadomoœci
        fprintf(stderr, "recv() failed: %d\n", WSAGetLastError());
		closesocket(sockfd);
		return 0;		
	}

	// Wypisujemy wiadomoœæ
	printf("%s\n", &message);

    if (closesocket(sockfd) == SOCKET_ERROR) {
        fprintf(stderr, "closesocket() failed: %d\n", WSAGetLastError());
    }

    if (WSACleanup() == SOCKET_ERROR) {
        fprintf(stderr, "WSACleanup failed: %d\n", WSAGetLastError());
    }

	system("pause");

	return EXIT_SUCCESS;
}