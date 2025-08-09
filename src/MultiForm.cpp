/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MultiForm.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 09:49:35 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/08/09 20:14:56 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"

void	Response::saveDataToFile(const std::string &data, std::string filename) {
	std::string upload_dir = request->path;
	if (access(upload_dir.c_str(), F_OK) == -1) {
		if (mkdir(upload_dir.c_str(), 0755) == -1) {
			setStatus(500, getStatusCodeMap(500));
			return;
		}
	}
	if (filename.empty()) {
		std::time_t result = std::time(NULL);
		filename = upload_dir + "/upload_" + to_string_c98(result) + ".txt";
	}
	else
		filename = upload_dir + "/" + filename;
	std::ofstream outfile(filename.c_str(), std::ios::binary);
	if (!outfile) {
		setStatus(500, getStatusCodeMap(500));
		return;
	}
	outfile.write(data.c_str(), data.length());
	outfile.close();

	if (outfile.fail()) {
		setStatus(500, getStatusCodeMap(500));
		return;
	}
	setStatus(201, getStatusCodeMap(201));
	headers["Location"] = upload_dir;
}

void Response::saveMultiFormDataToFile(std::string data) {
	size_t headers_pos = data.find("\r\n\r\n", 0);
	if (headers_pos == std::string::npos) {
		std::cerr << "Invalid multipart data format." << std::endl;
		return;
	}

	std::string headers = data.substr(0, headers_pos);
	std::string filename = extractFilename(headers);
	std::string content = removeTrailingCRLF(data.substr(headers_pos + 4));

	if (content.empty() && filename.empty()) {
		return;
	}

	if (filename.empty()) {
		filename = generateDefaultFilename();
	}

	std::string final_content;
	if (isTextFile(filename)) {
		final_content = processTextContent(content);
	} else {
		final_content = content;
	}

	saveDataToFile(final_content, filename);
}

void Response::uploadMultiForm(std::string &content_type) {
	std::string boundary;
	std::string last_boundary;
	std::string body = request->body;
	size_t pos = content_type.find("boundary=");

	if (pos == std::string::npos) {
		request->status = 400;
		return;
	}

	std::string boundary_value = content_type.substr(pos + 9);
	size_t boundary_end = boundary_value.find_first_of("; \t\r\n");
	if (boundary_end != std::string::npos) {
		boundary_value = boundary_value.substr(0, boundary_end);
	}

	boundary = "--" + boundary_value;
	last_boundary = boundary + "--";

	pos = body.find(boundary);
	if (pos == std::string::npos) {
		request->status = 400;
		return;
	}

	while (pos != std::string::npos) {

		if (body.substr(pos, last_boundary.length()) == last_boundary) {
			break;
		}

		pos += boundary.length();
		if (pos + 2 <= body.length() && body.substr(pos, 2) == "\r\n") {
			pos += 2;
		}

		size_t next_boundary = body.find(boundary, pos);

		if (next_boundary == std::string::npos) {
			break;
		}
		std::string part_data = body.substr(pos, next_boundary - pos);
		saveMultiFormDataToFile(part_data);
		pos = next_boundary;
	}

	if (status_code == 200) {
		setStatus(201, getStatusCodeMap(201));
	}
}
