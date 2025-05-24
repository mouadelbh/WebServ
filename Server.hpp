/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 10:05:55 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/24 14:03:50 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "include.hpp"

class Client;

class Server {
	public:
		std::vector<struct pollfd> fds;
		int socket_fd;

		Server();
		~Server();
		void	initServer();
		void	AcceptConnection(std::unordered_map<int, Client> &clients);
		void	runServer(std::unordered_map<int, Client> &clients);
};

