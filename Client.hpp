/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 10:05:05 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/24 14:16:33 by mel-bouh         ###   ########.fr       */
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
		std::string response;
		std::string request;
		sockaddr_in addr;
		socklen_t addr_len;
		ClientState state;

		Client();
		Client(int fd, sockaddr_in addr, socklen_t addr_len);
		// bool	getRequest(std::unordered_map<int, Client> &clients, std::vector<struct pollfd> &fds, size_t *index);
		// bool	sendResponse(std::unordered_map<int, Client> &clients, std::vector<struct pollfd> &fds, size_t *index);
};
