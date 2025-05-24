/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 10:08:13 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/24 17:24:01 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/Client.hpp"

Client::Client() : fd(-1), addr(), addr_len(0), state(READING) {
	response_raw = "HTTP/1.1 200 OK\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length: 17\r\n"
				"\r\n"
				"Niggas in Paris!\n";
}

Client::Client(int fd, sockaddr_in addr, socklen_t addr_len)
	: fd(fd), addr(addr), addr_len(addr_len) {
	this->state = READING;
	this->response_raw = "HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: 17\r\n"
			"\r\n"
			"Niggas in Paris!\n";
}

Client::~Client() {}

bool	Client::getRequest(std::vector<struct pollfd> &fds, size_t *index) {
	char buffer[1024];
	int bytes_received = 1;

	memset(buffer, 0, sizeof(buffer));
	while (bytes_received > 0) {
		bytes_received = recv(fd, buffer, sizeof(buffer) - 1, 0);
		if (bytes_received < 0) {
			std::cerr << "Error receiving data" << std::endl;
			return false;
		}
		request_raw.append(buffer, bytes_received);
		if (request_raw.find("\r\n\r\n") != std::string::npos) {
			break; // End of HTTP request_raw
		}
		memset(buffer, 0, sizeof(buffer));
	}
	std::cout << "-------Getting Request from client " << fd << "------" << std::endl;
	std::cout << request_raw << std::endl;
	state = WRITING;
	fds[*index].events = POLLOUT | POLLERR | POLLHUP | POLLNVAL; // Change to POLLOUT for writing response
	request.parse(request_raw);
	return true;
}

void	Client::buildStatusLine() {
	if (request.method != "GET") response.setStatus(405, "Method Not Allowed");
	if (request.version != "HTTP/1.1" && request.version != "HTTP/1.0") response.setStatus(505, "HTTP Version Not Supported");
	if (request.path != "/") response.setStatus(404, "Not Found");
	response.version = "HTTP/1.1";
	std::cout << "Path: " << request.path << " status: " << response.status_code << std::endl;
}

void	Client::buildBody() {
	switch (response.status_code) {
		case 200:
			response.body = readFile("www/index.html");
			break ;
		case 404:
			response.body = readFile("status_errors/404.html");
			break ;
		case 405:
			response.body = readFile("status_errors/405.html");
			break ;
		case 505:
			response.body = readFile("status_errors/505.html");
	}
}

void	Client::buildHeaders() {
	response.headers["Content-Length"] = std::to_string(response.body.size());
	response.headers["Content-Type"] = "text/html";

	response.headers["Connection"] = "keep-alive";
	response.headers["Server"] = "Webserv/1.0";
}

void	Client::buildResponse() {
	this->buildStatusLine();
	this->buildBody();
	this->buildHeaders();
	response_raw = response.toString();
}

bool	Client::sendResponse(std::vector<struct pollfd> &fds, size_t *index) {
	this->buildResponse();
	int bytes_sent = send(fd, response_raw.c_str(), response_raw.size(), 0);
	if (bytes_sent < 0) {
		std::cerr << "Error sending response_raw" << std::endl;
		return false;
	}
	std::cout << "------response_raw sent to client " << fd << "------" << std::endl;
	std::cout << response_raw << std::endl;
	state = READING; // Change state back to READING
	fds[*index].events = POLLIN | POLLHUP | POLLERR | POLLNVAL; // Change back to POLLIN for reading next request
	return true;
}
