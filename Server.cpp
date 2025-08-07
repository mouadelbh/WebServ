/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 13:50:04 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/08/07 17:11:27 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

Server::Server() : socket_fd(-1), index(0) {}

Server::~Server() {
	for (struct pollfd fd : fds) {
		close(fd.fd);
	}
}

void	Server::initServer() {
	int opt = 1;

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
		std::cerr << "Error creating socket" << std::endl, exit(1);
	sockaddr_in addr;
	fds.push_back((struct pollfd){socket_fd, POLLIN | POLLERR | POLLHUP | POLLNVAL, 0});
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

void	Server::AcceptConnection(std::map<int, Client> &clients) {
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
	fcntl(client_fd, F_SETFL, O_NONBLOCK);
	clients[client_fd] = Client(client_fd, client_addr, client_len);
	fds.push_back((struct pollfd){client_fd, POLLIN | POLLHUP | POLLERR | POLLNVAL, 0});
}
