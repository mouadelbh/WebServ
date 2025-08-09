/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils3.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 09:46:50 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/08/09 16:29:16 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/include.hpp"

int stringToInt(const std::string &s) {
    std::istringstream iss(s);
    int value;
    if (!(iss >> value)) {
        throw std::runtime_error("Invalid integer: " + s);
    }
    return value;
}

// Helper function to convert string to lowercase (C++98 compatible)
std::string toLower(const std::string& str) {
    std::string result = str;
    for (size_t i = 0; i < result.length(); ++i) {
        result[i] = std::tolower(result[i]);
    }
    return result;
}

// Helper function to check if file extension indicates text file
bool isTextFile(const std::string& filename) {
    size_t dot_pos = filename.find_last_of('.');
    if (dot_pos == std::string::npos) {
        return false;
    }

    std::string extension = toLower(filename.substr(dot_pos + 1));

    // Check against common text file extensions
    return (extension == "txt" || extension == "csv" ||
            extension == "html" || extension == "css" ||
            extension == "js" || extension == "json" ||
            extension == "xml" || extension == "md" ||
            extension == "htm" || extension == "log");
}

// Helper function to extract filename from multipart headers
std::string extractFilename(const std::string& headers) {
    size_t pos = headers.find("filename=\"");
    if (pos == std::string::npos) {
        return "";
    }

    pos += 10; // Skip 'filename="'
    size_t end_pos = headers.find('"', pos);
    if (end_pos == std::string::npos) {
        return "";
    }

    return headers.substr(pos, end_pos - pos);
}

// Helper function to process text content (convert \r\n to \n)
std::string processTextContent(const std::string& content) {
    std::string processed_content;
    processed_content.reserve(content.length()); // Optimize memory allocation

    for (size_t i = 0; i < content.length(); ++i) {
        if (i < content.length() - 1 &&
            content[i] == '\r' && content[i + 1] == '\n') {
            processed_content += '\n';
            ++i; // Skip the \n
        } else {
            processed_content += content[i];
        }
    }

    return processed_content;
}

// Helper function to remove trailing CRLF
std::string removeTrailingCRLF(const std::string& content) {
    if (content.length() >= 2 &&
        content.substr(content.length() - 2) == "\r\n") {
        return content.substr(0, content.length() - 2);
    }
    return content;
}

// Helper function to generate default filename
std::string generateDefaultFilename() {
    std::ostringstream oss;
    oss << "upload_" << std::time(NULL) << ".txt";
    return oss.str();
}
