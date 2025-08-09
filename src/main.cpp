/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 16:21:14 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/08/09 18:36:13 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/include.hpp"
#include "../Parse_Config/Parser.hpp"

bool	run = true;
bool 	autoIndex = true;
std::vector<struct pollfd> fds;

void	close_clients(std::map<int, Client> &clients) {
	std::map<int, Client>::iterator it;
	for (it = clients.begin(); it != clients.end(); ++it) {
		close(it->second.fd);
	}
	clients.clear();
}

void	close_sockets(std::map<int, Server> &servers, std::map<int, Client> &clients) {
	close_clients(clients);
	std::map<int, Server>::iterator server_it;
	for (server_it = servers.begin(); server_it != servers.end(); ++server_it) {
		close(server_it->second.socket_fd);
	}
	servers.clear();
	fds.clear();
}

void	runServer(std::map<int, Server> &servers) {
	bool client_kicked;
	std::map<int, Client> clients;
	while (run) {
		try {
			int res = poll(fds.data(), fds.size(), -1);

			if (res < 0 || run == false)
				break ;
			for (size_t i = 0; i < fds.size();) {
				int current_fd = fds[i].fd;
				client_kicked = false;
				if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
					if (servers.find(current_fd) != servers.end()) {
						std::cerr << "SERVER SOCKET ERROR!" << std::endl;
						break;
					}
					else
						kickClient(clients, fds, &i, &client_kicked);
					continue;
				}

				if (servers.find(current_fd) != servers.end()) {
					if (fds[i].revents & POLLIN)
						servers[current_fd].AcceptConnection(clients);
					i++;
					continue;
				}
				if (clients.find(current_fd) != clients.end() && !checkIdle(clients[current_fd])) {
					kickClient(clients, fds, &i, &client_kicked);
					continue;
				}
				if (fds[i].revents & POLLIN && clients[current_fd].state != WRITING)
					clients[current_fd].getRequest(fds, &i) ? (void)0 : kickClient(clients, fds, &i, &client_kicked);
				else if (fds[i].revents & POLLOUT && clients[current_fd].state == WRITING) {
					if (clients[current_fd].sendResponse(fds, &i)) {
						std::cerr << "ghyrha mnhna " << current_fd << std::endl;
					} else {
						std::cerr << "Error sending response to client " << current_fd << std::endl;
						kickClient(clients, fds, &i, &client_kicked);
					}
					// clients[current_fd].sendResponse(fds, &i) ? (void)0 : kickClient(clients, fds, &i, &client_kicked);
				}
				if (!client_kicked)
					i++;
			}
		}
		catch (const std::bad_alloc &e) {
			std::cerr << "Memory allocation failed: " << e.what() << std::endl;
			close_clients(clients);
		}
		catch (const std::exception &e) {
			std::cerr << "An error occurred: " << e.what() << std::endl;
			close_clients(clients);
		}
		catch (...) {
			std::cerr << "An unknown error occurred." << std::endl;
			close_clients(clients);
		}
	}
	close_sockets(servers, clients);
}

int main(int ac, char **av)
{
	std::string config_path = "Parse_Config/default.conf";
	if (ac == 2) config_path = av[1];

	try
	{
		Parser parser(config_path);
		std::vector<ServerConfig> servers = parser.getServers();
		if (servers.empty()) {
			std::cerr << "No valid server config found!" << std::endl;
			return 1;
		}

		std::cout << "Found " << servers.size() << " server configuration(s)" << std::endl;

		for (size_t i = 0; i < servers.size(); ++i)
		{
			std::cout << "Server " << (i + 1) << ": " << servers[i].server_name
					  << " on " << servers[i].host << ":" << servers[i].port << std::endl;
		}

		std::vector<Server> starter;
		std::map<int, Server> servers_map;
		std::map<int, Client> clients;
		init(av);
		printf("Servers initialized successfully.\n");
		for (size_t i = 0; i < servers.size(); ++i) {
			starter.push_back(Server());
		}
		for (size_t i = 0; i < servers.size(); ++i) {
			starter[i].config = &servers[i];
			starter[i].initServer();
		}
		for (size_t i = 0; i < servers.size(); ++i) {
			servers_map[starter[i].socket_fd] = starter[i];
			std::cout << "socket_fd "<< servers_map[starter[i].socket_fd].socket_fd << std::endl;
		}
		runServer(servers_map);
	}
	catch (const std::exception& e) {
		std::cerr << "Configuration error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
