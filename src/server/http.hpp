#pragma once
#include <string>
#include <httplib/httplib.h>

#include "../core/core.hpp"

namespace server {
class Http {
public:
    explicit Http(core::Core* core);
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
    core::Core* core_;
    httplib::SSLServer server_;

    httplib::Headers last_headers_;
    httplib::Params last_params_;
};
}
