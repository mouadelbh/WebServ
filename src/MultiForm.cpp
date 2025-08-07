/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MutliForm.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 09:49:35 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/08/07 09:51:23 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"

void	Response::saveDataToFile(const std::string &data, std::string filename) {
	std::string upload_dir = "www/uploads"; // Or get from config
	// Ensure the upload directory exists
	if (access(upload_dir.c_str(), F_OK) == -1) {
		if (mkdir(upload_dir.c_str(), 0755) == -1) {
			setStatus(500, "Internal Server Error");
			return;
		}
	}
	std::cout << RED << data << RESET << std::endl;
	std::cout << "filename: " << filename << std::endl;
	// Create a unique filename (e.g., using a timestamp)
	if (filename.empty()) {
		std::time_t result = std::time(nullptr);
		filename = upload_dir + "/upload_" + std::to_string(result) + ".txt";
	}
	else
		filename = upload_dir + "/" + filename;
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
	std::cout << GREEN << data << RESET << std::endl;
	setStatus(201, "Created");
	headers["Location"] = upload_dir;
}

void Response::saveMultiFormDataToFile(std::string data) {
	size_t headers_pos = data.find("\r\n\r\n", 0);
	if (headers_pos == std::string::npos) {
		std::cerr << "Invalid multipart data format." << std::endl;
		return;
	}

	// Extract components
	std::string headers = data.substr(0, headers_pos);
	std::string filename = extractFilename(headers);
	std::string content = removeTrailingCRLF(data.substr(headers_pos + 4));

	// Early return for empty content
	if (content.empty() && filename.empty()) {
		return;
	}

	// Use default filename if none provided
	if (filename.empty()) {
		filename = generateDefaultFilename();
	}

	// Process content based on file type
	std::string final_content;
	if (isTextFile(filename)) {
		final_content = processTextContent(content);
		std::cout << GREEN << "Processing as text file: " << filename << RESET << std::endl;
	} else {
		final_content = content;
		std::cout << GREEN << "Processing as binary file: " << filename << RESET << std::endl;
	}

	// Save the file
	saveDataToFile(final_content, filename);

	std::cout << GREEN << "Saved file: " << filename
				<< " (size: " << final_content.length() << " bytes)" << RESET << std::endl;
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

	std::cout << YELLOW << body << RESET << std::endl;
	std::cout << "Boundary: [" << boundary << "]" << std::endl;
	std::cout << "Last boundary: [" << last_boundary << "]" << std::endl;
	// Find first boundary
	pos = body.find(boundary);
	if (pos == std::string::npos) {
		request->status = 400;
		return;
	}

	while (pos != std::string::npos) {
		std::cout << "Current pos: " << pos << std::endl;

		if (body.substr(pos, last_boundary.length()) == last_boundary) {
			std::cout << "Found final boundary!" << std::endl;
			break;
		}

		pos += boundary.length();
		if (pos + 2 <= body.length() && body.substr(pos, 2) == "\r\n") {
			pos += 2;
		}

		size_t next_boundary = body.find(boundary, pos);

		if (next_boundary == std::string::npos) {
			std::cout << "No more boundaries found" << std::endl;
			break;
		}

		std::cout << "Processing part from " << pos << " to " << next_boundary << std::endl;

		std::string part_data = body.substr(pos, next_boundary - pos);

		// Remove trailing CRLF

		saveMultiFormDataToFile(part_data);

		// Move past the current boundary so we don't reprocess it
		pos = next_boundary;
	}

	if (status_code == 200) {
		setStatus(201, "Created");
	}
}
