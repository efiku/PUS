#pragma once

#include <string>
#include <sstream>
#include <map>

typedef std::map<std::string, std::string> HttpHeaders;

/**
 * Nag³owek HTTP
 * 
 * @author Adam Wójs <adam@wojs.pl:
 */
class HttpRequest {
	std::string method;
	std::string uri;
	std::string version;

	HttpHeaders headers;
	std::string body;

public:	
	const std::string& getMethod() const {
		return method;
	}

	void setMethod(const std::string& method) {
		this->method = method;
	}

	const std::string& getURI() const {
		return uri;
	}

	void setURI(const std::string& uri) {
		this->uri = uri;
	}

	const std::string& getVersion() const {
		return version;
	}

	void setVersion(const std::string& version) {
		this->version = version;
	}

	const HttpHeaders& getHeaders() const {
		return headers;
	}

	void setHeaders(const HttpHeaders& headers) {
		this->headers = headers;
	}

	const std::string& getBody() const {
		return body;
	}

	void setBody(const std::string& body) {
		this->body = body;
	}
};