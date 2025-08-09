/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseCGI.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/09 13:08:26 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/08/09 13:09:41 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"

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
		std::string env_cookies = "HTTP_COOKIE=" + (request->headers.count("Cookie") ? request->headers.at("Cookie") : "");
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
			const_cast<char*>(env_cookies.c_str()),
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
