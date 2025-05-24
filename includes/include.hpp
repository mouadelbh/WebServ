/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   include.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 16:18:09 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/24 17:09:09 by mel-bouh         ###   ########.fr       */
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
#include "Client.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"

extern bool run;

#define PORT 8080

std::string	&readFile(std::string const &path);
void	terminate_server(int sig);
void	init(char **av);
void	kickClient(std::unordered_map<int, Client> &clients, std::vector<struct pollfd> &fds, size_t *index);
