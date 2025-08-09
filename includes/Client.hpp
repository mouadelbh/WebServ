/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 10:05:05 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/08/09 18:01:06 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "include.hpp"

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
class ServerConfig;
struct LocationConfig;

class Response {
	public:
		bool CGI_active;
		std::string version;
		int status_code;
		std::string status_message;
		std::string uri;
		std::map<std::string, std::string> headers;
		std::string body;
		Request *request;
		LocationConfig *locationConfig;
		ServerConfig *config;

		Response();
		~Response();
		bool	isCGIRequest(const std::string &path);
		void	executeCGI();
		void	buildStatusLine();
		void	buildHeaders();
		void	buildPostHeaders();
		void	buildGetBody();
		void	executePostBody();
		void	uploadRawFile();
		void	uploadMultiForm(std::string &content_type);
		void	saveDataToFile(const std::string &data, std::string filename);
		void	saveMultiFormDataToFile(std::string data);
		void	uploadUrlEncoded();
		void	saveFormDataToFile(const std::map<std::string, std::string> &form_data);
		void	executeDeleteBody();
		void	build();
		void	createBody();
		void	setStatus(int code, const std::string &message);
		void	clear();
		void	setGetPath();
		void	debugResponse() const;
		bool	autoIndexStatus();
		std::string	getIndex();
		std::string	getErrorPage(int code);
		std::string toString() const;
};

class Request {
	public:
		ParseState	parse_state;
		std::string method;
		std::string path;
		std::string version;
		std::map<std::string, std::string> headers;
		std::string body;
		std::string dir;
		int status;
		int Index;
		size_t  body_length;
		size_t chunk_size;
		BodyType	body_type;
		ChunkParseState chunk_state;
		LocationConfig *locationConfig;
		ServerConfig *config;

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
		bool	getDirectoryFromPath();
		bool	MethodAllowed();
		bool	bodySizeOk(size_t size) const;
		bool	autoIndexStatus();
		bool	isRedirect();
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
	ServerConfig *config;
	size_t	bytes_read;

	Client();
	Client(int fd, sockaddr_in addr, socklen_t addr_len, ServerConfig *config);
	~Client();
	void	buildResponse();
	bool	getRequest(std::vector<struct pollfd> &fds, size_t *index);
	bool	sendResponse(std::vector<struct pollfd> &fds, size_t *index);
	void	clear();
};
