/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostMethod.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/28 13:59:57 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/08/05 18:13:26 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"

bool	isUploadMeta(const std::string &content_type) {
	return content_type == "multipart/form-data" || content_type == "application/x-www-form-urlencoded";
}

void	Response::uploadRawFile() {
	std::string upload_dir = "www/uploads"; // Or get from config

	// Ensure the upload directory exists
	if (access(upload_dir.c_str(), F_OK) == -1) {
		if (mkdir(upload_dir.c_str(), 0755) == -1) {
			setStatus(500, "Internal Server Error");
			return;
		}
	}

	// Create a unique filename (e.g., using a timestamp)
	std::time_t result = std::time(nullptr);
	std::string filename = upload_dir + "/upload_" + std::to_string(result) + ".txt";

	// Create and write to the new file
	std::ofstream outfile(filename.c_str(), std::ios::binary);
	if (!outfile) {
		setStatus(500, "Internal Server Error");
		return;
	}

	outfile.write(request->body.c_str(), request->body.length());
	outfile.close();

	if (outfile.fail()) {
		setStatus(500, "Internal Server Error");
		return;
	}

	setStatus(201, "Created");
	headers["Location"] = filename.substr(filename.find("/"));
}

bool	urlDecoder(const std::string &str, std::string &result) {
	result.clear();
	for (size_t i = 0; i < str.length(); i++) {
		if (str[i] == '%') {
			if (i + 2 >= str.length()) return false;  // Incomplete %XX

			// Check if next 2 chars are valid hex
			if (!std::isxdigit(str[i+1]) || !std::isxdigit(str[i+2])) {
				return false;  // Invalid hex digits
			}

			int ascii_val = std::stoi(str.substr(i + 1, 2), nullptr, 16);
			result += static_cast<char>(ascii_val);
			i += 2;
		} else if (str[i] == '+') {
			result += ' ';
		} else {
			result += str[i];
		}
	}
	return true;
}

void	Response::saveFormDataToFile(const std::map<std::string, std::string> &form_data) {
	std::ostringstream outfile;

	for (const auto &pair : form_data) {
		outfile << pair.first << "=" << pair.second << "\n";
	}
	std::string data = outfile.str();
	this->saveDataToFile(data, "");
}

void	Response::uploadUrlEncoded() {
	std::map<std::string, std::string> form_data;
	std::string decoded_body;

	// URL decode first - if this fails, it's an error
	if (!urlDecoder(request->body, decoded_body)) {
		request->status = 400; // Bad Request - malformed URL encoding
		return ;
	}

	// Parse pairs
	size_t pos = 0;
	while (pos < decoded_body.length()) {
		size_t amp_pos = decoded_body.find('&', pos);
		if (amp_pos == std::string::npos) amp_pos = decoded_body.length();

		std::string pair = decoded_body.substr(pos, amp_pos - pos);

		// Skip empty pairs (from && or leading/trailing &)
		if (!pair.empty()) {
			size_t eq_pos = pair.find('=');
			if (eq_pos != std::string::npos) {
				std::string key = pair.substr(0, eq_pos);
				std::string value = pair.substr(eq_pos + 1);
				if (!key.empty()) {  // Don't store empty keys
					form_data[key] = value;
				}
			} else {
				// Key without value
				form_data[pair] = "";
			}
		}
		pos = amp_pos + 1;
	}

	// Empty form_data is OK - just save it
	saveFormDataToFile(form_data);  // This handles empty maps gracefully
	setStatus(201, "Created");
}

void	Response::saveDataToFile(const std::string &data, std::string filename) {
	std::string upload_dir = "www/uploads"; // Or get from config

	// Ensure the upload directory exists
	if (access(upload_dir.c_str(), F_OK) == -1) {
		if (mkdir(upload_dir.c_str(), 0755) == -1) {
			setStatus(500, "Internal Server Error");
			return;
		}
	}

	// Create a unique filename (e.g., using a timestamp)
	if (filename.empty()) {
		std::time_t result = std::time(nullptr);
		filename = upload_dir + "/upload_" + std::to_string(result) + ".txt";
	}
	// Create and write to the new file
	std::ofstream outfile(filename.c_str(), std::ios::binary);
	if (!outfile) {
		setStatus(500, "Internal Server Error");
		return;
	}

	outfile.write(data.c_str(), data.length());
	outfile.close();

	if (outfile.fail()) {
		setStatus(500, "Internal Server Error");
		return;
	}

	setStatus(201, "Created");
	headers["Location"] = filename.substr(filename.find("/"));
}

void	Response::saveMultiFormDataToFile(std::string data) {
	size_t headers_pos = data.find("\r\n\r\n", 0);
	std::string filename = "";
	std::string content;
	if (headers_pos == std::string::npos) {
		std::cerr << "Invalid multipart data format." << std::endl;
		return;
	}
	std::string headers = data.substr(0, headers_pos);
	size_t pos = headers.find("filename=");
	if (pos != std::string::npos) {
		pos += 10;
		size_t end_pos = headers.find('"', pos);
		filename = headers.substr(pos, end_pos - pos);
	}
	content = data.substr(headers_pos + 4); // Skip the \r\n after headers
	pos = content.find("\r\n");
	data = content.substr(0, pos); // Get the content before the next boundary
	if (data.empty() && filename.empty()) {
		return;
	}
	if (filename.empty()) {
		filename = "upload_" + std::to_string(std::time(nullptr)) + ".txt"; // Default filename
	}
	saveDataToFile(content, filename);
}

void	Response::uploadMultiForm(std::string &content_type) {
	std::string	boundary;
	std::string last_boundary;
	std::string body = request->body;
	size_t pos = content_type.find("boundary=");

	if (pos == std::string::npos) {
		request->status = 400; // Bad Request - no boundary found
		return;
	}
	boundary = "--" + content_type.substr(pos + 9); // 9 is length of "boundary="
	if (boundary.empty()) {
		request->status = 400; // Bad Request - empty boundary
		return;
	}
	last_boundary = boundary + "--";
	pos = body.find(boundary, 0);
	if (pos == std::string::npos) {
		request->status = 400; // Bad Request - no boundary found
		return;
	}
	while (body.substr(pos, last_boundary.length()) != last_boundary) {
		pos += boundary.length(); // Move past the boundary
		if (body.substr(pos, 2) == "\r\n") {
			pos += 2; // Skip the CRLF after the boundary
		} else {
			request->status = 400; // Bad Request - expected CRLF after boundary
			return;
		}
		size_t end_pos = body.find(boundary, pos);
		if (end_pos == std::string::npos) {
			request->status = 400; // Bad Request - no closing boundary found
			return;
		}
		saveMultiFormDataToFile(body.substr(pos, end_pos - pos));
		pos = end_pos;
	}
	if (status_code == 200)
		setStatus(201, "Created");
}

void	Response::executePostBody() {
	std::string content_type = request->headers["Content-Type"];
	if (content_type == "application/x-www-form-urlencoded")
		uploadUrlEncoded();
	else if (content_type.rfind("multipart/form-data", 0) == 0)
		uploadMultiForm(content_type);
	else
		uploadRawFile();
}
