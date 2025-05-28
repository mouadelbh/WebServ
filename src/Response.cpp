/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 16:28:05 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/28 13:35:07 by mel-bouh         ###   ########.fr       */
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
	if (status_code == 400 || status_code == 401 || status_code == 403 ||
		status_code == 405 || status_code == 413 || status_code == 414 ||
		status_code == 501 || status_code == 505) {
		return;
	}
	if (!request.pathIsValid(0)) setStatus(404, "Not Found");
	std::cout << "Path: " << request.path << " status: " << status_code << std::endl;
}

void	Response::buildBody(Request &request) {
	switch (status_code) {
		case 200:
			body = readFile(request.path);
			break ;
		case 404:
			body = readFile("status_errors/404.html");
			break ;
		case 405:
			body = readFile("status_errors/405.html");
			break ;
		case 505:
			body = readFile("status_errors/505.html");
	}
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
