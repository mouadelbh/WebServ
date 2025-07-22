/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 13:50:04 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/06/20 05:14:55 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

Server::Server() : socket_fd(-1), index(0) {}

Server::~Server() {
	for (struct pollfd fd : fds) {
		close(fd.fd);
	}
}

const ServerConfig& Server::getConfig() const {
	return config;
}

void	Server::initServer(const ServerConfig& serverConfig) 
{
	int opt = 1;
	config = serverConfig; // Store the configuration

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
		std::cerr << "Error creating socket" << std::endl, exit(1);
	
	sockaddr_in addr;
	fds.push_back((struct pollfd){socket_fd, POLLIN | POLLERR | POLLHUP | POLLNVAL, 0});
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	
	if (config.host == "localhost") {
		addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	} else if (config.host == "127.0.0.1") {
		addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	} else {
		addr.sin_addr.s_addr = INADDR_ANY;
	}
	
	addr.sin_port = htons(config.port);
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		std::cerr << "Error binding socket on " << config.host << ":" << config.port << std::endl, exit(1);
	if (listen(socket_fd, 100) < 0)
		std::cerr << "Error listening on socket" << std::endl, exit(1);
	
	std::cout << "Server '" << config.server_name << "' initialized on " << config.host << ":" << config.port << std::endl;
}

void	Server::AcceptConnection(std::unordered_map<int, Client> &clients) {
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
	clients[client_fd].setServerConfig(&config); // Pass server configuration to client
	fds.push_back((struct pollfd){client_fd, POLLIN | POLLHUP | POLLERR | POLLNVAL, 0});
}

void	Server::runServer(std::unordered_map<int, Client> &clients) 
{
	while (run) 
	{
		try {
			int res = poll(fds.data(), fds.size(), -1);
			if (res < 0 || run == false)
				break ;
			for (size_t i = 0; i < fds.size(); i++) {
			int current_fd = fds[i].fd;

			if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
				if (current_fd == socket_fd) {
					std::cerr << "SERVER SOCKET ERROR!" << std::endl;
					break;
				}
				else
					kickClient(clients, fds, &i);
				continue;
			}

			if (fds[i].revents & POLLIN && current_fd == socket_fd)
				this->AcceptConnection(clients);
			else if (fds[i].revents & POLLIN && clients[current_fd].state != WRITING)
				clients[current_fd].getRequest(fds, &i) ? (void)0 : kickClient(clients, fds, &i);
			else if (fds[i].revents & POLLOUT && clients[current_fd].state == WRITING)
				clients[current_fd].sendResponse(fds, &i) ? (void)0 : kickClient(clients, fds, &i);
			}
		}
		catch (const std::bad_alloc &e) {
			std::cerr << "Memory allocation failed: " << e.what() << std::endl;
		}
		catch (const std::exception &e) {
			std::cerr << "An error occurred: " << e.what() << std::endl;
		}
		catch (...) {
			std::cerr << "An unknown error occurred." << std::endl;
		}
	}
	close(socket_fd);
}
