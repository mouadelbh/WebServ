/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 16:28:05 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/08/09 11:32:45 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"
#include <sys/wait.h>

Response::Response() : version("HTTP/1.1") ,status_code(200) ,request(NULL)
{
	this->CGI_active = false;
}

Response::~Response() {
	clear();
}

void Response::debugResponse() const {
    std::string response_str = toString();
    std::cout << "=== RESPONSE DEBUG ===" << std::endl;
    std::cout << "Response length: " << response_str.length() << " bytes" << std::endl;

    // Find the Content-Length header
    size_t header_end = response_str.find("\r\n\r\n");
    if (header_end != std::string::npos) {
        std::string headers_only = response_str.substr(0, header_end);
        std::string body_only = response_str.substr(header_end + 4);

        std::cout << "Headers section length: " << headers_only.length() << std::endl;
        std::cout << "Body section length: " << body_only.length() << std::endl;

        // Extract Content-Length value
        size_t cl_pos = headers_only.find("Content-Length:");
        if (cl_pos != std::string::npos) {
            size_t cl_start = headers_only.find(":", cl_pos) + 1;
            size_t cl_end = headers_only.find("\r\n", cl_pos);
            std::string cl_value = headers_only.substr(cl_start, cl_end - cl_start);

            // Trim whitespace
            size_t first = cl_value.find_first_not_of(" \t");
            if (first != std::string::npos) {
                cl_value = cl_value.substr(first);
            }

            int declared_length = std::stoi(cl_value);
            std::cout << "Declared Content-Length: " << declared_length << std::endl;
            std::cout << "Actual body length: " << body_only.length() << std::endl;

            if (declared_length != (int)body_only.length()) {
                std::cout << "❌ CONTENT-LENGTH MISMATCH!" << std::endl;
                std::cout << "Difference: " << (declared_length - (int)body_only.length()) << " bytes" << std::endl;
            } else {
                std::cout << "✅ Content-Length matches body size" << std::endl;
            }
        }
    }

    // Show the actual response (first 200 chars)
    std::cout << "Response preview:" << std::endl;
    std::cout << response_str.substr(0, 200) << "..." << std::endl;
    std::cout << "=== END DEBUG ===" << std::endl;
}

void Response::setStatus(int code, const std::string &message) {
	status_code = code;
	status_message = message;
}

std::string Response::toString() const
{
    std::ostringstream oss;

    // Status line with CRLF
    oss << version << " " << status_code << " " << status_message << "\r\n";

    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        oss << it->first << ": " << it->second << "\r\n";
    }

    oss << "\r\n";
    oss << body;

    return oss.str();
}

bool	Response::autoIndexStatus() {
	if (locationConfig && locationConfig->autoindex != -1) {
		return locationConfig->autoindex;
	}
	else if (config && config->autoindex != -1) {
		return config->autoindex;
	}
	else {
		return autoIndex;
	}
}

void	Response::buildStatusLine() {
	version = "HTTP/1.0";
	if (request->status != 0)
		setStatus(request->status, getStatusCodeMap(request->status));
}

void	Response::setGetPath() {
	std::string &path = request->path;

	if (isDirectory(request->path) && !endsWith(request->path, "/"))
		path += "/";
	if (isDirectory(request->path) && fileReadable(request->path + (config->index.empty() ? "index.html" : config->index)))
		path += (config->index.empty() ? "index.html" : config->index);
	if (isDirectory(request->path) && !fileReadable(request->path + (config->index.empty() ? "index.html" : config->index)) && !autoIndexStatus()) {
		setStatus(403, getStatusCodeMap(403));
		return;
	}
	if (!isDirectory(request->path) && !fileExists(request->path)) {
		if (errno == ENOENT || errno == ENOTDIR) {
			std::cout << "File not found: " << request->path << std::endl;
			setStatus(404, getStatusCodeMap(404));
		}
		else if (errno == EACCES)
			setStatus(403, getStatusCodeMap(403));
		else
			setStatus(500, getStatusCodeMap(500));
	}
}

std::string	Response::getErrorPage(int code) {
	std::string error_page = "error_pages/";
	if (config->error_pages.find(code) != config->error_pages.end())
		return config->error_pages[code];
	error_page += std::to_string(code);
	error_page += ".html";
	return error_page;
}

void	Response::createBody() {
	if (status_code == 204) {
		body.clear();
		return;
	}
	else if (status_code == 201)
		body = readError("www/upload.html", 201);
	else if (status_code == 200)
		body = readFile(request->path);
	else
		body = readError(getErrorPage(status_code), status_code);
}

void	Response::buildGetBody() {
	if (status_code == 200 || status_code == 301)
		this->setGetPath();
	if ((status_code == 200 || status_code == 301) && autoIndexStatus() && isDirectory(request->path)) {
		body = generateAutoindexPage(request->path, uri);
	}
	else
		this->createBody();
}

