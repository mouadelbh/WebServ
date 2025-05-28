/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 14:33:47 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/28 09:39:32 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/include.hpp"

std::string readFile(std::string const &path) {
	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open())
		throw std::runtime_error("Failed to open file: " + path);

	std::ostringstream ss;
	ss << file.rdbuf();
	file.close();

	return ss.str();
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
