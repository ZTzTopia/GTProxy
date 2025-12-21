#pragma once
#include <httplib.h>
#include <magic_enum/magic_enum.hpp>
#include <nlohmann/json.hpp>

#include "web_server.hpp"
#include "../../client/client.hpp"
#include "../../core/core.hpp"
#include "../../utils/network.hpp"

namespace extension::web_server {
class WebServerExtension final : public IWebServerExtension {
    core::Core* core_;
    httplib::SSLServer server_;

    std::string address_;
    uint16_t port_;

public:
    explicit WebServerExtension(core::Core* core)
        : core_{ core }
        , server_{ "./resources/cert.pem", "./resources/key.pem" }
        , port_{ 65535 }
    {

    }

    ~WebServerExtension() override
    {
        server_.stop();
    }

    void init() override
    {
        core_->get_event_dispatcher().prependListener(
            core::EventType::Connection,
            [&](const core::EventConnection& evt)
            {
                if (evt.from != core::EventFrom::FromClient) {
                    return;
                }

                // Only allow localhost to redirect to the server.
                if (evt.get_player().get_peer()->address.host != 16777343) {
                    return;
                }

                if (address_.empty() || port_ == 65535) {
                    return;
                }

                std::ignore = core_->get_client()->connect(address_, port_);
                evt.canceled = true;

                address_.clear();
                port_ = 65535;
            }
        );

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

        if (!server_.bind_to_port("0.0.0.0", 443)) {
            spdlog::error("Failed to bind to port 443.");
            return;
        }

        spdlog::info("HTTP(s) server listening on port 443.");
        std::thread{ &WebServerExtension::listen_internal, this }.detach();
    }

    void free() override
    {
        delete this;
    }

protected:
    bool validate_server_response(const httplib::Result& response)
    {
        if (!response) {
            spdlog::error(
                "Response is null with error: httplib::Error::{}",
                magic_enum::enum_name(response.error())
            );
            return false;
        }

        const httplib::Error error_response{ response.error() };
        const int status_code{ response->status };

        if (error_response != httplib::Error::Success || status_code != 200) {
            spdlog::error(
                "Failed to get response from server: {}",
                error_response == httplib::Error::Success
                    ? std::format("HTTP status code: {}", status_code)
                    : std::format("HTTP error: {}", httplib::to_string(error_response))
            );
            return false;
        }

        return true;
    }

    enum class DomainResolverStatus {
        NoError,
        FormatError,
        ServerFail,
        NameError,
        NotImplemented,
        Refused,
        YXDomain,
        YXRRSet,
        NXRRSet,
        NotAuth,
        NotZone
    };

    struct Result {
        DomainResolverStatus status_;
        std::string ip_;
    };

    Result resolve_domain_name(const std::string& domain_name)
    {
        std::string host{
            core_->get_config().get("client.dnsServer") == "cloudflare"
                ? "cloudflare-dns.com"
                : "dns.google"
        };
        std::string path{
            core_->get_config().get("client.dnsServer") == "cloudflare"
                ? "/dns-query"
                : "/resolve"
        };

        httplib::Headers headers = {
            { "Accept", "application/dns-json" }
        };

        static httplib::Client cli{ std::format("https://{}", host) };
        httplib::Result res{ cli.Get(std::format("{}?name={}&type=A", path, domain_name), headers) };
        if (!validate_server_response(res)) {
            return { DomainResolverStatus::ServerFail, {} };
        }

        if (res->body.empty()) {
            return { DomainResolverStatus::ServerFail, {} };
        }

        auto j{ nlohmann::json::parse(res->body) };
        const DomainResolverStatus status{ j["Status"] };

        if (status != DomainResolverStatus::NoError) {
            return { status, {} };
        }

        return {
            status,
            j["Answer"][j["Answer"].size() - 1]["data"].get<std::string>()
        };
    }

    std::string resolve_ip_address(std::string_view host)
    {
        std::string resolved_ip{ host };
        if (network::classify_host(resolved_ip) == network::HostType::Hostname) {
            auto [status, ip]{ resolve_domain_name(resolved_ip) };

            if (status != DomainResolverStatus::NoError) {
                spdlog::error(
                    "Error occurred while resolving {} ip address. DNS server returned {}",
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

    void listen_internal()
    {
        server_.Post("/growtopia/server_data.php", [&](
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

            httplib::Client cli{
                std::format(
                    "https://{}",
                    resolve_ip_address(core_->get_config().get("server.address"))
                )
            };
            cli.enable_server_certificate_verification(false);

            const httplib::Headers headers {
                { "User-Agent", req.get_header_value("User-Agent") },
                { "Host", core_->get_config().get("server.address") }
            };

            httplib::Result result{ cli.Post("/growtopia/server_data.php", headers, req.params) };
            if (!validate_server_response(result)) {
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

            // Set server address and port that client (Proxy) should connect to.
            address_ = text_parse.get("server");
            port_ = std::stoi(text_parse.get("port"));

            // Set server address and port that client (Growtopia) should connect to.
            text_parse.set("server", { "127.0.0.1" });
            text_parse.set("port", { std::to_string(core_->get_config().get<unsigned int>("server.port")) });
            text_parse.set("type2", { "1" });

            res.set_content(text_parse.get_raw(), "text/html");
            return true;
        });

        server_.listen_after_bind();
    }
};
}
