/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 10:08:13 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/08/09 20:14:42 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"

Client::Client() : fd(-1), addr(), addr_len(0), state(READING), bytes_read(0) {
	response_raw = "HTTP/1.1 200 OK\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length: 17\r\n"
				"\r\n"
				"Niggas in Paris!\n";
	last_activity = time(NULL);
}

Client::Client(int fd, sockaddr_in addr, socklen_t addr_len, ServerConfig *config)
	: fd(fd), addr(addr), addr_len(addr_len), bytes_read(0) {
	this->config = config;
	request.config = config;
	response.config = config;
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
				request.status = 400;
				state = WRITING;
				fds[*index].events = POLLOUT | POLLERR | POLLHUP | POLLNVAL;
				return true;
		}
		return false;
	}
	last_activity = time(NULL);
	request_raw.append(buffer, bytes_received);

	if (request.parse(request_raw) || request.status != 0) {
		state = WRITING;
		fds[*index].events = POLLOUT | POLLERR | POLLHUP | POLLNVAL;
	}
	return true;
}

void	Client::buildResponse() {
	response.request = &request;
	response.locationConfig = request.locationConfig;
	response.build();
	response_raw = response.toString();
}

bool Client::sendResponse(std::vector<struct pollfd> &fds, size_t *index) {
	std::string buffer;
	if (bytes_read == 0) {
		this->buildResponse();
		buffer = response_raw;
	}
	if (response_raw.size() > MAX_CHUNK_SIZE) {
		buffer = response_raw.substr(bytes_read, MAX_CHUNK_SIZE);
		bytes_read += buffer.size();
	}

	int bytes_sent = send(fd, buffer.c_str(), buffer.size(), 0);
	if (bytes_sent < 0) {
		std::cerr << "Error sending response: " << strerror(errno) << std::endl;
		return false;
	}

	last_activity = time(NULL);
	if (bytes_read >= response_raw.size() || bytes_read == 0) {
		bytes_read = 0;
		state = READING;
		fds[*index].events = POLLIN | POLLHUP | POLLERR | POLLNVAL; 
		this->clear();
	}
	return true;
}

void	Client::clear() {
	bytes_read = 0;
	response_raw.clear();
	request_raw.clear();
	request.clear();
	response.clear();
}






