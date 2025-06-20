/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 05:28:14 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/06/20 05:30:37 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"

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
	if (method != "GET" && method != "POST" && method != "DELETE") {
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
		size_t line_end = buffer.find("\r\n", autoIndex);
		if (line_end == std::string::npos) {
			return false; // Wait for more data
		}
		if (!parseRequestLine(buffer.substr(autoIndex, line_end - autoIndex))) {
			return false; // Error in request line
		}
		autoIndex = line_end + 2;
		parse_state = HEADERS;
		std::cout << "Parsed Request Line: method=" << method << ", path=" << path << ", version=" << version << std::endl;
	}

	if (parse_state == HEADERS) {
		size_t headers_end = buffer.find("\r\n\r\n", autoIndex);
		if (headers_end == std::string::npos) {
			return false; // Wait for more data
		}
		if (!parseHeaders(buffer.substr(autoIndex, headers_end - autoIndex))) {
			std::cout << status << std::endl;
			return false; // Error in headers
		}
		autoIndex = headers_end + 4;
		parse_state = INFO;
		std::cout << "Parsed Headers:\n";
    	for (auto &h : headers)
     	   std::cout << h.first << ": " << h.second << "\n";
	}
	if (parse_state == INFO) {
		if (!getBodyInfo())
			return false; // Error in body info
		parse_state = BODY;
		 std::cout << "Body type = " << body_type << ", length = " << body_length << std::endl;
	}
	if (parse_state == BODY) {
		std::cout << "Parsing body...\n";
		if (body_type == CONTENT) {
			if (buffer.length() - autoIndex < body_length)
				return false;
			if (!parseBody(buffer.substr(autoIndex)))
				return false;
		}
		if (body_type == CHUNKED) {
			if (!parseChunkedBody(buffer))
				return false;
		}
		parse_state = END;
		std::cout << "Parsed Body: " << body << std::endl;
	}
	return parse_state == END;
}
