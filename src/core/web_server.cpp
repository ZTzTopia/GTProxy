#include "web_server.hpp"

#include <fmt/format.h>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "../utils/text_parse.hpp"
// ReSharper disable once CppUnusedIncludeDirective
#include "../utils/formatter/text_parse_formatter.hpp"

namespace core {
WebServer::WebServer(
    Config& config,
    event::Dispatcher& dispatcher,
    network::Client& client,
    network::Server& server
)
    : config_{ config }
    , dispatcher_{ dispatcher }
    , client_{ client }
    , server_{ server }
    , dns_resolver_{ network::create_dns_provider(config_.get_client_config().dns_server) }
    , https_server_{ "./resources/cert.pem", "./resources/key.pem" }
    , pending_port_{ 65535 }
{
    setup_server();

    if (!https_server_.bind_to_port("0.0.0.0", 443)) {
        spdlog::error("Failed to bind to port 443");
        return;
    }

    dispatcher_.appendListener(event::Type::ClientConnect, [this](const event::Event& e) { on_client_connect(e); });
    dispatcher_.appendListener(event::Type::ClientDisconnect, [this](const event::Event& e) { on_client_disconnect(e); });

    spdlog::info("HTTPS server listening on port 443");
    server_thread_ = std::thread{ &WebServer::listen_internal, this };
}

WebServer::~WebServer()
{
    https_server_.stop();
    server_thread_.join();
}

void WebServer::setup_server()
{
    https_server_.set_logger([](const httplib::Request& req, const httplib::Response& res) {
        spdlog::info("{} {} {}", req.method, req.path, res.status);
    });

    https_server_.set_error_handler([](const httplib::Request&, httplib::Response& res) {
        res.set_content(
            fmt::format("{} ({})", httplib::status_message(res.status), res.status),
            "text/plain"
        );
    });

    https_server_.set_exception_handler([](const httplib::Request&, httplib::Response& res, const std::exception_ptr& ep) {
        res.status = 500;
        try {
            std::rethrow_exception(ep);
        }
        catch (const std::exception& e) {
            res.set_content(fmt::format("{}", e.what()), "text/plain");
        }
        catch (...) { // This should never happen, but just in case
            res.set_content("Unknown Exception", "text/plain");
        }
    });
}

void WebServer::listen_internal()
{
    https_server_.Post("/growtopia/server_data.php", [this](
        const httplib::Request& req,
        httplib::Response& res
    ) -> bool {
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

        std::string server_address = config_.get_server_config().address;
        std::string resolved_ip = dns_resolver_.resolve_ip(server_address);

        if (resolved_ip.empty()) {
            res.status = 500;
            res.set_content("Failed to resolve server address", "text/plain");
            return true;
        }

        httplib::Client cli{ fmt::format("https://{}", resolved_ip) };
        cli.enable_server_certificate_verification(false);

        httplib::Headers headers{
            { "User-Agent", req.get_header_value("User-Agent") },
            { "Host", server_address }
        };

        httplib::Result response{ cli.Post("/growtopia/server_data.php", headers, req.params) };
        if (!response) {
            spdlog::error(
                "Response is null: httplib::Error::{}",
                magic_enum::enum_name(response.error())
            );

            res.status = 502;
            res.set_content("Failed to get response from server", "text/plain");
            return true;
        }

        if (response.error() != httplib::Error::Success || response->status != 200) {
            spdlog::error(
                "Failed to get response: {}",
                response.error() == httplib::Error::Success
                    ? fmt::format("HTTP status: {}", response->status)
                    : fmt::format("HTTP error: {}", httplib::to_string(response.error()))
            );

            res.status = 502;
            res.set_content("Failed to get response from server", "text/plain");
            return true;
        }

        if (response->body.empty()) {
            res.status = 502;
            res.set_content("Empty response from server", "text/plain");
            return true;
        }

        TextParse text_parse{ response->body };
        if (text_parse.empty()) {
            spdlog::error("Failed to parse server_data.php response");
            res.status = 500;
            return true;
        }

        spdlog::info("Original server_data.php response:\n{}", text_parse);

        pending_address_ = text_parse.get("server", 0);

        std::string port{ text_parse.get("port", 0) };
        if (
            auto [ptr, ec] = std::from_chars(port.data(), port.data() + port.size(), pending_port_);
            ec != std::errc{}
        ) {
            spdlog::error("Failed to parse port from server_data.php response");
            res.status = 500;
            return true;
        }

        text_parse.set("server", { "127.0.0.1" });
        text_parse.set("port", { std::to_string(config_.get_server_config().port) });
        text_parse.set("type2", { "1" });

        spdlog::debug("Modified server_data.php response:\n{}", text_parse);

        res.set_content(text_parse.get_raw(), "text/html");
        return true;
    });

    https_server_.listen_after_bind();
}

void WebServer::on_client_connect(const event::Event& e)
{
    if (pending_port_ == 65535) {
        return;
    }

    spdlog::debug("Connecting to Growtopia server at {}:{}", pending_address_, pending_port_);

    client_.connect(pending_address_, pending_port_);

    pending_address_.clear();
    pending_port_ = 65535;
}

void WebServer::on_client_disconnect(const event::Event& e)
{
    pending_address_.clear();
    pending_port_ = 65535;
}
}
