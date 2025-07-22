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

Response::Response() : version("HTTP/1.1") ,status_code(200), server_config(nullptr) {}

Response::~Response() {
	clear();
}

void Response::setServerConfig(const ServerConfig* config) {
	server_config = config;
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
	const LocationConfig* location = request.findLocation(request.path);
	
	// Use location-specific root if available, otherwise server root
	std::string rootPath = server_config ? server_config->root : "/www";
	if (location && !location->root.empty()) {
		rootPath = location->root;
	}
	
	// Remove leading slash to make paths relative to current directory
	if (!rootPath.empty() && rootPath[0] == '/') {
		rootPath = rootPath.substr(1);
	}
	
	// Use location-specific index if available, otherwise server index
	std::string indexFile = server_config ? server_config->index : "index.html";
	if (location && !location->index.empty()) {
		indexFile = location->index;
	}
	
	// Check autoindex setting from location or server
	bool shouldAutoindex = autoIndex; // Global setting
	if (server_config) {
		shouldAutoindex = server_config->autoindex;
	}
	if (location) {
		shouldAutoindex = location->autoindex;
	}

	if (isDirectory(request.path) && !endsWith(request.path, "/"))
		path += "/";
	if (isDirectory(request.path) && fileReadable(request.path + indexFile))
		path += indexFile;
	if (isDirectory(request.path) && !fileReadable(request.path + indexFile) && !shouldAutoindex) {
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
		body = readFile(getErrorPage(status_code));
}

void	Response::buildGetBody(Request &request) {
	if (request.status == 0)
			this->setGetPath(request);
	
	// Determine autoindex setting
	bool shouldAutoindex = autoIndex; // Global default
	if (server_config) {
		shouldAutoindex = server_config->autoindex;
	}
	const LocationConfig* location = request.findLocation(request.path);
	if (location) {
		shouldAutoindex = location->autoindex;
	}
	
	if (shouldAutoindex && isDirectory(request.path))
		body = generateAutoindexPage(request.path, uri);
	else
		this->createBody(request);
}

void	Response::buildHeaders(Request &request) {
	(void)request; // Suppress unused parameter warning
	if (status_code != 204) {
		headers["Content-Length"] = std::to_string(body.size());
		if (status_code == 200) {
			// Use the original URI for content type detection, not the modified path
			headers["Content-Type"] = getContentType(uri);
		} else {
			headers["Content-Type"] = "text/html";
		}
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
	
	// Store original path for method checking
	std::string originalPath = request.path;
	
	// Use server root from config, remove leading slash if present
	std::string rootPath = server_config ? server_config->root : "/www";
	if (!rootPath.empty() && rootPath[0] == '/') {
		rootPath = rootPath.substr(1); // Remove leading slash to make it relative
	}
	request.path = rootPath + request.path;
	
	this->buildStatusLine(request);
	
	// Check if method is allowed AFTER building status line but BEFORE processing
	if (request.status == 0 && !request.isMethodAllowed(originalPath)) {
		setStatus(405, "Method Not Allowed");
		request.status = 405;
	}
	
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

std::string Response::getErrorPage(int code) {
	if (!server_config) return "www/status_errors/" + std::to_string(code) + ".html";
	
	// Check if error page is defined in server config
	auto it = server_config->error_pages.find(code);
	if (it != server_config->error_pages.end()) {
		std::string errorPagePath = it->second;
		// Remove leading slash to make it relative
		if (!errorPagePath.empty() && errorPagePath[0] == '/') {
			errorPagePath = errorPagePath.substr(1);
		}
		return "www/" + errorPagePath;
	}
	
	// Default error page location
	return "www/status_errors/" + std::to_string(code) + ".html";
}

std::string Response::getContentType(const std::string& path) {
	if (endsWith(path, ".html")) return "text/html";
	if (endsWith(path, ".css")) return "text/css";
	if (endsWith(path, ".png")) return "image/png";
	if (endsWith(path, ".jpg") || endsWith(path, ".jpeg")) return "image/jpeg";
	if (endsWith(path, ".gif")) return "image/gif";
	if (endsWith(path, ".js")) return "application/javascript";
	if (endsWith(path, ".json")) return "application/json";
	if (endsWith(path, ".xml")) return "application/xml";
	if (endsWith(path, ".pdf")) return "application/pdf";
	if (endsWith(path, ".txt")) return "text/plain";
	
	// Check if it's a directory (for autoindex)
	if (isDirectory(path)) return "text/html";
	
	return "application/octet-stream";
}
