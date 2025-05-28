/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 16:07:51 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/28 13:15:09 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"

Request::Request() : status(0) {}

Request::~Request() {
	clear();
}

std::string Request::toString() const {
	std::string request_line = method + " " + path + " " + version + "\r\n";
	std::string headers_str;
	for (const auto &header : headers) {
		headers_str += header.first + ": " + header.second + "\r\n";
	}
	return request_line + headers_str + "\r\n" + body;
}

bool	Request::parseRequestLine(const std::string &line) {
	size_t pos, end;

	pos = line.find(' ');
	end = line.find(' ', pos + 1);
	if (pos == std::string::npos || end == std::string::npos || pos == 0 || end == pos + 1 ||
		end == line.length() - 1) {
		status = 400;
		return false;
	}
	method = line.substr(0, pos);
	path = line.substr(pos + 1, end - pos - 1);
	version = line.substr(end + 1);
	if (!isKnownMethod(method) || !isValidRequestPath(path) || !isValidHttpVersion(version)) {
		status = 400;
		return false;
	}
	if (method != "GET") {
		status = 501;
		return false;
	}
	if (version != "HTTP/1.1" && version != "HTTP/1.0") {
		status = 505;
		return false;
	}
	return true;
}

bool	Request::parseHeaders(const std::string &headers, size_t &curr) {
	size_t	pos = 0;
	size_t	end, colon, vstart;
	std::string temp, key, value;

	while (true) {
		end = headers.find("\r\n", pos);
		if (end == std::string::npos) {
			status = 400;
			return false;
		}
		temp = headers.substr(pos, end - pos);
		if (temp.empty()) {
			curr += end + 2;
			break ;
		}
		colon = temp.find(":");
		if (colon == std::string::npos || colon == 0 || colon == temp.length() - 1) {
			status = 400;
			return false;
		}
		key = temp.substr(0, colon), value = temp.substr(colon + 1);
		vstart = value.find_first_not_of(" \t");
		if (vstart != std::string::npos)
			value = value.substr(vstart);
		if (!isValidHeaderKey(key) || !isValidHeaderValue(value)) {
			status = 400;
			return false;
		}
		this->headers[key] = value;
		pos = end + 2;
	}
	return true;
}

void	Request::parseBody(const std::string &body) {
	std::unordered_map<std::string, std::string>::iterator it = headers.find("Content-Length");
	if (it == headers.end() && (method == "POST" || method == "PUT")) {
		status = 411;
		return ;
	}
	else if (it != headers.end()) {
		if (!isNumber(it->second)) {
			status = 400;
			return ;
		}
		int content_length = std::stoi(it->second);
		if (content_length < 0) {
			status = 400;
			return ;
		}
		else if (content_length > MAX_BODY_SIZE) {
			status = 413;
			return ;
		}
		if (body.length() < static_cast<size_t>(content_length)) {
			status = 400;
			return ;
		}
		this->body = body.substr(0, content_length);
	}
}

void	Request::parse(const std::string &str) {
	size_t	pos = 0;
	size_t	end = str.find("\r\n", pos);

	if (end == std::string::npos) {
		status = 400;
		return;
	}

	clear();
	if (!parseRequestLine(str.substr(pos, end)))
		return ;
	pos = end + 2;
	if (!parseHeaders(str.substr(pos), pos))
		return ;
	parseBody(str.substr(pos));
	if (path == "/")
		path = "www/index.html";
	else if (endsWith(path, "/"))
		path = "www" + path + "index.html";
	else
		path = "www" + path;
}

std::string	Request::getType() {
	if (endsWith(path, ".html")) return "text/html";
	if (endsWith(path, ".css")) return "text/css";
	if (endsWith(path, ".png")) return "image/png";
	if (endsWith(path, ".jpg") || endsWith(path, ".jpeg")) return "image/jpeg";
	if (endsWith(path, ".gif")) return "image/gif";
	if (endsWith(path, ".js")) return "application/javascript";
	return "application/octet-stream";
}

bool	Request::pathIsValid(int index) {
	(void)index;
	if (open(path.c_str(), O_RDONLY) == -1)
		return false;
	return true;
}

void	Request::clear() {
	method.clear();
	path.clear();
	version.clear();
	headers.clear();
	body.clear();
}
