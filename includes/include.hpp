/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   include.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 16:18:09 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/08/09 13:10:14 by mel-bouh         ###   ########.fr       */
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
#include <ctime>
#include <iomanip>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include "Client.hpp"
#include "Server.hpp"

extern bool run;
extern bool autoIndex;
extern std::vector<struct pollfd> fds;


#define GREEN "\033[1;32m"
#define RED     "\033[1;31m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define CYAN    "\033[1;36m"
#define WHITE   "\033[1;37m"
#define RESET   "\033[0m"

#define PORT 8080
#define MAX_BODY_SIZE 10000000 // 10MB
#define MAX_CHUNK_SIZE 100000 // 100KB
#define IDLE_TIMEOUT 10
#define HEADERS_TIMEOUT 5

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
void	kickClient(std::map<int, Client> &clients, std::vector<struct pollfd> &fds, size_t *index, bool *client);
bool	endsWith(const std::string& str, const std::string& suffix);
bool	isValidHeaderValue(const std::string &value);
bool	isValidHeaderKey(const std::string &value);
bool	isNumber(const std::string &str);
bool	isKnownMethod(const std::string &method);
bool	isValidRequestPath(const std::string &path);
bool	isValidHttpVersion(const std::string &version);
bool	isValidContentType(const std::string &contentType);
bool	isDirectory(const std::string &path);
bool	fileReadable(const std::string &directoryPath);
bool	fileExists(const std::string& path);
bool	iequals(const std::string& a, const std::string& b);
bool	checkIdle(Client& client);
bool	isTextFile(const std::string& filename);
std::string readError(std::string const &path, int code);
std::string generateAutoindexPage(const std::string &directoryPath, const std::string &uri);
std::string &getStatusCodeMap(int code);
std::string generateDefaultFilename();
std::string removeTrailingCRLF(const std::string& content);
std::string processTextContent(const std::string& content);
std::string extractFilename(const std::string& headers);
std::string toLower(const std::string& str);
PathStatus checkPath(const std::string& path_str);
