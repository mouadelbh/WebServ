/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 16:21:14 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/08/05 15:36:42 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/include.hpp"
#include "../Parse_Config/Parser.hpp"

bool	run = true;
bool 	autoIndex = true;
const ServerConfig* g_current_server_config = nullptr;

int main(int ac, char **av)
{
	std::string config_path = "Parse_Config/default.conf";
	if (ac == 2) config_path = av[1];

	try
	{
		Parser parser(config_path);
		const std::vector<ServerConfig>& servers = parser.getServers();
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

		Server server;
		std::unordered_map<int, Client> clients;
		init(av);
		g_current_server_config = &servers[0];
		server.initServer();
		server.runServer(clients);
	}
	catch (const std::exception& e) {
		std::cerr << "Configuration error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
