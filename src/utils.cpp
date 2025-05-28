/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 14:33:47 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/28 13:22:48 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/include.hpp"

std::map<int, std::string> createStatusCodeMap() {
	std::map<int, std::string> statusMap;

	statusMap[200] = "OK";
	statusMap[201] = "Created";
	statusMap[202] = "Accepted";
	statusMap[204] = "No Content";

	statusMap[301] = "Moved Permanently";
	statusMap[302] = "Found";
	statusMap[304] = "Not Modified";

	statusMap[400] = "Bad Request";
	statusMap[401] = "Unauthorized";
	statusMap[403] = "Forbidden";
	statusMap[404] = "Not Found";
	statusMap[405] = "Method Not Allowed";
	statusMap[408] = "Request Timeout";
	statusMap[411] = "Length Required";
	statusMap[413] = "Payload Too Large";
	statusMap[414] = "URI Too Long";

	statusMap[500] = "Internal Server Error";
	statusMap[501] = "Not Implemented";
	statusMap[502] = "Bad Gateway";
	statusMap[503] = "Service Unavailable";
	statusMap[505] = "HTTP Version Not Supported";

	return statusMap;
}

std::string &getStatusCodeMap(int code) {
	static std::map<int, std::string> map = createStatusCodeMap();
	return map[code];
}

bool	isValidHttpVersion(const std::string &version) {
	if (version.empty() || version.substr(0, 5) != "HTTP/" || version.length() > 8)
		return false;
	return true;
}

bool isValidRequestPath(const std::string &path) {
	if (path.empty() || path[0] != '/')
		return false;

	for (size_t i = 0; i < path.length(); ++i) {
		char c = path[i];

		// Reject control characters and spaces
		if (c <= 0x1F || c == 0x7F || c == ' ')
			return false;

		// Optional: reject backslashes (Windows path)
		if (c == '\\')
			return false;
	}

	return true;
}

bool isKnownMethod(const std::string &method) {
	if (method.empty())
		return false;
	static const std::string known[] = {
		"GET", "POST", "DELETE", "PUT", "HEAD", "OPTIONS", "CONNECT", "TRACE", "PATCH"
	};
	for (size_t i = 0; i < sizeof(known)/sizeof(*known); ++i)
		if (method == known[i])
			return true;
	return false;
}

bool isNumber(const std::string &str) {
	if (str.empty())
		return false;
	for (size_t i = 0; i < str.length(); ++i) {
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}

bool isValidHeaderKey(const std::string &key) {
	if (key.empty())
		return false;
	for (size_t i = 0; i < key.length(); ++i) {
		char c = key[i];
		if (!isalnum(c) && std::string("!#$%&'*+-.^_`|~").find(c) == std::string::npos)
			return false;
	}
	return true;
}

bool isValidHeaderValue(const std::string &value) {
	if (value.empty())
		return false;
	for (size_t i = 0; i < value.length(); ++i) {
		char c = value[i];
		if (iscntrl(c) && c != '\t') // allow horizontal tab only
			return false;
	}
	return true;
}

std::string readFile(std::string const &path) {
	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open())
		return "";

	std::ostringstream ss;
	ss << file.rdbuf();
	file.close();

	return ss.str();
}

bool	endsWith(const std::string& str, const std::string& suffix) {
	if (str.length() < suffix.length()) return false;
	return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

void	terminate_server(int sig) {
	if (sig == SIGINT) {
		run = false;
		std::cout << "\nServer shutting down..." << std::endl;
	}
}

void	init(char **av) {
	(void)av;
	signal(SIGINT, terminate_server);
}

void	kickClient(std::unordered_map<int, Client> &clients, std::vector<struct pollfd> &fds, size_t *index) {
	int client_fd = fds[*index].fd;
	std::cout << "Kicking client with fd: " << client_fd << std::endl;
	close(client_fd);
	clients.erase(client_fd);
	fds.erase(fds.begin() + *index);
	*index = 0; // Reset index to avoid out of bounds
}
