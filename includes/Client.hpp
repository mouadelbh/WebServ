/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 10:05:05 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/06/28 14:13:58 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "include.hpp"

#define ClientsMap std::unordered_map<int, Client>

enum BodyType { NONE, CONTENT, CHUNKED };
enum ChunkParseState { SIZE, DATA, DATA_CRLF, TRAILERS, CHUNK_DONE };


enum ParseState {
	REQUEST_LINE,
	HEADERS,
	BODY,
	INFO,
	END
};

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
		void	buildPostHeaders(Request &request);
		void	buildGetBody(Request &request);
		void	executePostBody(Request &request);
		void	uploadRawFile(Request &request);
		void	uploadUrlEncoded(Request &request);
		void	uploadMultiForm(Request &request);
		void	executeDeleteBody(Request &request);
		void	build(Request &request);
		void	createBody(Request &request);
		void	setStatus(int code, const std::string &message);
		void	clear();
		void	setGetPath(Request &request);
		std::string toString() const;
};

class Request {
	public:
		ParseState	parse_state;
		std::string method;
		std::string path;
		std::string version;
		std::unordered_map<std::string, std::string> headers;
		std::string body;
		int status;
		int autoIndex;
		size_t  body_length;
		size_t chunk_size;
		BodyType	body_type;
		ChunkParseState chunk_state;

		Request();
		~Request();

		bool	parseHeaders(const std::string &request_str);
		bool	parseRequestLine(const std::string &request_str);
		bool	parseBody(const std::string &method);
		bool	parse(const std::string &request_str);
		void	clear();
		bool	pathIsValid(int indexing);
		bool	getBodyInfo();
		bool	parseChunkedBody(const std::string &buffer);
		bool	getChunkSize(const std::string &buffer);
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
		time_t last_activity;

		Client();
		Client(int fd, sockaddr_in addr, socklen_t addr_len);
		~Client();
		void	buildResponse();
		bool	getRequest(std::vector<struct pollfd> &fds, size_t *index);
		bool	sendResponse(std::vector<struct pollfd> &fds, size_t *index);
		void	clear();
};
