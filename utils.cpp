/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 14:33:47 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/24 17:05:34 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/include.hpp"

std::string &readFile(std::string const &path) {
	static std::string body;
	body.clear();
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cerr << "Error opening file: " << path << std::endl;
		return body;
	}
	std::string line;
	while (std::getline(file, line)) {
		body += line + "\n";
	}
	file.close();
	if (body.empty()) {
		std::cerr << "File is empty: " << path << std::endl;
		return body;
	}
	body.erase(body.find_last_not_of("\n") + 1); // Remove trailing newline
	return body;
}

void	terminate_server(int sig) {
	if (sig == SIGINT) {
		run = false;
		std::cout << "\nServer shutting down..." << std::endl;
	}
}

void	init(char **av) {
	(void)av;
	signal(SIGINT, terminate_server);
}

void	kickClient(std::unordered_map<int, Client> &clients, std::vector<struct pollfd> &fds, size_t *index) {
	int client_fd = fds[*index].fd;
	std::cout << "Kicking client with fd: " << client_fd << std::endl;
	close(client_fd);
	clients.erase(client_fd);
	fds.erase(fds.begin() + *index);
	*index = 0; // Reset index to avoid out of bounds
}
