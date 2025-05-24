/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 16:21:14 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/24 13:43:51 by mel-bouh         ###   ########.fr       */
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

void initServer(std::vector<struct pollfd> &fds, int &socket_fd) {
	int opt = 1;

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
		std::cerr << "Error creating socket" << std::endl, exit(1);
	sockaddr_in addr;
	fds.push_back((struct pollfd){socket_fd, POLLIN, 0});
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = INADDR_ANY;
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		std::cerr << "Error binding socket" << std::endl, exit(1);
	if (listen(socket_fd, 100) < 0)
		std::cerr << "Error listening on socket" << std::endl, exit(1);
	std::cout << "Server initialized on port " << PORT << std::endl;
}

void	AcceptConnection(std::unordered_map<int, Client> &clients, std::vector<struct pollfd> &fds, int socket_fd) {
	int	client_fd;
	sockaddr_in client_addr;
	socklen_t client_len;

	std::cout << "Waiting for a new connection..." << std::endl;
	client_len = sizeof(client_addr);
	memset(&client_addr, 0, client_len);
	client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_len);
	if (client_fd < 0) {
		std::cerr << "Error accepting connection" << std::endl;
		return ;
	}
	clients[client_fd] = Client(client_fd, client_addr, client_len);
	fds.push_back((struct pollfd){client_fd, POLLIN | POLLHUP | POLLERR | POLLNVAL, 0});
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

void runServer(std::vector<struct pollfd> &fds, int socket_fd, std::unordered_map<int, Client> &clients) {
	while (run) {
		int res = poll(fds.data(), fds.size(), -1);
		if (res < 0) {
			std::cerr << "Poll error" << std::endl;
			break;
		}
		for (size_t i = 0; i < fds.size(); i++) {
			int current_fd = fds[i].fd;

			if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
				if (current_fd == socket_fd) {
					std::cerr << "SERVER SOCKET ERROR!" << std::endl;
					run = false;
					break;
				}
				else {
					close(current_fd);
					clients.erase(current_fd); // Only erase if it's a client
					fds.erase(fds.begin() + i);
					i--;
				}
				continue;
			}

			if (fds[i].revents & POLLIN && current_fd == socket_fd)
				AcceptConnection(clients, fds, socket_fd);
			else if (fds[i].revents & POLLIN && clients[current_fd].state != WRITING)
				getRequest(clients, fds, &i);
			else if (fds[i].revents & POLLOUT && clients[current_fd].state == WRITING)
				sendResponse(clients, fds, &i);
		}
	}
	close(socket_fd);
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
	initServer(server.fds, server.socket_fd);
	runServer(fds, server.socket_fd, clients);
}
