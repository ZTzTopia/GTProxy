#include <thread>
#include <spdlog/spdlog.h>

#include "http.h"
#include "../utils/text_parse.h"

namespace server {
    Http::Http(Config* config) : m_config{ config }
    {
        std::ofstream cert_file{ "./cert.pem" };
        cert_file << cert;
        cert_file.close();

        std::ofstream key_file{ "./key.pem" };
        key_file << key;
        key_file.close();

        m_server = new httplib::SSLServer{ "./cert.pem", "./key.pem" };

        std::remove("./cert.pem");
        std::remove("./key.pem");
    }

    Http::~Http() {
        stop();
        delete m_server;
    }

    void Http::bind_to_port(const std::string& host, int port)
    {
        m_server->bind_to_port(host.c_str(), port);
    }

    void Http::listen_after_bind()
    {
        std::thread{ &Http::listen_internal, this }.detach();
    }

    void Http::listen(const std::string& host, int port)
    {
        bind_to_port(host, port);
        listen_after_bind();
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
                spdlog::info("Headers:");
                for (auto& header : req.headers) {
                    spdlog::info("\t{}: {}", header.first, header.second);
                }
            }

            if (!req.params.empty()) {
                spdlog::info("Params:");
                spdlog::info("\t{}", httplib::detail::params_to_query_str(req.params));
            }
        });

        m_server->set_error_handler([](const httplib::Request& req, httplib::Response& res) {
            res.set_content("Hello, world!", "text/plain");
        });

        m_server->Post("/growtopia/server_data.php", [&](const httplib::Request& req, httplib::Response& res) {
            set_server_data_headers(req.headers);
            set_server_data_params(req.params);

            utils::TextParse text_parse{ request_server_data() }; // TODO: Handle crash.
            text_parse.set("server", "127.0.0.1");
            text_parse.set("port", "17000");

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
            if (response.error() == httplib::Error::Success)
                spdlog::error("Failed to get server data. HTTP status code: {}", response->status);
            else
                spdlog::error("Failed to get server data. HTTP error: {}", httplib::to_string(response.error()));

            return "";
        }

        return response->body;
    }
}
