#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <lib.h>
#include <Windows.h>

#pragma comment(lib, "ws2_32.lib")

#define IP_HEADER_SIZE 20

#define PACKAGE_SIZE 44
#define PACKAGE_DATA_SIZE (PACKAGE_SIZE - sizeof(struct icmphdr))

#define MAX_TTL 255

void print_ping(char* response) {
	struct ip* ip = (struct ip*) response; 
	struct icmphdr* icmp = (struct icmphdr*) (response + sizeof(struct ip));
				
	printf("HOST: %s\n", inet_ntoa(ip->ip_src));
	printf("TTL: %d\n", ip->ip_ttl);
	printf("IP HEADER SIZE: %d\n", ip->ip_len);
	printf("IP HEADER DEST: %s\n", inet_ntoa(ip->ip_dst));
	printf("ICMP HEADER TYPE: %d, CODE: %d, ID: %d, SEQUENCE: %d\n", 
		icmp->type,
		icmp->code,
		icmp->un.echo.id, 
		icmp->un.echo.sequence
	);
}

DWORD WINAPI recv_echo_replay(LPVOID hostname) {
	SOCKET sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd == INVALID_SOCKET) {
        fprintf(stderr, "Could not create replay socket.\n");
        WSACleanup();
        exit(EXIT_FAILURE);
	}	
	
	struct sockaddr_in from;
	from.sin_family = AF_INET;
	from.sin_addr.s_addr = INADDR_ANY;
	from.sin_port = 0;	

	int len = sizeof(struct sockaddr_in);

	if(bind(sockfd, (sockaddr*) &from, len) == SOCKET_ERROR) {
		fprintf(stderr, "bind() error: %d\n", WSAGetLastError());
	}

	char *response = (char*) malloc(PACKAGE_SIZE + IP_HEADER_SIZE);

	for(int i = 0; i < 4; i++) {		
		memset(response, 0, PACKAGE_SIZE + IP_HEADER_SIZE);

		int retval = recvfrom(sockfd, response, PACKAGE_SIZE + IP_HEADER_SIZE, 0, (sockaddr*) &from, &len);
		if(retval == SOCKET_ERROR) {
			fprintf(stderr, "recv() failed: %d\n", WSAGetLastError());			
			continue; 
		}		
		
		print_ping(response);		

		Sleep(1000);
	}

	free(response);
	
    if (closesocket(sockfd) == SOCKET_ERROR) {
        fprintf(stderr, "closesocket() failed: %d\n", WSAGetLastError());
    }

	return 0;
}

int main(int argc, char* argv[]) {
	srand((unsigned int) time(NULL));	

    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <IPv4 ADDRESS>\n", argv[0]);
        system("pause");
        exit(EXIT_FAILURE);
    }

	printf("CURRENT PID: %d\n", GetCurrentProcessId());

    /* Inicjalizacja Winsock DLL przez proces: */
    int retval = 0;
	
	WSADATA wsaData;	
    if ((retval = WSAStartup(MAKEWORD(2,2), &wsaData)) != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", retval);
        exit(EXIT_FAILURE);
    }	

	HANDLE replayThread = CreateThread(NULL, 0, &recv_echo_replay, argv[1], 0, NULL);

	struct addrinfo hints, *result, *rp;
	
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;

	if((retval = getaddrinfo(argv[1], NULL, &hints, &result)) != 0) {
		fprintf(stderr, "getaddrinfo() failed: %d\n", retval);
        WSACleanup();
        exit(EXIT_FAILURE);		
	}

	SOCKET sockfd;
	for(rp = result; rp != NULL; rp = rp->ai_next) {
		sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockfd == INVALID_SOCKET) {
			fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
			continue;
		}

		int ttl = MAX_TTL;
		if((retval = setsockopt(sockfd, IPPROTO_IP, IP_TTL, (LPSTR) &ttl, sizeof(ttl))) == SOCKET_ERROR) {
			fprintf(stderr, "setsockopt() failed: %d\n", WSAGetLastError());
			continue;
		}
		
		break;
	}

	if(rp == NULL) {
        fprintf(stderr, "Could not create socket.\n");
        WSACleanup();
        exit(EXIT_FAILURE);
	}	

	char *package = (char*) malloc(PACKAGE_SIZE);			
	for(int i = 0; i < 4; i++) {
		memset(package, 0, PACKAGE_SIZE);
		
		struct icmphdr* header = (struct icmphdr*) package;
		header->type = ICMP_ECHO;
		header->code = 0;		 			 
		header->un.echo.id = GetCurrentProcessId();
		header->un.echo.sequence = i + 1;
		
		char* data = package + sizeof(struct icmphdr);
		for(int i = 0; i < PACKAGE_DATA_SIZE; i++) {
			data[i] = "ABCDEFGHIJKLMNOUPRSTUWXYZ0123456789"[(rand() % 36)];
		}
		
		header->checksum = 0;
		header->checksum = internet_checksum((unsigned short*) package, PACKAGE_SIZE);

		retval = sendto(sockfd, package, PACKAGE_SIZE, 0, rp->ai_addr, rp->ai_addrlen);
		if(retval == SOCKET_ERROR) {
			fprintf(stderr, "sentdo() failed: %d\n", WSAGetLastError());
			continue;
		}

		Sleep(1000);
	}

	
	//recv_echo_replay(NULL);

	free(package);

	Sleep(10000);
	CloseHandle(replayThread);

	freeaddrinfo(result);

    if (closesocket(sockfd) == SOCKET_ERROR) {
        fprintf(stderr, "closesocket() failed: %d\n", WSAGetLastError());
    }

    if (WSACleanup() == SOCKET_ERROR) {
        fprintf(stderr, "WSACleanup failed: %d\n", WSAGetLastError());
    }

	system("pause");

	return EXIT_SUCCESS;
}