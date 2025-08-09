/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 16:28:05 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/08/09 19:48:54 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"

Response::Response() : version("HTTP/1.1") ,status_code(200) ,request(NULL)
{
	this->CGI_active = false;
}

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

            int declared_length = std::atoi(cl_value.c_str());
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

bool	Response::autoIndexStatus() {
	if (config && config->autoindex != -1) {
		return config->autoindex;
	}
	else {
		return autoIndex;
	}
}

void	Response::buildStatusLine() {
	version = "HTTP/1.0";
	if (request->status != 0)
		setStatus(request->status, getStatusCodeMap(request->status));
}

std::string	Response::getIndex() {
	if (locationConfig && !locationConfig->index.empty())
		return locationConfig->index;
	else if (config && !config->index.empty())
		return config->index;
	return "";
}

void	Response::setGetPath() {
	std::string &path = request->path;

	if (isDirectory(request->path) && !endsWith(request->path, "/"))
		path += "/";
	if (isDirectory(request->path) && fileReadable(request->path + getIndex()))
		path += getIndex();
	else if (isDirectory(request->path) && !fileReadable(request->path + getIndex()) && !autoIndexStatus()) {
		setStatus(403, getStatusCodeMap(403));
		return;
	}
	if (!isDirectory(request->path) && !fileExists(request->path)) {
		if (errno == ENOENT || errno == ENOTDIR) {
			std::cout << "File not found: " << request->path << std::endl;
			setStatus(404, getStatusCodeMap(404));
		}
		else if (errno == EACCES)
			setStatus(403, getStatusCodeMap(403));
		else
			setStatus(500, getStatusCodeMap(500));
	}
}

std::string	Response::getErrorPage(int code) {
	std::string error_page = "error_pages/";
	if (config->error_pages.find(code) != config->error_pages.end())
		return config->error_pages[code];
	error_page += to_string_c98(code);
	error_page += ".html";
	return error_page;
}

std::string removeString(const std::string &str, const std::string &to_remove) {
	size_t pos = str.find(to_remove);
	if (pos != std::string::npos) {
		return str.substr(0, pos) + str.substr(pos + to_remove.length());
	}
	return str;
}

void	Response::createBody() {
	if (status_code == 204) {
		body.clear();
		return;
	}
	else if (status_code == 201)
		body = readError("www/upload.html", 201);
	else if (status_code == 200)
		body = readFile(request->path);
	else if (status_code == 301) {
		headers["Location"] = removeString(request->path, config->root);
		body += "<h1>Moved Permanently</h1>";
	}
	else
		body = readError(getErrorPage(status_code), status_code);
}

void	Response::buildGetBody() {
	if (status_code == 200)
		this->setGetPath();
	if ((status_code == 200) && autoIndexStatus() && isDirectory(request->path)) {
		body = generateAutoindexPage(request->path, uri);
	}
	else if (!autoIndexStatus()) {
		setStatus(403, getStatusCodeMap(403));
		this->createBody();
	}
	else
		this->createBody();
}

void Response::buildHeaders() {
    // FIRST: Make sure body is finalized BEFORE calculating Content-Length
    if (status_code != 204) {
        if (status_code == 200 && !CGI_active)
            headers["Content-Type"] = request->getType();
        else
            headers["Content-Type"] = "text/html";
    }

    // Calculate Content-Length AFTER body is complete and finalized
    if (status_code != 204) {
        // CRITICAL: Use body.size(), not body.length()
        // In case of binary data, these might differ
        headers["Content-Length"] = to_string_c98(body.size());
    }

    headers["Connection"] = "close";
    headers["Server"] = config->server_name.empty() ? "Webserv" : config->server_name;
}

void	Response::executeDeleteBody() {
	struct stat fileStat;

	if (stat(request->path.c_str(), &fileStat) == -1) {
		if (errno == ENOENT) {
			setStatus(404, getStatusCodeMap(404));
		}
		else if (errno == EACCES)
			setStatus(403, getStatusCodeMap(403));
		else
			setStatus(500, getStatusCodeMap(500));
		return;
	}
	if (!S_ISREG(fileStat.st_mode)) {
		setStatus(403, getStatusCodeMap(403));
		return;
	}
	if (std::remove(request->path.c_str()) != 0) {
		if (errno == EACCES)
			setStatus(403, getStatusCodeMap(403));
		else
			setStatus(500, getStatusCodeMap(500));
		return;
	} else {
		setStatus(204, getStatusCodeMap(204));
		body.clear();
	}
}

bool	validStatusCode(int code) {
	if (code >= 400)
		return false;
	return true;
}

void	Response::build() {
	clear();
	uri = request->path;
	if (config->root[config->root.length() - 1] != '/' && request->path[0] != '/')
		config->root = config->root + '/';
	if (config->root[0] == '/')
		config->root = config->root.substr(1);
	request->path = config->root + request->path;
	if (request->path[0] == '/')
		request->path = request->path.substr(1);
	std::cout << GREEN << "Request path: " << request->path << RESET << std::endl;
	this->buildStatusLine();

	if (isCGIRequest(request->path) && (request->method == "GET" || request->method == "POST")) {
		this->executeCGI();
		if (status_code != 200)
			this->createBody();
	}
	else if (request->method == "GET" || !validStatusCode(status_code)) {
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
		this->setStatus(501, getStatusCodeMap(501));
		this->createBody();
	}
	this->buildHeaders();
}


bool Response::isCGIRequest(const std::string &path) {
	std::string file_path = path;
	size_t query_pos = file_path.find('?');
	if (query_pos != std::string::npos) {
		file_path = file_path.substr(0, query_pos);
	}

	// Check if the file has a CGI extension with c++ 98
	if (config->cgi_ext.empty())
		return false;
	if (file_path.find_last_of('.') == std::string::npos)
		return false;
	std::string extension = file_path.substr(file_path.find_last_of('.'));
	if (config->cgi_ext.find(extension) == config->cgi_ext.end())
		return false;
	return true;
}

void	Response::clear() {
	version.clear();
	status_code = 200;
	status_message.clear();
	headers.clear();
	body.clear();
}
