/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 16:07:51 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/28 10:28:43 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"

Request::Request() {}

Request::~Request() {
	clear();
}

void	Request::parse(const std::string &str) {
	std::istringstream stream(str);
	stream >> method >> path >> version;
	std::cout << "Method: " << method << " Path: " << path << std::endl;
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
