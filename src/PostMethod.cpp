/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostMethod.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/28 13:59:57 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/08/09 18:12:57 by mel-bouh         ###   ########.fr       */
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
			setStatus(500, getStatusCodeMap(500));
			return;
		}
	}

	// Create a unique filename (e.g., using a timestamp)
	std::time_t result = std::time(NULL);
	std::string filename = upload_dir + "/upload_" + to_string_c98(result) + ".txt";

	// Create and write to the new file
	std::ofstream outfile(filename.c_str(), std::ios::binary);
	if (!outfile) {
		setStatus(500, getStatusCodeMap(500));
		return;
	}

	outfile.write(request->body.c_str(), request->body.length());
	outfile.close();

	if (outfile.fail()) {
		setStatus(500, getStatusCodeMap(500));
		return;
	}

	setStatus(201, getStatusCodeMap(201));
	headers["Location"] = upload_dir;
}

static int hexToInt(char high, char low) {
	int value = 0;

	// Convert high nibble
	if (high >= '0' && high <= '9')
		value = (high - '0') << 4;
	else if (high >= 'A' && high <= 'F')
		value = (high - 'A' + 10) << 4;
	else if (high >= 'a' && high <= 'f')
		value = (high - 'a' + 10) << 4;

	// Convert low nibble
	if (low >= '0' && low <= '9')
		value |= (low - '0');
	else if (low >= 'A' && low <= 'F')
		value |= (low - 'A' + 10);
	else if (low >= 'a' && low <= 'f')
		value |= (low - 'a' + 10);

	return value;
}

bool urlDecoder(const std::string &str, std::string &result) {
	result.clear();
	for (size_t i = 0; i < str.length(); i++) {
		if (str[i] == '%') {
			if (i + 2 >= str.length()) return false;  // Incomplete %XX

			if (!std::isxdigit(static_cast<unsigned char>(str[i+1])) ||
				!std::isxdigit(static_cast<unsigned char>(str[i+2]))) {
				return false;  // Invalid hex digits
			}

			int ascii_val = hexToInt(str[i+1], str[i+2]);
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

	for (std::map<std::string, std::string>::const_iterator it = form_data.begin(); it != form_data.end(); ++it) {
		outfile << it->first << "=" << it->second << "\n";
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
	setStatus(201, getStatusCodeMap(201));
}

void	Response::executePostBody() {
	std::string content_type = request->headers["Content-Type"];
	if (fileExists(request->path)) {
		setStatus(409, getStatusCodeMap(409));
		return;
	}
	if (!isDirectory(request->path)) {
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
	if (content_type == "application/x-www-form-urlencoded")
		uploadUrlEncoded();
	else if (content_type.rfind("multipart/form-data", 0) == 0)
		uploadMultiForm(content_type);
	else
		uploadRawFile();
}
