#include <cstdio>
#include <cstdlib>
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <tchar.h>
#include "HttpRequestParser.h"
#include "HttpResponse.h"

#pragma comment(lib, "ws2_32.lib")

std::string int2string(long n) {
	std::ostringstream out;
	out << n;
	return out.str();
}

/* Zwraca rozszerzenie pliku */
std::string getFileExt(const std::string& filename) {
	int pos = filename.find('.');
	if(pos > -1) {
		return filename.substr(pos + 1);
	}

	return "";
}

/* Sprawdza czy plik jest obrazkiem */
bool isImage(const std::string& filename) {
	const char* images[] = {
		"png", "jpg", "jpeg", "gif"
	};

	std::string ext = getFileExt(filename);
	for(int i = 0; i < 4; i++) {
		if(ext == images[i]) return true;
	}

	return false;
}

/* Generuje odpowiedŸ o kodzie 404: File not found */
HttpResponse getFileNotFoundResponse() {
	HttpResponse response;
	response.setCode(404);
	response.setStatus("File not found");
	response.addHeader("Content-Type", "text/html");
	response.addHeader("Content-Length", "14");
	response.setContent("File not found");

	return response;
}

/* Zwrca odpowiedŸ w postaci pliku graficznego */
HttpResponse getImageResponse(const HttpRequest& request, TCHAR *img) {	
	HttpResponse response;
	
	HANDLE file = CreateFile((LPCSTR) img, GENERIC_READ, FILE_SHARE_READ, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if(file == INVALID_HANDLE_VALUE) {
		// Plik nie istnieje: 404
		return getFileNotFoundResponse();
	}	

	// Pobieramy rozmiar pliku
	unsigned int size = GetFileSize(file, NULL);

	// Alokujemy bufor na zawartoœæ pliku
	char *buffer = (char*) malloc(size + 1);

	int readed = 0;
	if(!ReadFile(file, buffer, size, (LPDWORD) &readed, NULL)) {
		response = getFileNotFoundResponse();
	}	
	else {		
		response.setCode(200);
		response.setStatus("OK");
		response.addHeader("Content-Type", "image/" + getFileExt(img));
		response.addHeader("Content-Length", int2string(size));
		response.setContent(std::string(buffer, size));	
	}

	// Zwalniamy bufor z zawartoœci¹ pliku
	free(buffer);
	// Zamykamy plik
	CloseHandle(file);

	return response;
}

HttpResponse getIndexResponse(HttpRequest request) {
	WIN32_FIND_DATA findData;

	std::ostringstream out;
	out << "<html><body><center>\n";

	HANDLE find = FindFirstFile(TEXT("img\\*"), &findData);
	while(find != INVALID_HANDLE_VALUE) {
		if(isImage(findData.cFileName)) {
			out << "<img src=\"img/" << findData.cFileName << "\" /><br />";
		}		

		if (!FindNextFile(find, &findData)) {
			FindClose(find);
			find = INVALID_HANDLE_VALUE;
		}		
	}
	
	out << "</center></body></html>";
	
	std::string content = out.str();

	HttpResponse response;
	response.setCode(200);
	response.setStatus("OK");
	response.addHeader("Content-Type", "text/html");
	response.addHeader("Content-Length", int2string(content.length())); 
	response.setContent(content);

	return response;

}

DWORD WINAPI handleRequest(LPVOID fd) {
	SOCKET client = (SOCKET) fd;	

	// Bufor na treœæ ¿¹dania 
	char requeststr[8096] = {0};

	int retval;
	if((retval = recv(client, requeststr, sizeof(requeststr), 0)) == SOCKET_ERROR) {
		// Nie uda³o siê odebranie wiadomoœci
        fprintf(stderr, "recv() failed: %d\n", WSAGetLastError());
		closesocket(client);
		return 0;		
	}

	// Parsujemy ¿¹danie HTTP
	HttpRequest request = HttpRequestParser(requeststr).parse();	

	HttpResponse response;
	if(isImage(request.getURI())) {
		// Œcie¿ka do obrazka
		std::string img = "." + request.getURI();
		// Zwrcamy plik
		response = getImageResponse(request, (TCHAR*) img.c_str());
	}
	else {
		// Zwracamy indeks plików
		response = getIndexResponse(request);
	}

	// Generujemy i wysy³amy odpowiedŸ
	std::string responsestr = response.getResponse(request);
	if((retval = send(client, responsestr.c_str(), responsestr.length(), 0)) == SOCKET_ERROR) {
		// Nieuda³o siê dostarczenie wiadomoœci
        fprintf(stderr, "send() failed: %d\n", WSAGetLastError());
		closesocket(client);
		return 0;
	}

    if (closesocket(client) == SOCKET_ERROR) {
        fprintf(stderr, "closesocket() failed: %d\n", WSAGetLastError());
    }	

	return 0;
}

int main(int argc, char* argv[]) {
	int retval = 0;

	if(argc < 2) {
		fprintf(stderr, "Uzycie: %s <port> <div>", argv[0]);
		system("pause");
		return EXIT_FAILURE;
	}

	printf("%s\n", argv[0]);

	WSADATA wsaData;
    /* Inicjalizacja 'Windows Sockets' DLL: */
    retval = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (retval != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", retval);
        exit(EXIT_FAILURE);
    }

    /* Utworzenie gniazda dla protokolu TCP: */
    SOCKET listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if (listenfd == INVALID_SOCKET) {
        fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    }

	struct sockaddr_in server_addr;
    /* Wyzerowanie struktury adresowej serwera: */
    memset(&server_addr, 0, sizeof(server_addr));
    /* Domena komunikacyjna (rodzina protokolow): */
    server_addr.sin_family      =   AF_INET;
    /* Adres nieokreslony (ang. wildcard address): */
    server_addr.sin_addr.s_addr =   htonl(INADDR_ANY);
    /* Numer portu: */
    server_addr.sin_port        =   htons(atoi(argv[1]));
    /* Rozmiar struktury adresowej serwera w bajtach: */
    int server_addr_len         =   sizeof(server_addr);

    /* Powiazanie lokalnego adresu IP i numeru portu z gniazdem: */
    if (bind(listenfd, (struct sockaddr*) &server_addr, server_addr_len) == SOCKET_ERROR) {
        fprintf(stderr, "bind() failed: %d\n", WSAGetLastError());
        closesocket(listenfd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    /* Przeksztalcenie gniazda w gniazdo nasluchujace: */
    if (listen(listenfd, SOMAXCONN) == SOCKET_ERROR) {
        fprintf(stderr, "listen() failed: %d\n", WSAGetLastError());
        closesocket(listenfd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

	fprintf(stdout, "Server is listening for incoming connection...\n");

	while(true) {
		/* Próba nawi¹zania nowego po³¹czenia z serwerem */
		struct sockaddr_in client_addr;
		/* Rozmiar struktury adresowej dla adresu klienta. */
		int client_addr_len = sizeof(client_addr); 

		/* Funkcja pobiera polaczenie z kolejki polaczen oczekujacych na zaakceptowanie
			* i zwraca deskryptor dla gniazda polaczonego: */
		SOCKET connfd = accept(listenfd, (struct sockaddr*) &client_addr, &client_addr_len);
		if (connfd == INVALID_SOCKET) {
			fprintf(stderr, "accept() failed: %d\n", WSAGetLastError());
			closesocket(listenfd);
			WSACleanup();
			exit(EXIT_FAILURE);
		}

		printf("TCP connection accepted from %s:%d\n", 
			inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));		

		/* Utworzenie nowego w¹tku i obs³uga ¿¹dania */
		CreateThread(NULL, 0, &handleRequest, (LPVOID) connfd, 0, NULL);
	}

	closesocket(listenfd);
    if (WSACleanup() == SOCKET_ERROR) {
        fprintf(stderr, "WSACleanup failed: %d\n", WSAGetLastError());
    }

	system("PAUSE");
	return 0;
}