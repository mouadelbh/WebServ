/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 14:33:47 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/08/08 14:50:45 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/include.hpp"

bool fileExists(const std::string& path) {
    std::cout << "path : " << path << std::endl;
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf) != 0)
		return false;
	if (!fileReadable(path))
		return false; // Check if the file is readable
	return S_ISREG(statbuf.st_mode);
}

std::string generateAutoindexPage(const std::string &directoryPath, const std::string &uri) {
    std::ostringstream html;

    // Start HTML document and head
    html << "<!DOCTYPE html>\n";
    html << "<html lang=\"en\">\n<head>\n";
    html << "    <meta charset=\"UTF-8\">\n";
    html << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    html << "    <title>Index of " << uri << "</title>\n";
    // Link to the external stylesheet
    html << "    <link rel=\"stylesheet\" type=\"text/css\" href=\"/autoindex_style.css\">\n";
    html << "</head>\n<body>\n";

    // Main content container
    html << "    <div class=\"container\">\n";
    html << "        <h1>Index of " << uri << "</h1>\n";
    html << "        <hr>\n";
    html << "        <ul>\n";

    // Attempt to open the directory
    DIR *dir = opendir(directoryPath.c_str());
    if (!dir) {
        // If opendir fails (e.g., permissions), return a simple error message.
        // The caller (Response::buildBody) should ideally set a 403 status.
        // This string is a fallback if the caller doesn't handle it.
        html << "            <li>Could not open directory. Check permissions.</li>\n";
        html << "        </ul>\n";
        html << "        <div class=\"footer\"><p>Webserv Autoindex</p></div>\n"; // Simple footer
        html << "    </div>\n</body>\n</html>";
        // It's better to return a distinct error indicator or let the caller handle the HTTP status.
        // For now, returning the "403 Forbidden" string as per your original code if opendir fails.
        // This will be caught by buildBody to set the actual 403 status.
        // closedir(dir); // Should not be called if dir is NULL. This was a bug in original.
                        // It's safer to check dir before closing.
                        // This part of the code is problematic if !dir, as html stream is built for normal page
                        // and then a raw H1 is returned. Let's assume for now that if opendir fails,
                        // the server will handle sending the 403 status and the "<h1>403 Forbidden</h1>"
                        // string might be used as the body. The CSS will attempt to style this raw H1.
        return "<h1>403 Forbidden</h1>"; // Fallback from your original code
    }

    // Add "Parent Directory" link if not at the root
    if (uri != "/" && !uri.empty()) {
        std::string parentUri = uri;
        // Remove trailing slash if present (except if it's just "/")
        if (parentUri.length() > 1 && parentUri.back() == '/') {
            parentUri.pop_back();
        }
        // Find the last slash to go up one level
        size_t lastSlash = parentUri.find_last_of('/');
        if (lastSlash != std::string::npos) {
            parentUri = parentUri.substr(0, lastSlash + 1); // Keep the slash for the parent
            if (parentUri.empty() || parentUri[0] != '/') { // Ensure it's a valid path
                    parentUri = "/"; // Default to root if manipulation fails
            }
        } else {
            parentUri = "/"; // If no other slash, parent is root
        }
        // Original logic: if (uri == "/") parentUri = ""; // Don't show parent for root itself
        // This condition is covered by the initial `if (uri != "/")` check for adding the parent link.
        // So if uri is "/", this block is skipped.

        if (!parentUri.empty()) { //This check might be redundant if parentUri is always set to "/" or a path
            html << "            <li><a href=\"" << parentUri << "\" class=\"parent-dir\">.. (Parent Directory)</a></li>\n";
        }
    }


    std::vector<std::pair<std::string, bool>> entries; // Pair: <name, is_directory>
    struct dirent *entry;

    // Read all entries from the directory
    while ((entry = readdir(dir)) != NULL) {
        std::string name(entry->d_name);
        if (name == "." || name == "..") { // Skip current and parent directory entries here
            continue;
        }

        std::string fullPath = directoryPath;
        if (!fullPath.empty() && fullPath.back() != '/') {
            fullPath += "/";
        }
        fullPath += name;

        struct stat s;
        bool is_dir = false;
        if (stat(fullPath.c_str(), &s) == 0) {
            is_dir = S_ISDIR(s.st_mode);
        }
        entries.push_back(std::make_pair(name, is_dir));
    }
    closedir(dir); // Close the directory stream

    // Sort entries: directories first, then files, both alphabetically
    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        if (a.second != b.second) { // If one is a dir and other is a file
            return a.second > b.second; // Directories (true) come before files (false)
        }
        return a.first < b.first; // Otherwise, sort alphabetically
    });

    // Generate list items for each entry
    for (const auto& p : entries) {
        const std::string& name = p.first;
        bool is_dir = p.second;

        html << "            <li><a href=\"";
        // Ensure the base URI for links ends with a slash if it's a directory path
        std::string baseUri = uri;
        if (!baseUri.empty() && baseUri.back() != '/') {
             baseUri += "/";
        } else if (baseUri.empty()){ // if uri is empty (e.g. root of a specific autoindex config)
            baseUri = "/"; // assume it's root if empty
        }
        // Handle case where uri is "/" - don't add extra slash if name starts with one (should not happen with readdir)
        // or if uri is already just "/"
        if (uri == "/") {
             baseUri = "/"; // Ensure it's just one slash before the name
        }


        html << baseUri << name;
        if (is_dir) {
            html << "/\" class=\"directory\">" << name << "/";
        } else {
            html << "\" class=\"file\">" << name;
        }
        html << "</a></li>\n";
    }

    html << "        </ul>\n";
    html << "        <hr>\n"; // Second hr, before footer
    html << "        <div class=\"footer\"><p>Webserv Autoindex</p></div>\n";
    html << "    </div>\n</body>\n</html>";

    return html.str();
}


