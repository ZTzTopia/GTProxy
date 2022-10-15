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

    std::string request_server_data();

private:
    void listen_internal();

public:
    void set_server_data_headers(const httplib::Headers& headers) { m_server_data_headers = headers; }

    void set_server_data_params(const httplib::Params& params) { m_server_data_params = params; }

private:
    Config* m_config;
    httplib::Server* m_server;
    httplib::Headers m_server_data_headers;
    httplib::Params m_server_data_params;
};
}
