#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include "HttpRequest.h"

/**
 * Parser ¿¹dañ HTTP
 *
 * @author Adam Wójs <adam@wojs.pl>
 */
class HttpRequestParser
{	
	// Pozacyja parsera
	int pos;
	// Treœæ zapytania HTTP
	const std::string& request;

public:
	HttpRequestParser(const std::string& request) 
		: pos(0), request(request) 
	{
	}

	HttpRequest parse() {
		HttpRequest request;		

		// Parsujemy Request-line
		std::string method, uri, version;

		// Wczytujemy metode, uri i wersje protoko³u 
		std::stringstream(getNextLine()) >> method >> uri >> version;

		request.setMethod(method);
		request.setURI(uri);
		request.setVersion(version);

		// Parsujemy nag³owki
		HttpHeaders headers;

		std::string header = getNextLine();
		while(!header.empty()) {
			int pos = header.find(':');

			if(pos == -1) break;

			std::string name  = header.substr(0, pos);
			std::string value = header.substr(pos + 2); 
			headers[name] = value;
			
			header = getNextLine();
		}

		request.setHeaders(headers);

		// Treœæ zapytania
		request.setBody(this->request.substr(pos));

		return request;
	}

protected: 
	std::string getNextLine() {
		std::string line;
		for(; pos < request.length() - 1; pos++) {
			if(request[pos] == '\r' && request[pos + 1] == '\n') {
				pos += 1;
				break;
			}
			else {
				line += request[pos];
			}
		}

		return line;
 	}
};