bool fileReadable(const std::string &directoryPath) {
	std::string indexPath = directoryPath;

	int fd = open(indexPath.c_str(), O_RDONLY);
	if (fd == -1)
		return false;
	close(fd);
	return true;
}

bool isDirectory(const std::string &path) {
	struct stat pathStat;
	if (stat(path.c_str(), &pathStat) != 0)
		return false; // file doesn't exist or not accessible
	return S_ISDIR(pathStat.st_mode); // true if it's a directory
}

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

	if (path.find("//") != std::string::npos || path.find("/./") != std::string::npos)
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

bool isValidContentType(const std::string &contentType) {
    if (contentType.empty()) {
        return false;
    }
    // List of content types that are checked as prefixes
    static const std::string valid_types[] = {
        "application/x-www-form-urlencoded",
        "multipart/form-data",
        "application/json",
        "text/plain",
        "text/html",
        "text/css",          // Added
        "image/jpeg",        // Optional: for raw image uploads
        "image/png",         // Optional: for raw image uploads
        "image/gif",         // Optional: for raw image uploads
        "application/javascript", // Optional: for raw JS uploads
        "application/xml",
        "" // Sentinel value
    };

    for (int i = 0; !valid_types[i].empty(); ++i) {
        // Use rfind to check if the contentType starts with a valid type
        if (contentType.rfind(valid_types[i], 0) == 0) {
            return true;
        }
    }

    return false;
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

std::string readError(std::string const &path, int code) {
	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open())
		return "<h1>" + std::to_string(code) + " " + getStatusCodeMap(code) + "</h1>";

	std::ostringstream ss;
	ss << file.rdbuf();
	file.close();

	return ss.str();
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

void	kickClient(std::map<int, Client> &clients, std::vector<struct pollfd> &fds, size_t *index, bool *client) {
    int client_fd = fds[*index].fd;
	std::cout << RED << "Kicking client with fd: " << client_fd << RESET << std::endl;
	close(client_fd);
	clients.erase(client_fd);
	fds.erase(fds.begin() + *index);
	*client = true;
}
