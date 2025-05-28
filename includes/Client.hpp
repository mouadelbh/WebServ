/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 10:05:05 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/28 15:52:33 by mel-bouh         ###   ########.fr       */
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

class Request;
class Response;

class Response {
	public:
		std::string version;
		int status_code;
		std::string status_message;
		std::string uri;
		std::unordered_map<std::string, std::string> headers;
		std::string body;

		Response();
		~Response();
		void	buildStatusLine(Request &request);
		void	buildHeaders(Request &request);
		void	buildBody(Request &request);
		void	build(Request &request);
		void	setStatus(int code, const std::string &message);
		void	clear();
		void	setPath(Request &request);
		std::string toString() const;
};

class Request {
	public:
		std::string method;
		std::string path;
		std::string version;
		std::unordered_map<std::string, std::string> headers;
		std::string body;
		int status;

		Request();
		~Request();

		bool	parseHeaders(const std::string &request_str, size_t &curr);
		bool	parseRequestLine(const std::string &request_str);
		void	parseBody(const std::string &method);
		void	parse(const std::string &request_str);
		void	clear();
		bool	pathIsValid(int indexing);
		std::string	getType();
		std::string toString() const;
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
		bool	getRequest(std::vector<struct pollfd> &fds, size_t *index);
		bool	sendResponse(std::vector<struct pollfd> &fds, size_t *index);
		void	clear();
};
