#pragma once
#include <string>
#include <httplib/httplib.h>

#include "../config.h"

namespace server {
    inline std::string cert{ "-----BEGIN CERTIFICATE-----\n"
                             "MIIDXjCCAkagAwIBAgIUD23uIGJWhzrYV3IGWD10XjkFDwcwDQYJKoZIhvcNAQEL\n"
                             "BQAwFTETMBEGA1UEAwwKenR6cHJvZGVrajAeFw0yMjA2MTUxMTAwMTdaFw0yMzA2\n"
                             "MDYxMTAwMTdaMBUxEzARBgNVBAMMCnp0enByb2Rla2owggEiMA0GCSqGSIb3DQEB\n"
                             "AQUAA4IBDwAwggEKAoIBAQDAj6WWsIYJ1iK4G5m06JU4Ut3CpkZXYrH/VsiigfK5\n"
                             "Aqe9zmxLk4vXq1E4qOM5H36bKLYsSEPFP9GOg2Buq36SBj77/HCwpUYzN2VK4IRO\n"
                             "XhE/6XwEWGH83Ci6pWWXo4UAlXY+I5Ze6XUds1qj3UW3kc7J60saG+OZZ1Gn2mPA\n"
                             "DmqI8twcEI1mDVlU5SbbM7TAnWdbhQpX4vlMrzEDQOskBqkfpYzl3g6GzVJn7opF\n"
                             "FDCn/G/OOJ2GshRiFR7b4WsOABKC40ozi96PW6iDxrKRPQLxgx8UtDhxUQyYFOaB\n"
                             "tRT2NZiUu18SlZEH7skMw+mv7cXfj+GXubCpoCMtaNZPAgMBAAGjgaUwgaIwHQYD\n"
                             "VR0OBBYEFHADvOTQ1tQiXvE6ebKOEOF8x6AzMB8GA1UdIwQYMBaAFHADvOTQ1tQi\n"
                             "XvE6ebKOEOF8x6AzMA8GA1UdEwEB/wQFMAMBAf8wLQYDVR0RBCYwJIIQKi5ncm93\n"
                             "dG9waWExLmNvbYIQKi5ncm93dG9waWEyLmNvbTALBgNVHQ8EBAMCB4AwEwYDVR0l\n"
                             "BAwwCgYIKwYBBQUHAwEwDQYJKoZIhvcNAQELBQADggEBAAfEBUrGwt5HF8Y9BTQa\n"
                             "dMOy9etcxsMSCUbmxQ2thAfk4BRh3XLDToNBqYDSOfNWhXqagVjuiHp4xXrPtU2W\n"
                             "Oj9AVs8gROwhOJ0RUxc/FMW7BVS61JzJqTiu1DN4a+UnrMCOwYUUUbe5se6LTQBs\n"
                             "owtfLYkDXN/xOOWTG+FMCdC7NqF6LHEPeOw64y7zZWGPx18Lbp6ESLbtTgQeSb/4\n"
                             "b5mHXCnhcStNM+gMaVhIju5y3h0XEEu6qyDcEJxXm1sAqyooRYV9vEYIyv2Jk7ON\n"
                             "eiHFPRaY7q+2QFuUhPV/Y95IqqLENuvjeFXp+6zXjtqc+TZxf2C0KCC/D8lxyeg/\n"
                             "N4A=\n"
                             "-----END CERTIFICATE-----\n" };

