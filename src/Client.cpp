/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 10:08:13 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/07/24 15:21:59 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"

Client::Client() : fd(-1), addr(), addr_len(0), state(READING) {
	response_raw = "HTTP/1.1 200 OK\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length: 17\r\n"
				"\r\n"
				"Niggas in Paris!\n";
	last_activity = NULL;
}

Client::Client(int fd, sockaddr_in addr, socklen_t addr_len)
	: fd(fd), addr(addr), addr_len(addr_len) {
	this->state = READING;
	this->response_raw = "HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: 17\r\n"
			"\r\n"
			"Niggas in Paris!\n";
	last_activity = time(NULL);
}

Client::~Client() {}

bool	Client::getRequest(std::vector<struct pollfd> &fds, size_t *index) {
	char buffer[1024];
	int bytes_received = 1;

	std::memset(buffer, 0, sizeof(buffer));
	bytes_received = recv(fd, buffer, sizeof(buffer) - 1, 0);

	if (bytes_received <= 0) {
		if (bytes_received < 0)
			std::cerr << "Error receiving data\n";
		if (request.parse_state != END && request.body_type == CONTENT &&
				request.body.length() < request.body_length) {
				request.status = 400; // Incomplete body
				state = WRITING;
				fds[*index].events = POLLOUT | POLLERR | POLLHUP | POLLNVAL;
				return true;
		}
		return false;   // signal to kickClient(...)
	}
	last_activity = time(NULL);
	request_raw.append(buffer, bytes_received);

	if (request.parse(request_raw) || request.status != 0) {
		state = WRITING;
		fds[*index].events = POLLOUT | POLLERR | POLLHUP | POLLNVAL; // Change to POLLOUT for writing response
		std::cout << "------request received from client " << fd << "------" << std::endl;
		std::cout << request_raw << std::endl;
	}
	std::cout << "left here" << std::endl;
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
	if (response.headers["Connection"] == "close" || response.status_code != 200) {
		return false; // Signal to kickClient(...)
	}
	last_activity = time(NULL);
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
