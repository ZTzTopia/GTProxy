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
            if (!req.params.empty()) {
                spdlog::info("  {}", httplib::detail::params_to_query_str(req.params));
            }
        });

        m_server->set_error_handler([](const httplib::Request& req, httplib::Response& res) {
            res.set_content("Hello, world!", "text/plain");
        });

        m_server->Post("/growtopia/server_data.php", [&](const httplib::Request& req, httplib::Response& res) {
            httplib::Headers headers;
            headers.emplace("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/104.0.5110.0 Safari/537.36");

            httplib::Client cli{ m_config->m_server.host };
            httplib::Result response{ cli.Post("/growtopia/server_data.php", headers, req.params) };

            if (response.error() != httplib::Error::Success || response->status != 200) {
                if (response.error() == httplib::Error::Success)
                    spdlog::error("Failed to get server data. HTTP status code: {}", response->status);
                else
                    spdlog::error("Failed to get server data. HTTP error: {}", httplib::to_string(response.error()));

                return false;
            }

            utils::TextParse text_parse{ response->body };
            text_parse.set("server", "127.0.0.1");
            text_parse.set("port", "17000");

            res.set_content(text_parse.get_all_raw(), "text/html");
            return true;
        });

        m_server->listen_after_bind();
    }
}
