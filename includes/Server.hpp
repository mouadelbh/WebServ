/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 10:05:55 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/08/07 17:03:33 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "include.hpp"
#include "../Parse_Config/Parser.hpp"

class Client;
class ServerConfig;

class Server {
	public:
		ServerConfig *config;
		int	socket_fd;
		int	index;

		Server();
		~Server();
		void	initServer();
		void	AcceptConnection(std::map<int, Client> &clients);
};

