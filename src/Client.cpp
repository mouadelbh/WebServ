/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 10:08:13 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/06/20 05:08:30 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"

Client::Client() : fd(-1), addr(), addr_len(0), state(READING), server_config(nullptr) 
{
	response_raw = "HTTP/1.1 200 OK\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length: 13\r\n"
				"\r\n"
				"Hello World!\n";
}

Client::Client(int fd, sockaddr_in addr, socklen_t addr_len)
	: fd(fd), addr(addr), addr_len(addr_len), server_config(nullptr) 
{
	this->state = READING;
	this->response_raw = "HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: 13\r\n"
			"\r\n"
			"Hello World!\n";
}

Client::~Client() {}

void Client::setServerConfig(const ServerConfig* config) 
{
	server_config = config;
	request.setServerConfig(config);
	response.setServerConfig(config);
}

bool	Client::getRequest(std::vector<struct pollfd> &fds, size_t *index) {
	char buffer[1024];
	int bytes_received = 1;

	bytes_received = recv(fd, buffer, sizeof(buffer), 0);
	if (bytes_received <= 0) {
		if (bytes_received < 0)
			std::cerr << "Error receiving data\n";
		return false;   // signal to kickClient(...)
	}

	request_raw.append(buffer, bytes_received);

	std::cout << "outside the parser\n";
	if (request.parse(request_raw) || request.status != 0) {
		std::cout << "inside the parser\n";
		state = WRITING;
		fds[*index].events = POLLOUT | POLLERR | POLLHUP | POLLNVAL; // Change to POLLOUT for writing response
	}
	return true;
}

void	Client::buildResponse() {
	response.build(request);
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
	this->clear();
	return true;
}

void	Client::clear() {
	response_raw.clear();
	request_raw.clear();
	request.clear();
	response.clear();
}
