/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 10:05:05 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/24 11:33:21 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "include.hpp"

#define ClientsMap std::unordered_map<int, Client>

enum ClientState {
	READING,
	WRITING,
	DONE
};

class Client {
	public:
		int	fd;
		std::string response = "HTTP/1.1 200 OK\r\n"
								"Content-Type: text/plain\r\n"
								"Content-Length: 17\r\n"
								"\r\n"
								"Niggas in Paris!\n";
		std::string request;
		sockaddr_in addr;
		socklen_t addr_len;
		ClientState state;

		Client();
		Client(int fd, sockaddr_in addr, socklen_t addr_len);
};