void Response::buildHeaders() {
    // FIRST: Make sure body is finalized BEFORE calculating Content-Length
    if (status_code != 204) {
        if ((status_code == 200 || status_code == 301) && !CGI_active)
            headers["Content-Type"] = request->getType();
        else
            headers["Content-Type"] = "text/html";
    }

    // Calculate Content-Length AFTER body is complete and finalized
    if (status_code != 204) {
        // CRITICAL: Use body.size(), not body.length()
        // In case of binary data, these might differ
        headers["Content-Length"] = std::to_string(body.size());
    }

    headers["Connection"] = "close";
    headers["Server"] = config->server_name.empty() ? "Webserv" : config->server_name;
}

// void	Response::buildHeaders() {
// 	if (status_code != 204) {
// 		headers["Content-Length"] = std::to_string(body.size());
// 		if (status_code == 200)
// 			headers["Content-Type"] = request->getType();
// 		else
// 			headers["Content-Type"] = "text/html";
// 	}

// 	headers["Connection"] = "close";
// 	headers["Server"] = "Webserv";
// }

void	Response::executeDeleteBody() {
	struct stat fileStat;

	if (stat(request->path.c_str(), &fileStat) == -1) {
		if (errno == ENOENT) {
			setStatus(404, getStatusCodeMap(404));
		}
		else if (errno == EACCES)
			setStatus(403, getStatusCodeMap(403));
		else
			setStatus(500, getStatusCodeMap(500));
		return;
	}
	if (!S_ISREG(fileStat.st_mode)) {
		setStatus(403, getStatusCodeMap(403));
		return;
	}
	if (std::remove(request->path.c_str()) != 0) {
		if (errno == EACCES)
			setStatus(403, getStatusCodeMap(403));
		else
			setStatus(500, getStatusCodeMap(500));
		return;
	} else {
		setStatus(204, getStatusCodeMap(204));
		body.clear();
	}
}

bool	validStatusCode(int code) {
	if (code >= 400)
		return false;
	return true;
}

void	Response::build() {
	clear();
	uri = request->path;
	if (config->root[0] == '/')
		config->root = config->root.substr(1);
	request->path = config->root + request->path;
	if (request->path[0] == '/')
		request->path = request->path.substr(1);
	std::cout << GREEN << "Request path: " << request->path << RESET << std::endl;
	this->buildStatusLine();

	if (isCGIRequest(request->path) && (request->method == "GET" || request->method == "POST")) {
		this->executeCGI();
		if (status_code != 200)
			this->createBody();
	}
	else if (request->method == "GET" || !validStatusCode(status_code)) {
		this->buildGetBody();
	}
	else if (request->method == "POST") {
		this->executePostBody();
		this->createBody();
	}
	else if (request->method == "DELETE") {
		this->executeDeleteBody();
		this->createBody();
	}
	else {
		this->setStatus(501, getStatusCodeMap(501));
		this->createBody();
	}
	this->buildHeaders();
}


bool Response::isCGIRequest(const std::string &path) {
	std::string file_path = path;
	size_t query_pos = file_path.find('?');
	if (query_pos != std::string::npos) {
		file_path = file_path.substr(0, query_pos);
	}

	// Check if the file has a CGI extension
	if (file_path.find(".php") != std::string::npos ||
		file_path.find(".py") != std::string::npos ||
		file_path.find(".pl") != std::string::npos) {
			return true;
		}
	return false;
}

