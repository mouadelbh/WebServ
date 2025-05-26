/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 14:33:47 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/26 18:26:48 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/include.hpp"

std::string readFile(std::string const &path) {
	std::ostringstream ss;
	std::string body;
	body.clear();
	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "Error opening file: " << path << std::endl;
		return body;
	}
	ss << file.rdbuf();
	body = ss.str();
	file.close();
	if (body.empty()) {
		std::cerr << "File is empty: " << path << std::endl;
		return body;
	}
	body.erase(body.find_last_not_of("\n") + 1); // Remove trailing newline
	return body;
}

bool	endsWith(const std::string& str, const std::string& suffix) {
	if (str.length() < suffix.length()) return false;
	return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
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
