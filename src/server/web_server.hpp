#pragma once
#include <string>
#include <httplib/httplib.h>

#include "../core/core.hpp"

namespace server {
class WebServer {
public:
    explicit WebServer(core::Core* core);
    ~WebServer();

    bool bind_to_port(const std::string& host, uint16_t port);
    void listen_after_bind();
    bool listen(const std::string& host, uint16_t port);
    void stop();

protected:
    void listen_internal();

private:
    core::Core* core_;
    httplib::SSLServer server_;
};
}