void Response::executeCGI() {

	std::cout << BLUE << "Executing CGI for path: " << request->path << RESET << std::endl;
	std::string script_path = request->path;
	size_t query_pos = script_path.find('?');
	if (query_pos != std::string::npos) {
		script_path = script_path.substr(0, query_pos);
	}

	if (!isDirectory(request->path) && !fileExists(request->path)) {
		if (errno == ENOENT || errno == ENOTDIR) {
			std::cout << "File not found: " << request->path << std::endl;
			setStatus(404, getStatusCodeMap(404));
			return;
		}
		else if (errno == EACCES)
		{
			setStatus(403, getStatusCodeMap(403));
			return;
		}
		else
		{
			setStatus(500, getStatusCodeMap(500));
			return;
		}
	}
	// Determine the CGI interpreter based on file extension
	std::string interpreter;
	if (script_path.find(".php") != std::string::npos) {
		interpreter = "/usr/bin/php";
	} else if (script_path.find(".py") != std::string::npos) {
		std::cout << "Using Python interpreter for CGI script." << std::endl;
		interpreter = "/usr/bin/python3";
	} else {
		setStatus(500, "Internal Server Error");
		return;
	}

	// Create pipes for communication
	int pipe_in[2], pipe_out[2];
	if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
		setStatus(500, "Internal Server Error");
		return;
	}

	pid_t pid = fork();
	if (pid == -1) {
		close(pipe_in[0]); close(pipe_in[1]);
		close(pipe_out[0]); close(pipe_out[1]);
		setStatus(500, "Internal Server Error");
		return;
	}

	if (pid == 0) {
		close(pipe_in[1]);
		close(pipe_out[0]);
		dup2(pipe_in[0], STDIN_FILENO);
		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_in[0]);
		close(pipe_out[1]);

		std::string query_string = "";
		if (request->method == "GET") {
			size_t query_pos = uri.find('?');
			if (query_pos != std::string::npos)
				query_string = uri.substr(query_pos + 1);
		} else if (request->method == "POST") {
			query_string = request->body;
		}


		char *args[] = {
			const_cast<char*>(interpreter.c_str()),
			const_cast<char*>(script_path.c_str()),
			NULL
		};
		std::string env_request_method = "REQUEST_METHOD=" + request->method;
		std::string env_query_string = "QUERY_STRING=" + query_string;
		std::string env_content_type = "CONTENT_TYPE=" + (request->headers.count("Content-Type") ? request->headers.at("Content-Type") : "");
		std::string env_content_length = "CONTENT_LENGTH=" + std::to_string(request->body.length());
		std::string env_script_name = "SCRIPT_NAME=" + uri;
		std::string env_path_info = "PATH_INFO=";
		std::string env_server_software = "SERVER_SOFTWARE=WebServ/1.0";
		std::string env_server_protocol = "SERVER_PROTOCOL=HTTP/1.1";
		std::string env_gateway_interface = "GATEWAY_INTERFACE=CGI/1.1";

		std::cout << "QUERY_STRING: " << env_query_string << std::endl;

		char *envp[] = {
			const_cast<char*>(env_request_method.c_str()),
			const_cast<char*>(env_query_string.c_str()),
			const_cast<char*>(env_content_type.c_str()),
			const_cast<char*>(env_content_length.c_str()),
			const_cast<char*>(env_script_name.c_str()),
			const_cast<char*>(env_path_info.c_str()),
			const_cast<char*>(env_server_software.c_str()),
			const_cast<char*>(env_server_protocol.c_str()),
			const_cast<char*>(env_gateway_interface.c_str()),
			NULL
		};
		execve(interpreter.c_str(), args, envp);
		exit(1);
	}
	else
	{
		close(pipe_in[0]);
		close(pipe_out[1]);

		if (request->method == "POST" && !request->body.empty()) {
			write(pipe_in[1], request->body.c_str(), request->body.length());
		}
		close(pipe_in[1]);
		char buffer[4096];
		std::string cgi_output;
		ssize_t bytes_read;
		while ((bytes_read = read(pipe_out[0], buffer, sizeof(buffer))) > 0) {
			cgi_output.append(buffer, bytes_read);
		}
		close(pipe_out[0]);
		int status1;
		waitpid(pid, &status1, 0);
		// std::cout << RED << "CGI_output : " << cgi_output << RESET << std::endl;
		if (WEXITSTATUS(status1) != 0) {
			setStatus(500, getStatusCodeMap(500));
			return;
		}
		CGI_active = true;
		headers["Content-Type"] = "text/html";
		headers["Content-Length"] = std::to_string(cgi_output.length());
		size_t header_end = cgi_output.find("\r\n\r\n");
		if (header_end == std::string::npos) {
			header_end = cgi_output.find("\n\n");
			if (header_end != std::string::npos) {
				header_end += 2;
			}
		} else {
			header_end += 4;
		}

		if (header_end != std::string::npos) {
			// Parse headers
			std::string cgi_headers = cgi_output.substr(0, header_end - 2);
			body = cgi_output.substr(header_end);

			// Parse individual headers
			std::istringstream header_stream(cgi_headers);
			std::string line;
			while (std::getline(header_stream, line)) {
				if (!line.empty() && line.back() == '\r') {
					line.pop_back();
				}
				size_t colon_pos = line.find(':');
				if (colon_pos != std::string::npos) {
					std::string key = line.substr(0, colon_pos);
					std::string value = line.substr(colon_pos + 1);
					while (!value.empty() && (value[0] == ' ' || value[0] == '\t')) {
						value.erase(0, 1);
					}
					headers[key] = value;
				}
			}
		} else {
			// No headers separator found, treat everything as body
			body = cgi_output;
			headers["Content-Type"] = "text/html";
		}

		setStatus(200, "OK");
	}
}

void	Response::clear() {
	version.clear();
	status_code = 200;
	status_message.clear();
	headers.clear();
	body.clear();
}
