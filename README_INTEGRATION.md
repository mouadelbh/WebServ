# WebServ Configuration Integration

## Overview
This WebServ implementation now includes a fully integrated configuration parser that allows you to define multiple servers with different settings, locations, and error pages through a simple configuration file.

## Key Features

### 1. **Configuration-Driven Architecture**
- **Parser Integration**: The `Parse_Config/Parser.cpp` is now fully integrated with the main server
- **Dynamic Configuration**: Server settings are loaded from `Parse_Config/default.conf`
- **Multiple Servers**: Support for multiple server configurations (though currently only the first is used)

### 2. **Enhanced Server Configuration**
- **Host & Port Binding**: Servers bind to configured host:port combinations
- **Server Names**: Each server can have its own name
- **Root Directories**: Configurable document roots
- **Method Restrictions**: Define allowed HTTP methods per server/location
- **Client Size Limits**: Configurable maximum request body sizes
- **Autoindex Control**: Enable/disable directory listing per server/location

### 3. **Location-Based Routing**
- **Path Matching**: Exact path and prefix matching
- **Extension Matching**: Handle specific file extensions (e.g., `*.php`)
- **Per-Location Settings**: Override server settings for specific locations
- **Location-Specific Roots**: Different document roots for different paths

### 4. **Error Page Management**
- **Custom Error Pages**: Define custom error pages for different status codes
- **Hierarchical Configuration**: Location settings override server settings
- **Automatic Fallback**: Falls back to default error pages if custom ones aren't found

## Configuration Structure

### Server Block
```
server {
    listen              localhost:8080;
    server_name         webserv;
    root                /www;
    index               index.html;
    methods             GET POST DELETE;
    autoindex           on;
    client_size         10485760;
    error_page          404 /status_errors/404.html;
    
    location / {
        # Location-specific settings
    }
}
```

### Current Configuration
The system is configured with 3 servers:

1. **Main Server** (localhost:8080)
   - Full-featured server with multiple locations
   - Supports GET, POST, DELETE methods
   - Multiple location blocks for different purposes

2. **Admin Server** (127.0.0.1:8081)
   - Administrative interface
   - Restricted access and methods
   - Separate admin content

3. **Test Server** (localhost:8082)
   - Testing environment
   - Limited to GET methods only
   - Smaller client size limits

## Directory Structure

```
www/
├── index.html              # Main server homepage
├── admin/
│   └── admin.html         # Admin interface
├── test/
│   └── index.html         # Test server content
├── uploads/
│   └── index.html         # Upload directory info
└── status_errors/
    ├── 400.html
    ├── 403.html
    ├── 404.html
    └── ...                # Error pages
```

## Usage

### Compilation
```bash
make clean && make
```

### Running the Server
```bash
# Use default configuration
./webserv

# Use custom configuration file
./webserv path/to/config.conf
```

### Testing
```bash
# Validate configuration
./validate_config.sh

# Run integration test
./test_integration.sh
```

### Accessing the Servers
- **Main Server**: http://localhost:8080
- **Admin Panel**: http://127.0.0.1:8081
- **Test Server**: http://localhost:8082

## Integration Details

### Code Changes Made

1. **Parser Integration**
   - Added `Parse_Config/Parser.hpp` to includes
   - Modified `Server` class to accept `ServerConfig`
   - Updated `Client` and `Request` classes to use configuration

2. **Configuration Passing**
   - `Server` stores `ServerConfig` and passes it to clients
   - `Request` and `Response` classes have configuration awareness
   - Location matching and method validation implemented

3. **Dynamic Behavior**
   - Root paths determined by configuration
   - Error pages loaded from configured locations
   - Autoindex behavior controlled by configuration
   - Method restrictions enforced

4. **Enhanced Features**
   - Location-based routing
   - Configuration-driven error pages
   - Method validation per location
   - Client size limits per location

## Configuration Changes

When you modify the configuration file, the following will change automatically:

- **Server binding** (host:port combinations)
- **Document roots** (where files are served from)
- **Allowed methods** (GET, POST, DELETE restrictions)
- **Error page locations** (custom error pages)
- **Directory indexing** (autoindex on/off)
- **Client size limits** (request body size restrictions)
- **Location-specific overrides** (per-path settings)

## Benefits

1. **Flexibility**: Easy to configure multiple servers with different settings
2. **Maintainability**: Configuration changes don't require recompilation
3. **Scalability**: Support for multiple servers and complex routing
4. **Security**: Method restrictions and access controls
5. **User Experience**: Custom error pages and proper content serving

The integration maintains backward compatibility while adding powerful configuration capabilities that make the WebServ more versatile and production-ready.
