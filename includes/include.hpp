/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   include.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 16:18:09 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/06/20 05:06:19 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <new>
#include <exception>
#include <map>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
#include <cerrno>
#include <cctype>
#include "Client.hpp"
#include "Server.hpp"

extern bool run;
extern bool autoIndex;

#define PORT 8080
#define MAX_BODY_SIZE 10485760 // 10MB
#define MAX_CHUNK_SIZE 131072 // 128KB

enum PathStatus {
	PATH_IS_FILE,
	PATH_IS_DIRECTORY,
	PATH_NOT_FOUND,
	PATH_PERMISSION_DENIED,
	PATH_STAT_ERROR
};

std::string	readFile(std::string const &path);
void	terminate_server(int sig);
void	init(char **av);
void	kickClient(std::unordered_map<int, Client> &clients, std::vector<struct pollfd> &fds, size_t *index);
bool	endsWith(const std::string& str, const std::string& suffix);
bool	isValidHeaderValue(const std::string &value);
bool	isValidHeaderKey(const std::string &value);
bool	isNumber(const std::string &str);
bool	isKnownMethod(const std::string &method);
bool	isValidRequestPath(const std::string &path);
bool	isValidHttpVersion(const std::string &version);
bool	isDirectory(const std::string &path);
bool	fileReadable(const std::string &directoryPath);
bool	fileExists(const std::string& path);
bool	iequals(const std::string& a, const std::string& b);
std::string generateAutoindexPage(const std::string &directoryPath, const std::string &uri);
std::string &getStatusCodeMap(int code);
PathStatus checkPath(const std::string& path_str);
