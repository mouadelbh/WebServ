#pragma once

#include <vector>
#include <unordered_map>
#include <cstddef>

// Forward declarations
class Server;
class Client;
struct ServerConfig;

class ServerManager {
public:
    ServerManager();
    ~ServerManager();
    
    void initServers(const std::vector<ServerConfig>& configs);
    void runServers();
    void stopServers();
    
private:
    std::vector<Server*> servers;
    std::vector<std::unordered_map<int, Client>*> clientMaps;
    bool running;
    
    void runSingleServer(size_t serverIndex);
};
