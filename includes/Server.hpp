/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 10:05:55 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/26 16:25:45 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "include.hpp"

class Client;

class Server {
	public:
		std::vector<struct pollfd> fds;
		int	socket_fd;
		int	index;
		ServerConfig config;

		Server();
		~Server();
		void	initServer(const ServerConfig& serverConfig);
		void	AcceptConnection(std::unordered_map<int, Client> &clients);
		void	runServer(std::unordered_map<int, Client> &clients);
		const ServerConfig& getConfig() const;
};

