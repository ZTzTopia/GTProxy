#include <filesystem>
#include <thread>
#include <spdlog/spdlog.h>

#include "web_server.hpp"
#include "../client/domain_resolver.hpp"
#include "../utils/network.hpp"

namespace server {
WebServer::WebServer(core::Core* core)
    : core_{ core }
    , server_{ "./resources/cert.pem", "./resources/key.pem" }
{

}

WebServer::~WebServer()
{
    stop();
}

bool WebServer::bind_to_port(const std::string& host, const uint16_t port)
{
    if (server_.bind_to_port(host, port)) {
        spdlog::info("HTTP(s) server listening on port {}.", port);
        return true;
    }

    spdlog::error("Failed to bind to port {}.", port);
    return false;
}

void WebServer::listen_after_bind()
{
    std::thread{ &WebServer::listen_internal, this }.detach();
}

bool WebServer::listen(const std::string& host, const uint16_t port)
{
    if (!bind_to_port(host, port)) {
        return false;
    }

    listen_after_bind();
    return true;
}

void WebServer::stop()
{
    server_.stop();
}

std::string resolve_ip_address(std::string_view host)
{
    std::string resolved_ip{ host };
    if (network::classify_host(resolved_ip) == network::HostType::Hostname) {
        auto [status, ip]{ client::domain_resolver::resolve_domain_name(resolved_ip) };

        if (status != client::domain_resolver::DomainResolverStatus::NoError) {
            spdlog::error(
                "Error occurred while resolving {} ip address. Dns server returned {}",
                host,
                magic_enum::enum_name(status)
            );
            return {};
        }

        resolved_ip = ip;
        spdlog::info("Resolved {} to {}", host, resolved_ip);
    }

    return resolved_ip;
}

void WebServer::listen_internal()
{
    server_.set_logger([](const httplib::Request& req, const httplib::Response& res)
    {
        spdlog::info("{} {} {}", req.method, req.path, res.status);
    });

    server_.set_error_handler([](const httplib::Request&, httplib::Response& res)
    {
        res.set_content(
            fmt::format(
                "Hello, world!\r\n{} ({})",
                httplib::status_message(res.status),
                res.status
            ),
            "text/plain"
        );
    });

    server_.set_exception_handler([](const httplib::Request&, httplib::Response& res, const std::exception_ptr& ep)
    {
        res.status = 500;

        try {
            std::rethrow_exception(ep);
        }
        catch (std::exception &e) {
            res.set_content(fmt::format("Hello, world!\r\n{}", e.what()), "text/plain");
        }
        catch (...) {
            res.set_content("Hello, world!\r\nUnknown Exception", "text/plain");
        }
    });

    server_.Post("/growtopia/server_data.php", [&](const httplib::Request& req, httplib::Response& res)
    {
        if (!req.headers.empty()) {
            spdlog::info("Headers:");
            for (const auto& [key, value] : req.headers) {
                spdlog::info("\t{}: {}", key, value);
            }
        }

        if (!req.params.empty()) {
            spdlog::info("Params:");
            spdlog::info("\t{}", httplib::detail::params_to_query_str(req.params));
        }

        if (!req.body.empty()) {
            spdlog::info("Body:");
            spdlog::info("\t{}", req.body);
        }

        httplib::Client cli{
            std::format(
                "https://{}",
                resolve_ip_address(core_->get_config().get("server.address"))
            )
        };
        cli.enable_server_certificate_verification(false);

        const httplib::Headers headers {
            { "User-Agent", get_header_value(req.headers, "User-Agent") },
            { "Host", core_->get_config().get("server.address") }
        };

        httplib::Result result{ cli.Post("/growtopia/server_data.php", headers, req.params) };
        if (!network::validate_server_response(result)) {
            return true;
        }

        if (result->body.empty()) {
            return true;
        }

        TextParse text_parse{ result->body };
        if (text_parse.empty()) {
            spdlog::error("Failed to parse server_data.php response.");
            res.status = 500;
            return true;
        }

        text_parse.set("server", { "127.0.0.1" });
        text_parse.set("port", { std::to_string(core_->get_config().get<unsigned int>("server.port")) });
        text_parse.set("type2", { "1" });

        res.set_content(text_parse.get_raw(), "text/html");
        return true;
    });

    server_.listen_after_bind();
}
}
