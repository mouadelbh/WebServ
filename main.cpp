/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 16:21:14 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/24 13:52:26 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "include.hpp"

bool run = true;

void terminate_server(int sig) {
	if (sig == SIGINT) {
		run = false;
		std::cout << "\nServer shutting down..." << std::endl;
	}
	// return 0;
}

void init(char **av) {
	(void)av;
	signal(SIGINT, terminate_server);
}

void	getRequest(std::unordered_map<int, Client> &clients, std::vector<struct pollfd> &fds, size_t *index) {
	char buffer[1024];
	int client_fd = fds[*index].fd;
	int bytes_received = 1;

	if (clients.find(client_fd) == clients.end()) {
		std::cerr << "Error: Tried to process non-client fd " << client_fd << std::endl;
		return;
	}
	Client &client = clients[client_fd];
	memset(buffer, 0, sizeof(buffer));
	while (bytes_received > 0) {
		bytes_received = recv(client.fd, buffer, sizeof(buffer) - 1, 0);
		if (bytes_received < 0) {
			std::cerr << "Error receiving data" << std::endl;
			close(client.fd);
			fds.erase(fds.begin() + *index);
			clients.erase(client_fd);
			*index -= 1;
			return;
		}
		client.request.append(buffer, bytes_received);
		if (client.request.find("\r\n\r\n") != std::string::npos) {
			break; // End of HTTP request
		}
		memset(buffer, 0, sizeof(buffer));
	}
	client.state = WRITING;
	fds[*index].events = POLLOUT | POLLERR | POLLHUP | POLLNVAL; // Change to POLLOUT for writing response
}

void	sendResponse(std::unordered_map<int, Client> &clients, std::vector<struct pollfd> &fds, size_t *index) {
	int client_fd = fds[*index].fd;

	if (clients.find(client_fd) == clients.end()) {
		std::cerr << "Error: Tried to send response to non-client fd " << client_fd << std::endl;
		return;
	}
	Client &client = clients[client_fd];

	// Here you would process the request and prepare a response
	std::string response = client.response;
	std::cout << "Response: " << response << std::endl;
	int bytes_sent = send(client.fd, response.c_str(), response.size(), 0);
	if (bytes_sent < 0) {
		std::cerr << "Error sending response" << std::endl;
		close(client.fd);
		fds.erase(fds.begin() + *index);
		clients.erase(client_fd);
		*index -= 1;
		return;
	}

	client.state = READING; // Change state back to READING
	fds[*index].events = POLLIN | POLLHUP | POLLERR | POLLNVAL; // Change back to POLLIN for reading next request
}

int main(int ac, char **av) {
	if (ac != 1) {
		std::cerr << "Usage: " << av[0] << std::endl;
		return 1;
	}
	Server server;
	std::vector<struct pollfd> &fds = server.fds;
	std::unordered_map<int, Client> clients;
	init(av);
	server.initServer(server.fds, server.socket_fd);
	server.runServer(fds, server.socket_fd, clients);
}
