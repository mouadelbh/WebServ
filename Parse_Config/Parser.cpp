#include "Parser.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <limits>

LocationConfig::LocationConfig() : autoindex(-1), client_size(0) {}

ServerConfig::ServerConfig() : port(80), autoindex(-1), client_size(std::numeric_limits<size_t>::max()) {}

Parser::Parser(const std::string& filePath) : _filePath(filePath) {
    readFile();
    tokenizeAndParse();
}

Parser::~Parser() {}

const std::vector<ServerConfig>& Parser::getServers() const {
    return _servers;
}

void Parser::readFile() {
    std::ifstream file(_filePath.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Error: Could not open configuration file.");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    _content = buffer.str();
    file.close();
}

std::string Parser::getNextToken(size_t& pos) {
    while (pos < _content.length() && std::isspace(_content[pos])) {
        pos++;
    }

    if (pos >= _content.length()) {
        return "";
    }

    if (_content[pos] == '{' || _content[pos] == '}' || _content[pos] == ';') {
        return std::string(1, _content[pos++]);
    }

    size_t start = pos;
    while (pos < _content.length() && !std::isspace(_content[pos]) &&
           _content[pos] != '{' && _content[pos] != '}' && _content[pos] != ';') {
        pos++;
    }
    return _content.substr(start, pos - start);
}

void Parser::expectToken(const std::string& expected, size_t& pos) {
    std::string token = getNextToken(pos);
    if (token != expected) {
        throw std::runtime_error("Parser Error: Expected '" + expected + "' but got '" + token + "'");
    }
}

void Parser::parseErrorPage(std::map<int, std::string>& error_pages, size_t& pos) {
    std::vector<std::string> tokens;
    std::string token;

    while ((token = getNextToken(pos)) != ";") {
        if (token.empty()) {
            throw std::runtime_error("Parser Error: Unexpected end of file in error_page directive.");
        }
        tokens.push_back(token);
    }

    if (tokens.size() < 2) {
        throw std::runtime_error("Parser Error: Invalid error_page directive. Requires at least one code and a path.");
    }

    std::string page = tokens.back();
    tokens.pop_back();

    for (size_t i = 0; i < tokens.size(); ++i) {
        bool is_numeric = !tokens[i].empty() && tokens[i].find_first_not_of("0123456789") == std::string::npos;
        if (!is_numeric) {
             throw std::runtime_error("Parser Error: Invalid error code '" + tokens[i] + "' in error_page directive.");
        }
        long code = std::strtol(tokens[i].c_str(), NULL, 10);
        if (code < 300 || code >= 600) {
             throw std::runtime_error("Parser Error: Invalid error code '" + tokens[i] + "' (must be between 300 and 599).");
        }
        error_pages[static_cast<int>(code)] = page;
    }
}


void Parser::parseLocationBlock(size_t& pos, ServerConfig& server) {
    LocationConfig location;
    location.path = getNextToken(pos);
    if (location.path.empty() || location.path == "{") {
        throw std::runtime_error("Parser Error: Missing location path.");
    }

    expectToken("{", pos);

    std::string token;
    while ((token = getNextToken(pos)) != "}") {
        if (token.empty()) {
            throw std::runtime_error("Parser Error: Unexpected end of file in location block.");
        }
        if (token == "root") {
            expectToken(";", pos);
        } else if (token == "methods") {
            while ((token = getNextToken(pos)) != ";") {
                if (token.empty()) throw std::runtime_error("Parser Error: Unexpected end of file in methods directive.");
                location.methods.push_back(token);
            }
        } else if (token == "autoindex") {
            token = getNextToken(pos);
            location.autoindex = (token == "on");
            expectToken(";", pos);
        } else if (token == "index") {
            location.index = getNextToken(pos);
            expectToken(";", pos);
        } else if (token == "return") {
            location.redirect = getNextToken(pos);
            expectToken(";", pos);
        } else if (token == "client_size") {
            std::stringstream ss(getNextToken(pos));
            ss >> location.client_size;
            if (ss.fail()) throw std::runtime_error("Parser Error: Invalid client_size value.");
            expectToken(";", pos);
        }else {
            throw std::runtime_error("Parser Error: Unknown directive '" + token + "' in location block.");
        }
    }
    server.locations[location.path] = location;
}

void Parser::parseServerBlock(size_t& pos) {
    expectToken("{", pos);
    ServerConfig server;

    std::string token;
    while ((token = getNextToken(pos)) != "}")
    {
        if (token.empty()) {
            throw std::runtime_error("Parser Error: Unexpected end of file in server block.");
        }
        if (token == "listen") {
            std::string listen_str = getNextToken(pos);
            size_t colon_pos = listen_str.find(':');
            if (colon_pos != std::string::npos) {
                server.host = listen_str.substr(0, colon_pos);
                std::stringstream ss(listen_str.substr(colon_pos + 1));
                ss >> server.port;
                 if (ss.fail()) throw std::runtime_error("Parser Error: Invalid port value.");
            } else {
                server.host = listen_str;
            }
            expectToken(";", pos);
        } else if (token == "server_name") {
            server.server_name = getNextToken(pos);
            expectToken(";", pos);
        } else if (token == "root") {
            server.root = getNextToken(pos);
            expectToken(";", pos);
        } else if (token == "autoindex") {
            token = getNextToken(pos);
            server.autoindex = (token == "on");
            expectToken(";", pos);
        } else if (token == "index") {
            server.index = getNextToken(pos);
            expectToken(";", pos);
        } else if (token == "methods") {
             while ((token = getNextToken(pos)) != ";") {
                 if (token.empty()) throw std::runtime_error("Parser Error: Unexpected end of file in methods directive.");
                server.methods.push_back(token);
            }
        } else if (token == "client_size") {
            std::stringstream ss(getNextToken(pos));
            ss >> server.client_size;
            if (ss.fail()) throw std::runtime_error("Parser Error: Invalid client_size value.");
            expectToken(";", pos);
        } else if (token == "error_page") {
            parseErrorPage(server.error_pages, pos);
        } else if (token == "location") {
            parseLocationBlock(pos, server);
        }
        else if (token == "cgi_ext") {
            std::string ext = getNextToken(pos);
            if (ext.empty()) {
                throw std::runtime_error("Parser Error: Missing CGI extension in cgi_ext directive.");
            }
            std::string path = getNextToken(pos);
            if (path.empty()) {
                throw std::runtime_error("Parser Error: Missing CGI path in cgi_ext directive.");
            }
            server.cgi_ext[ext] = path;
            expectToken(";", pos);
        } else {
            throw std::runtime_error("Parser Error: Unknown directive '" + token + "' in server block.");
        }
    }
    _servers.push_back(server);
}

void Parser::tokenizeAndParse() {
    size_t pos = 0;
    while (pos < _content.length()) {
        std::string token = getNextToken(pos);
        if (token.empty()) {
            break;
        }
        if (token == "server") {
            parseServerBlock(pos);
        } else {
            throw std::runtime_error("Parser Error: Expected 'server' block.");
        }
    }
}
