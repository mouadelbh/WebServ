/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 10:08:13 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/24 14:46:21 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client() : fd(-1), addr(), addr_len(0), state(READING) {
	response = "HTTP/1.1 200 OK\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length: 17\r\n"
				"\r\n"
				"Niggas in Paris!\n";
}

Client::Client(int fd, sockaddr_in addr, socklen_t addr_len)
	: fd(fd), addr(addr), addr_len(addr_len) {
	this->state = READING;
	this->response = "HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: 17\r\n"
			"\r\n"
			"Niggas in Paris!\n";
}

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
		request.append(buffer, bytes_received);
		if (request.find("\r\n\r\n") != std::string::npos) {
			break; // End of HTTP request
		}
		memset(buffer, 0, sizeof(buffer));
	}
	std::cout << "-------Getting Request from client " << fd << "------" << std::endl;
	std::cout << request << std::endl;
	state = WRITING;
	fds[*index].events = POLLOUT | POLLERR | POLLHUP | POLLNVAL; // Change to POLLOUT for writing response
	return true;
}

bool	Client::sendResponse(std::vector<struct pollfd> &fds, size_t *index) {
	int bytes_sent = send(fd, response.c_str(), response.size(), 0);
	if (bytes_sent < 0) {
		std::cerr << "Error sending response" << std::endl;
		return false;
	}
	std::cout << "------Response sent to client " << fd << "------" << std::endl;
	std::cout << response << std::endl;
	state = READING; // Change state back to READING
	fds[*index].events = POLLIN | POLLHUP | POLLERR | POLLNVAL; // Change back to POLLIN for reading next request
	return true;
}
