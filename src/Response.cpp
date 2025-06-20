/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 16:28:05 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/06/19 00:13:12 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"

Response::Response() : version("HTTP/1.1") ,status_code(200) {}

Response::~Response() {
	clear();
}

void Response::setStatus(int code, const std::string &message) {
	status_code = code;
	status_message = message;
}

std::string Response::toString() const {
	std::ostringstream oss;
	oss << version << " " << status_code << " " << status_message << "\r\n";
	for (const auto &header : headers) {
		oss << header.first << ": " << header.second << "\r\n";
	}
	oss << "\r\n" << body;
	return oss.str();
}

void	Response::buildStatusLine(Request &request) {
	version = "HTTP/1.1";
	if (request.status != 0)
		setStatus(request.status, getStatusCodeMap(request.status));
	std::cout << "Path: " << request.path << " status: " << status_code << std::endl;
}

void	Response::setGetPath(Request &request) {
	std::string &path = request.path;

	if (isDirectory(request.path) && !endsWith(request.path, "/"))
		path += "/";
	if (isDirectory(request.path) && fileReadable(request.path + "index.html"))
		path += "index.html";
	if (isDirectory(request.path) && !fileReadable(request.path + "index.html") && !autoIndex) {
		setStatus(403, "Forbidden");
		return;
	}
	if (!isDirectory(request.path) && !fileExists(request.path)) {
		if (errno == ENOENT || errno == ENOTDIR) {
			std::cout << "File not found: " << request.path << std::endl;
			setStatus(404, "Not Found");
		}
		else if (errno == EACCES)
			setStatus(403, "Forbidden");
		else
			setStatus(500, "Internal Server Error");
		return;
	}
}

void	Response::createBody(Request &request) {
	if (status_code == 200)
		body = readFile(request.path);
	else
		body = readFile("status_errors/" + std::to_string(status_code) + ".html");
}

void	Response::buildGetBody(Request &request) {
	if (request.status == 0)
			this->setGetPath(request);
	if (autoIndex && isDirectory(request.path))
		body = generateAutoindexPage(request.path, uri);
	else
		this->createBody(request);
}

void	Response::buildHeaders(Request &request) {
	if (status_code != 204) {
		headers["Content-Length"] = std::to_string(body.size());
		if (status_code == 200)
			headers["Content-Type"] = request.getType();
		else
			headers["Content-Type"] = "text/html";
	}

	headers["Connection"] = "keep-alive";
	headers["Server"] = "Webserv";
}

void	Response::buildDeleteBody(Request &request) {
	struct stat fileStat;

	if (stat(request.path.c_str(), &fileStat) == -1) {
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
	if (std::remove(request.path.c_str()) != 0) {
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

void	Response::build(Request &request) {
	clear();
	uri = request.path;
	request.path = "www" + request.path;
	this->buildStatusLine(request);
	if (request.method == "GET" || request.status != 0) {
		this->buildGetBody(request);
	}
	else if (request.method == "DELETE") {
		this->buildDeleteBody(request);
		this->createBody(request);
	}
	this->buildHeaders(request);
}

void	Response::clear() {
	version.clear();
	status_code = 200;
	status_message.clear();
	headers.clear();
	body.clear();
}
