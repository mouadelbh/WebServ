/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 13:50:04 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/24 13:56:18 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server() : socket_fd(-1) {}

Server::~Server() {
	for (struct pollfd fd : fds) {
		close(fd.fd);
	}
}

void	Server::initServer(std::vector<struct pollfd> &fds, int &socket_fd) {
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

void	Server::AcceptConnection(std::unordered_map<int, Client> &clients, std::vector<struct pollfd> &fds, int socket_fd) {
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

void	Server::runServer(std::vector<struct pollfd> &fds, int socket_fd, std::unordered_map<int, Client> &clients) {
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
				this->AcceptConnection(clients, fds, socket_fd);
			else if (fds[i].revents & POLLIN && clients[current_fd].state != WRITING)
				getRequest(clients, fds, &i);
			else if (fds[i].revents & POLLOUT && clients[current_fd].state == WRITING)
				sendResponse(clients, fds, &i);
		}
	}
	close(socket_fd);
}
