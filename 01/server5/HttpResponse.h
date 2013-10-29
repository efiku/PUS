#pragma once

#include <string>
#include <sstream>
#include "HttpRequest.h"

class HttpResponse
{
	int code;
	std::string status;	
	std::string content;
	HttpHeaders headers;

public:

	std::string getResponse(HttpRequest request) const {
		std::ostringstream out;

		out << request.getVersion() << " " << code << status << "\r\n";

		// Nag³ówki odpowiedzi
		HttpHeaders::const_iterator it = headers.begin();
		for(; it != headers.end(); it++) {
			out << (*it).first << ": " << (*it).second << "\r\n";
		}

		out << "\r\n";

		// Treœæ odpowiedzi
		out << content;

		return out.str();
	}

	void setCode(int code) {
		this->code = code;
	}

	void setStatus(const std::string& status) {
		this->status = status;
	}

	void setContent(const std::string& content) {
		this->content = content;
	}

	void addHeader(const std::string& name, const std::string& value) {
		headers[name] = value;
	}
};

