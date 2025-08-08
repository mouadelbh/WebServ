/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 05:28:14 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/08/08 13:56:09 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"

bool	Request::getDirectoryFromPath()
{
	std::string::size_type pos = path.find_last_of('/');
	if (pos != std::string::npos) {
		dir = path.substr(0, pos);
		if (config && config->locations.find(dir) != config->locations.end()) {
			locationConfig = &config->locations[dir];
		}
		return true;
	}
	return false;
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
	while (locationConfig && locationConfig->redirect.empty()) {
		path = locationConfig->redirect;
		getDirectoryFromPath();
		status = 301;
	}
	if (!MethodAllowed()){
		status = 501;
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
	// FIX: Change < to <= to process the last header
	while (pos <= headers_str.size()) {
		end = headers_str.find("\r\n", pos);
		if (end == std::string::npos) {
				// If no more \r\n, it might be the last header without a trailing CRLF
				// This can happen depending on how the substring is passed.
				// We'll let the line processing handle it.
				end = headers_str.length();
		}
		std::string line = headers_str.substr(pos, end - pos);
		if (line.empty()) {
			pos = end + 2;
			if (end >= headers_str.length()) break; // Exit if we are at the end
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
	auto it_host = headers.find("Host");
	if (it_host == headers.end() || it_host->second.empty()) {
		status = 400;
		return false;
	}
	if (method == "POST") {
		auto it_content_type = headers.find("Content-Type");
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
			return false; // Wait for more data
		}
		if (!parseRequestLine(buffer.substr(Index, line_end - Index))) {
			return false; // Error in request line
		}
		Index = line_end + 2;
		parse_state = HEADERS;
	}

	if (parse_state == HEADERS) {
		size_t headers_end = buffer.find("\r\n\r\n", Index);
		if (headers_end == std::string::npos) {
			return false; // Wait for more data
		}
		if (!parseHeaders(buffer.substr(Index, headers_end - Index))) {
			std::cout << status << std::endl;
			return false; // Error in headers
		}
		Index = headers_end + 4;
		parse_state = INFO;
	}
	if (parse_state == INFO) {
		if (!getBodyInfo())
			return false; // Error in body info
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
