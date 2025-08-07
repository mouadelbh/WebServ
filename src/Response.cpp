/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 16:28:05 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/08/07 18:52:06 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"

Response::Response() : version("HTTP/1.1") ,status_code(200) ,request(NULL) {}

Response::~Response() {
	clear();
}

void Response::debugResponse() const {
    std::string response_str = toString();
    std::cout << "=== RESPONSE DEBUG ===" << std::endl;
    std::cout << "Response length: " << response_str.length() << " bytes" << std::endl;

    // Find the Content-Length header
    size_t header_end = response_str.find("\r\n\r\n");
    if (header_end != std::string::npos) {
        std::string headers_only = response_str.substr(0, header_end);
        std::string body_only = response_str.substr(header_end + 4);

        std::cout << "Headers section length: " << headers_only.length() << std::endl;
        std::cout << "Body section length: " << body_only.length() << std::endl;

        // Extract Content-Length value
        size_t cl_pos = headers_only.find("Content-Length:");
        if (cl_pos != std::string::npos) {
            size_t cl_start = headers_only.find(":", cl_pos) + 1;
            size_t cl_end = headers_only.find("\r\n", cl_pos);
            std::string cl_value = headers_only.substr(cl_start, cl_end - cl_start);

            // Trim whitespace
            size_t first = cl_value.find_first_not_of(" \t");
            if (first != std::string::npos) {
                cl_value = cl_value.substr(first);
            }

            int declared_length = std::stoi(cl_value);
            std::cout << "Declared Content-Length: " << declared_length << std::endl;
            std::cout << "Actual body length: " << body_only.length() << std::endl;

            if (declared_length != (int)body_only.length()) {
                std::cout << "❌ CONTENT-LENGTH MISMATCH!" << std::endl;
                std::cout << "Difference: " << (declared_length - (int)body_only.length()) << " bytes" << std::endl;
            } else {
                std::cout << "✅ Content-Length matches body size" << std::endl;
            }
        }
    }

    // Show the actual response (first 200 chars)
    std::cout << "Response preview:" << std::endl;
    std::cout << response_str.substr(0, 200) << "..." << std::endl;
    std::cout << "=== END DEBUG ===" << std::endl;
}

void Response::setStatus(int code, const std::string &message) {
	status_code = code;
	status_message = message;
}

std::string Response::toString() const
{
    std::ostringstream oss;

    // Status line with CRLF
    oss << version << " " << status_code << " " << status_message << "\r\n";

    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        oss << it->first << ": " << it->second << "\r\n";
    }

    oss << "\r\n";
    oss << body;

    return oss.str();
}

// std::string Response::toString() const {
// 	std::ostringstream oss;
// 	oss << version << " " << status_code << " " << status_message << "\r\n";
// 	for (const auto &header : headers) {
// 		oss << header.first << ": " << header.second << "\r\n";
// 	}
// 	oss << "\r\n" << body;
// 	return oss.str();
// }

void	Response::buildStatusLine() {
	version = "HTTP/1.1";
	if (request->status != 0)
		setStatus(request->status, getStatusCodeMap(request->status));
	std::cout << "Path: " << request->path << " status: " << status_code << std::endl;
}

void	Response::setGetPath() {
	std::string &path = request->path;

	if (isDirectory(request->path) && !endsWith(request->path, "/"))
		path += "/";
	if (isDirectory(request->path) && fileReadable(request->path + "index.html"))
		path += "index.html";
	if (isDirectory(request->path) && !fileReadable(request->path + "index.html") && !autoIndex) {
		setStatus(403, "Forbidden");
		return;
	}
	if (!isDirectory(request->path) && !fileExists(request->path)) {
		if (errno == ENOENT || errno == ENOTDIR) {
			std::cout << "File not found: " << request->path << std::endl;
			setStatus(404, "Not Found");
		}
		else if (errno == EACCES)
			setStatus(403, "Forbidden");
		else
			setStatus(500, "Internal Server Error");
	}
}

void	Response::createBody() {
	if (status_code == 204) {
		body.clear();
		return;
	}
	else if (status_code == 201)
		body = readFile("www/upload.html");
	else if (status_code == 200)
		body = readFile(request->path);
	else
		body = readError("status_errors/" + std::to_string(status_code) + ".html", status_code);
}

void	Response::buildGetBody() {
	if (status_code == 200)
		this->setGetPath();
	if (status_code == 200 && autoIndex && isDirectory(request->path)) {
		body = generateAutoindexPage(request->path, uri);
	}
	else
		this->createBody();
}

void Response::buildHeaders() {
    // FIRST: Make sure body is finalized BEFORE calculating Content-Length
    if (status_code != 204) {
        if (status_code == 200)
            headers["Content-Type"] = request->getType();
        else
            headers["Content-Type"] = "text/html";
    }

    // Calculate Content-Length AFTER body is complete and finalized
    if (status_code != 204) {
        // CRITICAL: Use body.size(), not body.length()
        // In case of binary data, these might differ
        headers["Content-Length"] = std::to_string(body.size());
    }

    headers["Connection"] = "close";
    headers["Server"] = "Webserv";
}

// void	Response::buildHeaders() {
// 	if (status_code != 204) {
// 		headers["Content-Length"] = std::to_string(body.size());
// 		if (status_code == 200)
// 			headers["Content-Type"] = request->getType();
// 		else
// 			headers["Content-Type"] = "text/html";
// 	}

// 	headers["Connection"] = "close";
// 	headers["Server"] = "Webserv";
// }

void	Response::executeDeleteBody() {
	struct stat fileStat;

	if (stat(request->path.c_str(), &fileStat) == -1) {
		if (errno == ENOENT) {;
			setStatus(404, "Not Found");
		}
		else if (errno == EACCES)
			setStatus(403, "Forbidden");
		else
			setStatus(500, "Internal Server Error");
		return;
	}
	if (!S_ISREG(fileStat.st_mode)) {
		setStatus(403, "Forbidden");
		return;
	}
	if (std::remove(request->path.c_str()) != 0) {
		if (errno == EACCES)
			setStatus(403, "Forbidden");
		else
			setStatus(500, "Internal Server Error");
		return;
	}
	else {
		setStatus(204, "No Content");
		body.clear();
	}
}

void	Response::build() {
	clear();
	uri = request->path;
	request->path = "www" + request->path;
	this->buildStatusLine();

	if (request->method == "GET" || status_code != 200) {
		this->buildGetBody();
	}
	else if (request->method == "POST") {
		this->executePostBody();
		this->createBody();
	}
	else if (request->method == "DELETE") {
		this->executeDeleteBody();
		this->createBody();
	}
	else {
		this->setStatus(501, "Not Implemented");
		this->createBody();
	}
	this->buildHeaders();
}

void	Response::clear() {
	version.clear();
	status_code = 200;
	status_message.clear();
	headers.clear();
	body.clear();
}
