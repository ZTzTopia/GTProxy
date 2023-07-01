#include <filesystem>
#include <thread>
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <magic_enum.hpp>

#include "http.h"
#include "ssl.h"
#include "../domain_resolver/domain_resolver.h"
#include "../utils/network.h"
#include "../utils/text_parse.h"

namespace server {
Http::Http(Config* config)
    : config_{ config }
{
    std::string cache_dir{ "./cache" };
    std::filesystem::create_directory(cache_dir);

    const std::string cert_path{ cache_dir + "/cert.pem" };
    if (!std::filesystem::exists(cert_path)) {
        std::ofstream cert_file{ cert_path };
        cert_file << ssl::cert;
        cert_file.close();
    }

    const std::string key_path{ cache_dir + "/key.pem" };
    if (!std::filesystem::exists(key_path)) {
        std::ofstream key_file{ key_path };
        key_file << ssl::key;
        key_file.close();
    }

    server_ = new httplib::SSLServer{ "./cache/cert.pem", "./cache/key.pem" };
}

Http::~Http()
{
    stop();
}

bool Http::bind_to_port(const std::string& host, int port)
{
    if (server_->bind_to_port(host, port)) {
        spdlog::info("HTTP(s) server listening on port {}.", port);
        return true;
    }

    spdlog::error("Failed to bind to port {}.", port);
    return false;
}

void Http::listen_after_bind()
{
    std::thread{ &Http::listen_internal, this }.detach();
}

bool Http::listen(const std::string& host, int port)
{
    if (!bind_to_port(host, port)) {
        return false;
    }

    listen_after_bind();
    return true;
}

void Http::stop()
{
    server_->stop();
}

void Http::listen_internal()
{
    server_->set_logger([](const httplib::Request& req, const httplib::Response& res) {
        spdlog::info("{} {} {}", req.method, req.path, res.status);
    });

    server_->set_error_handler([](const httplib::Request& req, httplib::Response& res) {
        res.set_content(
            fmt::format(
                "Hello, world!\r\n{} ({})",
                httplib::detail::status_message(res.status),
                res.status
            ),
            "text/plain"
        );
    });

    server_->set_exception_handler([](
        const httplib::Request& req,
        httplib::Response& res,
        const std::exception_ptr& ep
    ) {
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

    server_->Post("/growtopia/server_data.php", [&](
        const httplib::Request& req,
        httplib::Response& res
    ) {
        if (!req.headers.empty()) {
            spdlog::info("Headers:");
            for (auto& header : req.headers) {
                spdlog::info("\t{}: {}", header.first, header.second);
            }
        }

        if (!req.params.empty()) {
            spdlog::info("Params:");
            spdlog::info("\t{}", httplib::detail::params_to_query_str(req.params));
        }

        last_headers_ = req.headers;
        last_params_ = req.params;

        utils::TextParse text_parse{ get_server_data() };
        text_parse.set("server", "127.0.0.1");
        text_parse.set("port", config_->get_host().m_port);
        text_parse.set("type2", 1);

        res.set_content(text_parse.get_all_raw(), "text/html");
        return true;
    });

    server_->listen_after_bind();
}

std::string Http::get_server_data()
{
    spdlog::debug("Requesting server data from: https://{}", config_->get_server().m_host);

    auto validate_server_response{
        [](const httplib::Result& response)
        {
            if (!response) {
                return false;
            }

            httplib::Error error_response{ response.error() };

            if (!response) {
                spdlog::error(
                    "Response is null with error: httplib::Error::{}",
                    magic_enum::enum_name(error_response));

                return false;
            }

            int status_code{ response->status };

            if (error_response == httplib::Error::Success && status_code == 200) {
                return true;
            }

            spdlog::error(
                "Failed to get server data. {}.",
                error_response == httplib::Error::Success
                    ? fmt::format("HTTP status code: {}", status_code)
                    : fmt::format("HTTP error: {}", httplib::to_string(error_response)));
            return false;
        }
    };

    std::string resolved_ip = config_->get_server().m_host;

    if (utils::network::classify_host(config_->get_server().m_host) == utils::network::HostType::Hostname) {
        auto res = domain_resolver::resolve_domain_name(config_->get_server().m_host);

        if (res.status_ != domain_resolver::DomainResolverStatus::NoError) {
            spdlog::error(
                "Error occurred while resolving {} ip address. Dns server returned {}",
                config_->get_server().m_host,
                magic_enum::enum_name(res.status_)
            );
            return {};
        }

        resolved_ip = res.ip_;

        spdlog::info(
            "{} ip address is {}",
            config_->get_server().m_host,
            resolved_ip
        );
    }

    httplib::Client cli{ fmt::format("https://{}", resolved_ip) };
    cli.enable_server_certificate_verification(false);

    httplib::Headers header{
        { "User-Agent", get_header_value(last_headers_, "User-Agent") },
        { "Host", get_header_value(last_headers_, "Host") }
    };

    httplib::Result response{ cli.Post("/growtopia/server_data.php", header, last_params_) };
    if (validate_server_response(response)) {
        if (!response->body.empty()) {
            return response->body;
        }
    }

    response = cli.Get("/growtopia/server_data.php");
    if (validate_server_response(response)) {
        if (!response->body.empty()) {
            return response->body;
        }
    }

    spdlog::warn("Failed to retrieve server data from {}", config_->get_server().m_host);
    return {};
}
}
