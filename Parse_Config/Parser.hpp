#pragma once

#include <string>
#include <vector>
#include <map>
#include <stdexcept>

struct LocationConfig {
    std::string path;
    std::string root;
    bool autoindex;
    std::string index;
    std::vector<std::string> methods;
    std::string redirect;
    std::string cgi_pass;
    std::string upload_path;
    size_t client_size;
    std::map<int, std::string> error_pages;

    LocationConfig();
};

struct ServerConfig {
    std::string host;
    int port;
    std::string server_name;
    std::string root;
    bool autoindex;
    std::string index;
    std::vector<std::string> methods;
    size_t client_size;
    std::map<int, std::string> error_pages;
    std::vector<LocationConfig> locations;

    ServerConfig();
};

class Parser {
public:
    Parser(const std::string& filePath);
    ~Parser();

    const std::vector<ServerConfig>& getServers() const;

private:
    std::string _filePath;
    std::vector<ServerConfig> _servers;
    std::string _content;

    void readFile();
    void tokenizeAndParse();
    void parseServerBlock(size_t& pos);
    void parseLocationBlock(size_t& pos, ServerConfig& server);
    void parseErrorPage(std::map<int, std::string>& error_pages, size_t& pos);

    std::string getNextToken(size_t& pos);
    void expectToken(const std::string& expected, size_t& pos);
};
