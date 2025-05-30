/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils2.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 10:19:03 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/30 10:19:12 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/include.hpp"

PathStatus checkPath(const std::string& path_str) {
	struct stat statbuf;
	if (stat(path_str.c_str(), &statbuf) != 0) {
		if (errno == ENOENT) {
			return PATH_NOT_FOUND;
		} else if (errno == EACCES) {
			return PATH_PERMISSION_DENIED;
		} else {
			return PATH_STAT_ERROR; // Could log strerror(errno) here for details
		}
	}

	if (S_ISDIR(statbuf.st_mode)) {
		return PATH_IS_DIRECTORY;
	}
	if (S_ISREG(statbuf.st_mode)) {
		return PATH_IS_FILE;
	}

	// It's something else (symlink to non-existent, socket, pipe, etc.)
	// For a web server, often treat these as not found or forbidden.
	return PATH_NOT_FOUND; // Or a new status like PATH_NOT_SUPPORTED_TYPE
}

