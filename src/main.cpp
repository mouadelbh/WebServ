/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 16:21:14 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/26 17:48:49 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/include.hpp"

bool run = true;

int main(int ac, char **av) {
	if (ac != 1) {
		std::cerr << "Usage: " << av[0] << std::endl;
		return 1;
	}
	try {
		Server server;
		std::unordered_map<int, Client> clients;
		init(av);
		server.initServer();
		server.runServer(clients);
	}
	catch (const std::bad_alloc &e) {
		std::cerr << "Memory allocation failed: " << e.what() << std::endl;
	}
}
