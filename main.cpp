/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 16:21:14 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/24 14:16:55 by mel-bouh         ###   ########.fr       */
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

void kickClient(std::unordered_map<int, Client> &clients, std::vector<struct pollfd> &fds, size_t *index) {
	int client_fd = fds[*index].fd;
	std::cout << "Kicking client with fd: " << client_fd << std::endl;
	close(client_fd);
	clients.erase(client_fd);
	fds.erase(fds.begin() + *index);
	*index = 0; // Reset index to avoid out of bounds
}

bool	getRequest(std::unordered_map<int, Client> &clients, std::vector<struct pollfd> &fds, size_t *index) {
	char buffer[1024];
	int client_fd = fds[*index].fd;
	int bytes_received = 1;

	Client &client = clients[client_fd];
	memset(buffer, 0, sizeof(buffer));
	while (bytes_received > 0) {
		bytes_received = recv(client.fd, buffer, sizeof(buffer) - 1, 0);
		if (bytes_received < 0) {
			std::cerr << "Error receiving data" << std::endl;
			return false;
		}
		client.request.append(buffer, bytes_received);
		if (client.request.find("\r\n\r\n") != std::string::npos) {
			break; // End of HTTP request
		}
		memset(buffer, 0, sizeof(buffer));
	}
	client.state = WRITING;
	fds[*index].events = POLLOUT | POLLERR | POLLHUP | POLLNVAL; // Change to POLLOUT for writing response
	return true;
}

bool	sendResponse(std::unordered_map<int, Client> &clients, std::vector<struct pollfd> &fds, size_t *index) {
	int client_fd = fds[*index].fd;
	Client &client = clients[client_fd];

	// Here you would process the request and prepare a response
	std::string response = client.response;
	std::cout << "Response: " << response << std::endl;
	int bytes_sent = send(client.fd, response.c_str(), response.size(), 0);
	if (bytes_sent < 0) {
		std::cerr << "Error sending response" << std::endl;
		return false;
	}

	client.state = READING; // Change state back to READING
	fds[*index].events = POLLIN | POLLHUP | POLLERR | POLLNVAL; // Change back to POLLIN for reading next request
	return true;
}

int main(int ac, char **av) {
	if (ac != 1) {
		std::cerr << "Usage: " << av[0] << std::endl;
		return 1;
	}
	Server server;
	std::unordered_map<int, Client> clients;
	init(av);
	server.initServer();
	server.runServer(clients);
}
