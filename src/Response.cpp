/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 16:28:05 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/28 16:34:51 by mel-bouh         ###   ########.fr       */
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
	int	status_map[] = {400, 401, 403, 405, 413, 414, 501, 505};
	version = "HTTP/1.1";
	if (request.status != 0)
		setStatus(request.status, getStatusCodeMap(request.status));
	for (int i : status_map) {
		if (i == status_code)
			return;
	}
	// if (!request.pathIsValid(0)) setStatus(404, "Not Found");
	std::cout << "Path: " << request.path << " status: " << status_code << std::endl;
}

void	Response::setPath(Request &request) {
	std::string &path = request.path;

	path = "www" + path;
	if (isDirectory(request.path) && !endsWith(request.path, "/"))
		path += "/";
	if (isDirectory(request.path) && hasIndexHtml(request.path)) {
		path += "index.html";
	}
	if (isDirectory(request.path) && !hasIndexHtml(request.path) && !autoIndex) {
		request.status = 403;
		setStatus(403, "Forbidden");
		return;
	}
	if (!isDirectory(request.path) && !fileExists(request.path)) {
		request.status = 404;
		setStatus(404, "Not Found");
		return;
	}
}

void	Response::buildBody(Request &request) {
	uri = request.path;
	this->setPath(request);
	if (autoIndex && isDirectory(request.path))
		body = generateAutoindexPage(request.path, uri);
	else if (status_code == 200)
		body = readFile(request.path);
	else
		body = readFile("status_errors/" + std::to_string(status_code) + ".html");
}

void	Response::buildHeaders(Request &request) {
	headers["Content-Length"] = std::to_string(body.size());
	if (status_code == 200)
		headers["Content-Type"] = request.getType();
	else
		headers["Content-Type"] = "text/html";

	headers["Connection"] = "keep-alive";
	headers["Server"] = "Webserv";
}

void	Response::build(Request &request) {
	clear();
	this->buildStatusLine(request);
	this->buildBody(request);
	this->buildHeaders(request);
}

void	Response::clear() {
	version.clear();
	status_code = 200;
	status_message.clear();
	headers.clear();
	body.clear();
}
