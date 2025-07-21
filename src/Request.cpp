/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 16:07:51 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/06/20 05:29:09 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"

Request::Request() : status(0), autoIndex(0), body_length(0), chunk_size(0), body_type(NONE), server_config(nullptr) {
	parse_state = REQUEST_LINE;
	chunk_state = SIZE;
}

Request::~Request() {
	clear();
}

void Request::setServerConfig(const ServerConfig* config)
{
	server_config = config;
}

std::string Request::toString() const 
{
	std::string request_line = method + " " + path + " " + version + "\r\n";
	std::string headers_str;
	for (const auto &header : headers) {
		headers_str += header.first + ": " + header.second + "\r\n";
	}
	return request_line + headers_str + "\r\n" + body;
}

bool	Request::getBodyInfo() {
	auto it_content = headers.find("Content-Length");
	auto it_chunked = headers.find("Transfer-Encoding");

	if (it_chunked != headers.end() && it_content != headers.end()) {
		status = 400; return false;
	}
	if (it_chunked != headers.end()) {
		if (it_chunked->second != "chunked") {
			status = 400; return false;
		}
		body_type = CHUNKED;
	} else if (it_content != headers.end()) {
		body_type = CONTENT;
		if (!isNumber(it_content->second)) {
			status = 400;
			return false;
		}
		body_length = std::stoi(it_content->second);
		// body_length is size_t (unsigned), so no need to check < 0
		if (body_length > MAX_BODY_SIZE) {
			status = 413;
			return false;
		}
	} else
		body_type = NONE;

	if (body_type == NONE && (method == "POST" || method == "PUT")) {
		status = 411;
		return false;
	}
	return true;
}

bool	Request::getChunkSize(const std::string& buffer) {
	size_t index = buffer.length();
	size_t stop = buffer.find(";", autoIndex);
	if (stop != std::string::npos)
		index = stop;
	for (int i = 0; i < static_cast<int>(index); i++) {
		if (!std::isxdigit(buffer[i])) {
			status = 400; // Invalid chunk size
			chunk_size = 0;
			return false;
		}
		chunk_size = chunk_size * 16 + (buffer[i] >= '0' && buffer[i] <= '9' ? buffer[i] - '0' : std::tolower(buffer[i]) - 'a' + 10);
	}
	// chunk_size is size_t (unsigned), so no need to check < 0
	if (chunk_size > MAX_CHUNK_SIZE) {
		status = 413; // Chunk size too large
		chunk_size = 0;
		return false;
	}
	return true;
}

bool	Request::parseChunkedBody(const std::string& buffer) {
	size_t size_end;
	while (autoIndex < (int)buffer.length()) {
		if (chunk_state == SIZE) {
			size_end = buffer.find("\r\n", autoIndex);
			if (size_end == std::string::npos) {
				return false; // Wait for more data
			}
			if (!getChunkSize(buffer.substr(autoIndex, size_end - autoIndex)))
				return false; // Error in chunk size
			autoIndex = size_end + 2;
			if (chunk_size == 0)
				chunk_state = CHUNK_DONE; // End of chunks
			else
				chunk_state = DATA;
		}
		if (chunk_state == DATA) {
			if (buffer.length() - autoIndex < chunk_size + 2) {
				return false; // Wait for more data
			}
			if (buffer.substr(autoIndex + chunk_size, 2) != "\r\n") {
				status = 400; // Invalid chunk data
				return false;
			}
			body += buffer.substr(autoIndex, chunk_size);
			autoIndex += chunk_size + 2; // Move past chunk data and CRLF
			chunk_state = SIZE;
			chunk_size = 0;
		}
		if (chunk_state == CHUNK_DONE) {
			return true;
		}
	}
	return false;
}

std::string	Request::getType() {
	if (endsWith(path, ".html")) return "text/html";
	if (endsWith(path, ".css")) return "text/css";
	if (endsWith(path, ".png")) return "image/png";
	if (endsWith(path, ".jpg") || endsWith(path, ".jpeg")) return "image/jpeg";
	if (endsWith(path, ".gif")) return "image/gif";
	if (endsWith(path, ".js")) return "application/javascript";
	if (autoIndex && isDirectory(path)) return "text/html";
	return "application/octet-stream";
}

bool	Request::pathIsValid(int index) {
	(void)index;
	if (open(path.c_str(), O_RDONLY) == -1)
		return false;
	return true;
}

void	Request::clear() {
	status = 0;
	autoIndex = 0;
	body_length = 0;
	parse_state = REQUEST_LINE;
	chunk_state = SIZE;
	chunk_size = 0;
	body_type = NONE;
	method.clear();
	path.clear();
	version.clear();
	headers.clear();
	body.clear();
}

const LocationConfig* Request::findLocation(const std::string& requestPath) {
	if (!server_config) return nullptr;
	
	const LocationConfig* bestMatch = nullptr;
	size_t longestMatch = 0;
	
	for (const auto& location : server_config->locations)
	{
		// Handle exact path matches
		if (location.path == requestPath) 
		{
			return &location;
		}
		
		// Handle prefix matches
		if (requestPath.find(location.path) == 0) 
		{
			if (location.path.length() > longestMatch) 
			{
				longestMatch = location.path.length();
				bestMatch = &location;
			}
		}
		
		// Handle extension matches (*.ext)
		if (location.path.find("*.") == 0) 
		{
			std::string extension = location.path.substr(1); // Remove *
			if (endsWith(requestPath, extension)) {
				return &location;
			}
		}
	}
	
	return bestMatch;
}

bool Request::isMethodAllowed(const std::string& requestPath) {
	if (!server_config) return true; // Default allow if no config
	
	const LocationConfig* location = findLocation(requestPath);
	std::vector<std::string> allowedMethods;
	
	if (location && !location->methods.empty()) 
	{
		allowedMethods = location->methods;
	}
	else 
	{
		allowedMethods = server_config->methods;
	}
	
	// If no methods specified, allow all
	if (allowedMethods.empty()) return true;
	
	// Check if current method is in allowed list
	for (const auto& allowedMethod : allowedMethods) {
		if (method == allowedMethod) {
			return true;
		}
	}
	
	return false;
}
