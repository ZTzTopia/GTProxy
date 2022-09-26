#include <filesystem>
#include <thread>
#include <spdlog/spdlog.h>

#include "http.h"
#include "ssl.h"
#include "../utils/text_parse.h"

namespace server {
    Http::Http(Config* config) : m_config{ config }
    {
        std::string temp_dir{ "./cache" };
        if (!std::filesystem::exists(temp_dir)) {
            std::filesystem::create_directory(temp_dir);
        }

        if (!std::filesystem::exists("./cache/cert.pem")) {
            std::ofstream cert_file{ "./cache/cert.pem" };
            cert_file << ssl::cert;
            cert_file.close();
        }

        if (!std::filesystem::exists("./cache/key.pem")) {
            std::ofstream key_file{ "./cache/key.pem" };
            key_file << ssl::key;
            key_file.close();
        }

        m_server = new httplib::SSLServer{ "./cache/cert.pem", "./temp/cache.pem" };
    }

    Http::~Http()
    {
        stop();
    }

    bool Http::bind_to_port(const std::string& host, int port)
    {
        bool ret{ m_server->bind_to_port(host.c_str(), port) };
        if (ret) {
            // So we don't need to store port in a member variable.
            spdlog::info("HTTP(s) server listening on port {}.", port);
        }

        return ret;
    }

    void Http::listen_after_bind()
    {
        std::thread{ &Http::listen_internal, this }.detach();
    }

    bool Http::listen(const std::string& host, int port)
    {
        if (!bind_to_port(host, port)) {
            spdlog::error("Failed to bind to port {}.", port);
            return false;
        }

        listen_after_bind();
        return true;
    }

    void Http::stop()
    {
        m_server->stop();
    }

    void Http::listen_internal()
    {
        m_server->set_logger([](const httplib::Request& req, const httplib::Response& res) {
            spdlog::info("{} {} {}", req.method, req.path, res.status);

            if (!req.headers.empty()) {
                spdlog::debug("Headers:");
                for (auto& header : req.headers) {
                    spdlog::debug("\t{}: {}", header.first, header.second);
                }
            }

            if (!req.params.empty()) {
                spdlog::debug("Params:");
                spdlog::debug("\t{}", httplib::detail::params_to_query_str(req.params));
            }
        });

        m_server->set_error_handler([](const httplib::Request& req, httplib::Response& res) {
            res.set_content(fmt::format("Hello, world!\r\n{} ({})", httplib::detail::status_message(res.status), res.status), "text/plain");
        });

        m_server->set_exception_handler([](const httplib::Request& req, httplib::Response& res, std::exception& ex) {
            res.status = 500;
            res.set_content(fmt::format("Hello, world!\r\n{}", ex.what()), "text/plain");
        });

        m_server->Post("/growtopia/server_data.php", [&](const httplib::Request& req, httplib::Response& res) {
            set_server_data_headers(req.headers);
            set_server_data_params(req.params);

            utils::TextParse text_parse{ request_server_data() }; // TODO: Handle crash.
            text_parse.set("server", "127.0.0.1");
            text_parse.set("port", "16999");

            res.set_content(text_parse.get_all_raw(), "text/html");
            return true;
        });

        m_server->listen_after_bind();
    }

    std::string Http::request_server_data()
    {
        if (m_config->m_server.host.find("growtopia") != std::string::npos) {
            m_config->m_server.host = "a104-125-3-135.deploy.static.akamaitechnologies.com";
            if (!m_config->m_ssl.enabled) {
                m_config->m_ssl.enabled = true;
            }
        }

        httplib::Client cli{ fmt::format("{}{}", m_config->m_ssl.enabled ? "https://" : "http://", m_config->m_server.host) };
        cli.enable_server_certificate_verification(false);

        httplib::Result response{ cli.Post("/growtopia/server_data.php", m_server_data_headers, m_server_data_params) };
        if (response.error() != httplib::Error::Success || response->status != 200) {
            if (response.error() == httplib::Error::Success) {
                spdlog::error("Failed to get server data. HTTP status code: {}", response->status);
            }
            else {
                spdlog::error("Failed to get server data. HTTP error: {}", httplib::to_string(response.error()));
            }

            return "";
        }

        return response->body;
    }
}
