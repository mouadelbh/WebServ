/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   include.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 16:18:09 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/24 13:57:07 by mel-bouh         ###   ########.fr       */
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
#include "Client.hpp"
#include "Server.hpp"

extern bool run;

#define PORT 8082

void	getRequest(std::unordered_map<int, Client> &clients, std::vector<struct pollfd> &fds, size_t *index);
void	sendResponse(std::unordered_map<int, Client> &clients, std::vector<struct pollfd> &fds, size_t *index);