    inline std::string key{ "-----BEGIN PRIVATE KEY-----\n"
                            "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDAj6WWsIYJ1iK4\n"
                            "G5m06JU4Ut3CpkZXYrH/VsiigfK5Aqe9zmxLk4vXq1E4qOM5H36bKLYsSEPFP9GO\n"
                            "g2Buq36SBj77/HCwpUYzN2VK4IROXhE/6XwEWGH83Ci6pWWXo4UAlXY+I5Ze6XUd\n"
                            "s1qj3UW3kc7J60saG+OZZ1Gn2mPADmqI8twcEI1mDVlU5SbbM7TAnWdbhQpX4vlM\n"
                            "rzEDQOskBqkfpYzl3g6GzVJn7opFFDCn/G/OOJ2GshRiFR7b4WsOABKC40ozi96P\n"
                            "W6iDxrKRPQLxgx8UtDhxUQyYFOaBtRT2NZiUu18SlZEH7skMw+mv7cXfj+GXubCp\n"
                            "oCMtaNZPAgMBAAECggEACoQNu/P57fGWxc0kEMAe3guQdr/T2ZPXOqWHoXBNTBM8\n"
                            "C96JBXuSSH3fmqOGfSkeJeUMRdDQeqYokIrWKlUNPXY/3E0F7m+oLMXar0MhlpIG\n"
                            "iSH8FtSozUFMrBy6NpTEf6qp4WzaGmbPcYbI7Wf5FbRNwbMqz8s0G72LtQn1JpjK\n"
                            "km1whmFEN9OTDikLLBzp/AAxXkIC+8HytH5x0DMtUnHJbNLry2p/oOyzWO/JgB0C\n"
                            "7+3FDd6ddJnzTn2J3bPP+Nnb8B0MRaThvP6gNfFP94hoSTe/onfEMdnSd16IX4Ga\n"
                            "nyNv8oI08EH4+OVpAeyUz+BDqqZNjgFGLA/C8dqWKQKBgQD1yviPqbnlWMrjJecW\n"
                            "M3N39FccR6KtNtTDi6AVBQLPzo7w92EwJSCaogf2j7//bgOIABlJYIecKMIF6YD7\n"
                            "FpuzN296PHeXml3Ax90uFVt+ekDTmCeTR08Z4PVDvuhajmrbDbm6m+1z6STRAJGm\n"
                            "AyC9Bn3GuAVLtHYw9RPhHe4TiwKBgQDIjsSn328l3RB4xw39+aUMvKsxgoLtfWdN\n"
                            "HL9nn8EcATMPvGCkr7IBRmIzdPZcfzEOuyY+s9AY0BFlNBDJxUubUokZdQ7l1a6g\n"
                            "HN90vWuRTWQzPQenZZQjPZlZCStrsDKvKVsZ2FHI8CcPOFVY3+TtiogibJGCVqay\n"
                            "LiadpO+QzQKBgQCynl/ntwX6334BeGfowFcnUw+C46QakIAp4uvgmpWigo9qGbwt\n"
                            "Cq4Y8asryOdULpSuXrQBmP6zwwLM3RX4YkOgB6chg5O9PlbnY9ceSDMHRLybiDUq\n"
                            "WGXpfot+QdwFAv4wVlckf3AeDc3NfMZbiGZgN7lrkgt3KpvBlDhwHhoHZQKBgQCP\n"
                            "noU/PeanM6Y/BzSC4koKU/U38CD4Fqxp8SMz/pfYCRfatehzJooPFaru9FwTotWr\n"
                            "meNqVXO4wQ8j7OP0yX86DCG3hDcV6S4y5Fo0jAzCsawGcTbQ7hHmJo9wzfs1E6lH\n"
                            "/BKL8HeosCkYYhvkF/klDeYs2JhewNSAlkH69AjGJQKBgHHlj5i3zs/knX1CRah8\n"
                            "1IzQ4NjqdJjzbQ7VPO8AjEt4RRNKEVBZDXD8t22oRAD3/YlOfgPNA8CUtaoMZXYt\n"
                            "9bvQHpmlzzNaUpTyOXGqgI4RR+2o7aIlSBpKzvTgHhp2Cuzv1zWL7TshB06JSQuZ\n"
                            "73whfRg+s50OK40A4ruOVniX\n"
                            "-----END PRIVATE KEY-----\n" };

    class Http {
    public:
        explicit Http(Config* config);
        ~Http();

        void bind_to_port(const std::string& host, int port);
        void listen_after_bind();
        void listen(const std::string& host, int port);
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
