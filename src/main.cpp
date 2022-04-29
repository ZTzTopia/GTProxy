#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <httplib.h>

#include "config.h"
#include "enetwrapper/enetwrapper.h"
#include "server/server.h"
#include "utils/textparse.h"

int main() {
    {
        // Initialize logger.
        std::vector <spdlog::sink_ptr> sinks;
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>("proxy.log", 1024 * 1024 * 5, 15));

        auto logger = std::make_shared<spdlog::logger>("GTProxy", sinks.begin(), sinks.end());
        logger->set_pattern("[%n] [%^%l%$] %v");
        logger->set_level(spdlog::level::debug);
        logger->flush_on(spdlog::level::debug);

        spdlog::set_default_logger(logger);

        // Program logic.
        spdlog::info("Starting Growtopia proxy...");

        // Load config file.
        Config::get().load("config.json");

        // Initialize enet.
        if (!enetwrapper::ENetWrapper::one_time_init()) {
            spdlog::error("Failed to initialize ENet server.");
            return 1;
        }

        // Get meta from server_data.php.
        httplib::Client http_client{ Config::get().config()["server"]["host"] };
        httplib::Result response = http_client.Post("/growtopia/server_data.php");
        if (response.error() != httplib::Error::Success || response->status != 200) {
            spdlog::error("Failed to get server data. HTTP status code: {}", response->status);
            return 1;
        }

        utils::TextParse text_parse{response->body};
        std::string meta{text_parse.get("meta", 1)};

        // Start proxy server.
        auto proxy_server{std::make_unique<server::Server>()};
        if (!proxy_server->initialize()) {
            spdlog::error("Failed to initialize proxy server.");
            return 1;
        }

        // Start http server.
        httplib::Server http_server{};
        http_server.Post("/growtopia/server_data.php", [meta](const httplib::Request &req, httplib::Response &res) {
            if (!req.body.empty())
                spdlog::info("Request body from growtopia client: {}", req.body);

            res.set_content(fmt::format(
                    "server|127.0.0.1\n"
                    "port|17000\n"
                    "type|1\n"
                    "#maint|Server is under maintenance. We will be back online shortly. Thank you for your patience!\n"
                    "beta_server|beta.growtopiagame.com\n"
                    "beta_port|26999\n"
                    "beta_type|1\n"
                    "beta2_server|beta2.growtopiagame.com\n"
                    "beta2_port|26999\n"
                    "beta2_type|1\n"
                    "type2|1\n"
                    "meta|{}\n"
                    "RTENDMARKERBS1001", meta),
            "text/html");
            return true;
        });

        spdlog::info("HTTP Server listening to {}:{}", "0.0.0.0", 80);
        http_server.listen("0.0.0.0", 80);
    }
    return EXIT_SUCCESS;
}

/*
 * PACKET_STATE - 0
 * Variant.
 */

/*
 * PACKET_CALL_FUNCTION - 1
 * Variant.
 */

/*
 * PACKET_UPDATE_STATUS - 2
 * Unknown.
 */

/*
 * PACKET_TILE_CHANGE_REQUEST - 3
 * game update packet
 *  * 4 - net id (4 bytes)
 *  * 20 - item id (4 bytes)
 *  * 44 - x (4 bytes)
 *  * 48 - y (4 bytes)
 */

/*
 * PACKET_SEND_MAP_DATA - 4
 * Serialization
 *  * See World.
 */

/*
 * PACKET_SEND_TILE_UPDATE_DATA - 5
 * game update packet
 *  * 44 - x (4 bytes)
 *  * 48 - y (4 bytes)
 *
 * Serialization
 *  * See Tile.
 */

/*
 * PACKET_SEND_TILE_UPDATE_DATA_MULTIPLE - 6
 * game update packet
 *  * 44 - x (4 bytes)
 *  * 48 - y (4 bytes)
 *
 * Serialization
 *  * See Tile.
 */

/*
 * PACKET_TILE_ACTIVATE_REQUEST - 7 - Client
 * game update packet
 *  * 44 - x (4 bytes)
 *  * 48 - y (4 bytes)
 */

/*
 * PACKET_TILE_APPLY_DAMAGE - 8
 * game update packet
 *  * 4 - net id (4 bytes)
 *  * 20 - damage (4 bytes)
 *  * 44 - x (4 bytes)
 *  * 48 - y (4 bytes)
 */

/*
 * PACKET_SEND_INVENTORY_STATE - 9
 * game update packet
 *  * 4 - net id (4 bytes)
 *
 * Serialization
 *  * See PlayerItems.
 */

/*
 * PACKET_ITEM_ACTIVATE_REQUEST - 10
 */

/*struct PlayerItems {
    uint32_t max_size;
    uint32_t size;
    std::unordered_map<uint16_t, std::pair<uint8_t, uint8_t>> items; // item id, (count, unknown TODO!)
};

struct TileExtra {
    uint8_t type;
    union {
        uint16_t label_len;
    };
    union {
        std::string label;
        uint32_t owner;
        uint32_t heart_monitor;
    };
    union {
        uint32_t access_count;
    };
    std::vector<uint32_t> access;
};

struct Object {
    uint16_t id;
    CL_Vec2i pos;
    uint8_t amount;
    uint8_t flags; // TODO!
    uint32_t object_id;
};

struct Tile {
    uint16_t foreground;
    uint16_t background;
    uint16_t parent;
    uint16_t flags; // TODO!
    std::vector<TileExtra> tile_extras; // if flags is has extra tile.
};

struct World {
    uint16_t version;
    uint32_t pad; // TODO!
    uint16_t name_len;
    std::string name;
    uint32_t width;
    uint32_t height;
    uint32_t tile_count;
    std::vector<Tile> tiles; // if tile count is set to non-zero.
    uint32_t object_count;
    uint32_t current_object_id; // total_object_id - 1.
    std::vector<Object> objects; // if object_count is set to non-zero.
    uint32_t unk; // TODO!
    uint32_t unk2; // TODO!
};*/

/*
 * Object::Flag
 */

/*
 * Tile::Flag
 * 1 - Extra.
 * 2 - Locked.
 * 4 - Splicing.
 * 8 - Planting.
 */

/*
 * TileExtra::Type
 * 1 - Door.
 * 2 - Sign.
 * 3 - Lock.
 * 4 - Tree/Seed.
 * 6 - Mailbox.
 * 7 - Bulletin.
 * 9 - Provider.
 * 11 - Heart monitor.
 * 12 - Donation box.
 * 13 - Toy box.
 * 14 - Mannequin. Sign?
 * 19 - Dress up.
 * 20 - Crystal.
 * 21 - Burglar.
 * 24 - Vending machine.
 * 26 - Solar.
 * 31 - Tamagotchi.
 * 63 - Robot.
 */
