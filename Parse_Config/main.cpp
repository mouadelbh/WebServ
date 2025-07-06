#include "Parser.hpp"
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }

    try {
        Parser parser(argv[1]);
        const std::vector<ServerConfig>& servers = parser.getServers();

        for (size_t i = 0; i < servers.size(); ++i) {
            std::cout << "Server #" << i + 1 << ":" << std::endl;
            std::cout << "  Host: " << servers[i].host << std::endl;
            std::cout << "  Port: " << servers[i].port << std::endl;
            std::cout << "  Server Name: " << servers[i].server_name << std::endl;
            std::cout << "  Root: " << servers[i].root << std::endl;
            std::cout << "  Client Size: " << servers[i].client_size << std::endl;
            std::cout << "  Autoindex: " << (servers[i].autoindex ? "on" : "off") << std::endl;
            
            std::cout << "  Methods: ";
            for(size_t m = 0; m < servers[i].methods.size(); ++m) std::cout << servers[i].methods[m] << " ";
            std::cout << std::endl;

            for (std::map<int, std::string>::const_iterator it = servers[i].error_pages.begin(); it != servers[i].error_pages.end(); ++it) {
                std::cout << "  Error Page: " << it->first << " -> " << it->second << std::endl;
            }

            for (size_t j = 0; j < servers[i].locations.size(); ++j) {
                std::cout << "  Location " << servers[i].locations[j].path << ":" << std::endl;
                std::cout << "    Root: " << servers[i].locations[j].root << std::endl;
                std::cout << "    Index: " << servers[i].locations[j].index << std::endl;
                std::cout << "    Autoindex: " << (servers[i].locations[j].autoindex ? "on" : "off") << std::endl;
                std::cout << "    CGI Pass: " << servers[i].locations[j].cgi_pass << std::endl;
                 std::cout << "    Client Size: " << servers[i].locations[j].client_size << std::endl;
                std::cout << "    Redirect: " << servers[i].locations[j].redirect << std::endl;

                std::cout << "    Methods: ";
                for(size_t m = 0; m < servers[i].locations[j].methods.size(); ++m) std::cout << servers[i].locations[j].methods[m] << " ";
                std::cout << std::endl;

                 for (std::map<int, std::string>::const_iterator it = servers[i].locations[j].error_pages.begin(); it != servers[i].locations[j].error_pages.end(); ++it) {
                    std::cout << "    Error Page: " << it->first << " -> " << it->second << std::endl;
                }
            }
             std::cout << "--------------------" << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}