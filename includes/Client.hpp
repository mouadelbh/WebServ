/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 10:05:05 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/24 17:11:30 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "include.hpp"
#include "Request.hpp"
#include "Response.hpp"

#define ClientsMap std::unordered_map<int, Client>

enum ClientState {
	READING,
	WRITING,
	DONE
};

class Client {
	public:
		int	fd;
		std::string response_raw;
		std::string request_raw;
		sockaddr_in addr;
		socklen_t addr_len;
		ClientState state;
		Request request;
		Response response;

		Client();
		Client(int fd, sockaddr_in addr, socklen_t addr_len);
		~Client();
		void	buildResponse();
		void	buildStatusLine();
		void	buildHeaders();
		void	buildBody();
		bool	getRequest(std::vector<struct pollfd> &fds, size_t *index);
		bool	sendResponse(std::vector<struct pollfd> &fds, size_t *index);
};
