/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils2.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 10:19:03 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/08/09 20:20:16 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/include.hpp"

bool checkIdle(Client& client) {
	time_t current_time = time(NULL);
	ParseState &state = client.request.parse_state;
	if ((state == REQUEST_LINE || state == END) && current_time - client.last_activity > IDLE_TIMEOUT) {
		return false;
	}
	if (state == HEADERS && current_time - client.last_activity > HEADERS_TIMEOUT) {
		return false;
	}
	if ((state == BODY || state == INFO) && current_time - client.last_activity > IDLE_TIMEOUT) {
		return false;
	}
	return true;
}

bool iequals(const std::string& a, const std::string& b) {
	if (a.size() != b.size()) return false;
	for (size_t i = 0; i < a.size(); ++i) {
		if (std::tolower((unsigned char)a[i]) != std::tolower((unsigned char)b[i]))
			return false;
	}
	return true;
}

PathStatus checkPath(const std::string& path_str) {
	struct stat statbuf;
	if (stat(path_str.c_str(), &statbuf) != 0) {
		if (errno == ENOENT) {
			return PATH_NOT_FOUND;
		} else if (errno == EACCES) {
			return PATH_PERMISSION_DENIED;
		} else {
			return PATH_STAT_ERROR;
		}
	}

	if (S_ISDIR(statbuf.st_mode)) {
		return PATH_IS_DIRECTORY;
	}
	if (S_ISREG(statbuf.st_mode)) {
		return PATH_IS_FILE;
	}


	return PATH_NOT_FOUND;
}

