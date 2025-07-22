#include "../includes/ServerManager.hpp"
#include "../includes/include.hpp"
#include <iostream>

ServerManager::ServerManager() : running(false) {}

ServerManager::~ServerManager() {
    stopServers();
    // Clean up dynamically allocated servers and client maps
    for (auto* server : servers) {
        delete server;
    }
    for (auto* clientMap : clientMaps) {
        delete clientMap;
    }
}

void ServerManager::initServers(const std::vector<ServerConfig>& configs) {
    // Clean up existing servers
    for (auto* server : servers) {
        delete server;
    }
    for (auto* clientMap : clientMaps) {
        delete clientMap;
    }
    servers.clear();
    clientMaps.clear();
    
    // Create new servers
    for (size_t i = 0; i < configs.size(); ++i) {
        try {
            Server* server = new Server();
            server->initServer(configs[i]);
            servers.push_back(server);
            clientMaps.push_back(new std::unordered_map<int, Client>());
            std::cout << "✓ Server " << (i + 1) << " initialized successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "✗ Failed to initialize server " << (i + 1) << ": " << e.what() << std::endl;
        }
    }
}

void ServerManager::runSingleServer(size_t serverIndex) {
    if (serverIndex >= servers.size()) return;
    
    Server* server = servers[serverIndex];
    std::unordered_map<int, Client>* clients = clientMaps[serverIndex];
    
    std::cout << "Starting server " << (serverIndex + 1) << " on " 
              << server->getConfig().host << ":" << server->getConfig().port << std::endl;
    
    server->runServer(*clients);
}

void ServerManager::runServers() {
    if (servers.empty()) {
        std::cerr << "No servers to run!" << std::endl;
        return;
    }
    
    running = true;
    
    std::cout << "\n=== Starting WebServ Multi-Server ===\n" << std::endl;
    
    if (!servers.empty()) {
        runSingleServer(0); // Run the first server
    }
}

void ServerManager::stopServers() {
    running = false;
    std::cout << "\nStopping all servers..." << std::endl;
}
