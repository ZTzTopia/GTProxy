#pragma once
#include <string>
#include <httplib/httplib.h>

#include "../config.h"

namespace server {
class Http {
public:
    explicit Http(Config* config);
    ~Http();

    bool bind_to_port(const std::string& host, int port);
    void listen_after_bind();
    bool listen(const std::string& host, int port);
    void stop();

private:
    void listen_internal();

public:
    std::string get_server_data();

private:
    Config* config_;
    httplib::Server* server_;
    httplib::Headers last_headers_;
    httplib::Params last_params_;
};
}
