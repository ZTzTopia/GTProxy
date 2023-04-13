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
    Config* m_config;
    std::unique_ptr<httplib::Server> m_server;
    httplib::Headers m_last_headers;
    httplib::Params m_last_params;
};
}
