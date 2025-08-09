/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 05:28:14 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/08/09 20:16:54 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"

bool	Request::getDirectoryFromPath()
{
	if (isDirectory(path)) {
		dir = path;
		if (dir[dir.length() - 1] == '/')
			dir = dir.substr(0, dir.length() - 1);
		if (config && config->locations.find(dir) != config->locations.end()) {
			locationConfig = &config->locations[dir];
		}
	}
	else {
		size_t last_slash = path.find_last_of('/');
		if (last_slash != std::string::npos) {
			dir = path.substr(0, last_slash);
			if (dir[dir.length() - 1] == '/')
				dir = dir.substr(0, dir.length() - 1);
			if (config && config->locations.find(dir) != config->locations.end()) {
				locationConfig = &config->locations[dir];
			}
		}
	}
	return true;
}

bool	Request::MethodAllowed() {
	if (locationConfig && !locationConfig->methods.empty()) {
		if (std::find(locationConfig->methods.begin(), locationConfig->methods.end(), method) != locationConfig->methods.end()) {
			return true;
		}
	}
	else if (config && !config->methods.empty()) {
		if (std::find(config->methods.begin(), config->methods.end(), method) != config->methods.end()) {
			return true;
		}
	}
	else {
		if (method == "GET" || method == "DELETE" || method == "POST") {
			return true;
		}
	}
	return false;
}

bool Request::isRedirect() {
	if (locationConfig && !locationConfig->redirect.empty()) {
		path = locationConfig->redirect;
		return true;
	}
	return false;
}

bool Request::parseRequestLine(const std::string& line) {
	size_t pos, end;

	pos = line.find(' ');
	end = line.find(' ', pos + 1);
	if (pos == std::string::npos || end == std::string::npos || pos == 0 || end == pos + 1 || end == line.length() - 1) {
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
	getDirectoryFromPath();
	if (!MethodAllowed()){
		status = 501;
		return false;
	}
	if (isRedirect()) {
		status = 301;
		return false;
	}
	if (version != "HTTP/1.1" && version != "HTTP/1.0") {
		status = 505;
		return false;
	}
	return true;
}

bool Request::parseHeaders(const std::string& headers_str) {
	size_t pos = 0, end;
	while (pos <= headers_str.size()) {
		end = headers_str.find("\r\n", pos);
		if (end == std::string::npos) {
				end = headers_str.length();
		}
		std::string line = headers_str.substr(pos, end - pos);
		if (line.empty()) {
			pos = end + 2;
			if (end >= headers_str.length()) break;
			continue;
		}
		size_t colon = line.find(':');
		if (colon == std::string::npos || colon == 0 || colon == line.length() - 1) {
			status = 400;
			return false;
		}
		std::string key = line.substr(0, colon);
		std::string value = line.substr(colon + 1);
		size_t vstart = value.find_first_not_of(" \t");
		if (vstart != std::string::npos)
			value = value.substr(vstart);
		if (!isValidHeaderKey(key) || !isValidHeaderValue(value)) {
			status = 400;
			return false;
		}
		headers[key] = value;
		pos = end + 2;
		if (end >= headers_str.length()) break; // Exit after processing the last part
	}
	std::map<std::string, std::string>::iterator it_host = headers.find("Host");
	if (it_host == headers.end() || it_host->second.empty()) {
		status = 400;
		return false;
	}
	if (method == "POST") {
		std::map<std::string, std::string>::iterator it_content_type = headers.find("Content-Type");
		if (it_content_type == headers.end() || it_content_type->second.empty() \
		|| !isValidHeaderValue(it_content_type->second)) {
			status = 400;
			return false;
		}
	}
	return true;
}

bool Request::parseBody(const std::string& body_str) {
	body = body_str.substr(0, body_str.length());
	if (body.length() > body_length) {
		status = 400; return false;
	}
	return true;
}

bool Request::parse(const std::string& buffer) {
	if (parse_state == REQUEST_LINE) {
		size_t line_end = buffer.find("\r\n", Index);
		if (line_end == std::string::npos) {
			return false;
		}
		if (!parseRequestLine(buffer.substr(Index, line_end - Index))) {
			return false;
		}
		Index = line_end + 2;
		parse_state = HEADERS;
	}

	if (parse_state == HEADERS) {
		size_t headers_end = buffer.find("\r\n\r\n", Index);
		if (headers_end == std::string::npos) {
			return false;
		}
		if (!parseHeaders(buffer.substr(Index, headers_end - Index))) {
			return false;
		}
		Index = headers_end + 4;
		parse_state = INFO;
	}
	if (parse_state == INFO) {
		if (!getBodyInfo())
			return false; 
		parse_state = BODY;
	}
	if (parse_state == BODY) {
		if (body_type == CONTENT) {
			if (buffer.length() - Index < body_length) {
				return false;
			}
			if (!parseBody(buffer.substr(Index)))
				return false;
		}
		if (body_type == CHUNKED) {
			if (!parseChunkedBody(buffer))
				return false;
		}
		parse_state = END;
	}
	return parse_state == END;
}
