#include "connection_handler.hpp"

#include "../../packet/game/server.hpp"
#include "../../packet/game/item_database.hpp"

namespace core::handlers {
ConnectionHandler::ConnectionHandler(
    event::Dispatcher& dispatcher,
    network::Client& client,
    network::Server& server,
    Config& config
)
    : dispatcher_{ dispatcher }
    , client_{ client }
    , server_{ server }
    , config_{ config }
{
    setup_connection_handlers();
    setup_on_send_to_server_handler();
    setup_quit_handler();
    setup_disconnect_handler();
    setup_send_item_database_data_handler();
    setup_on_super_main_start_handler();
}

void ConnectionHandler::setup_connection_handlers()
{
    handles_.emplace_back(
        dispatcher_,
        event::Type::ClientConnect,
        dispatcher_.appendListener(event::Type::ClientConnect, [this](const event::Event&) {
            if (pending_port_ == 65535) {
                return;
            }

            spdlog::info("Connecting to Growtopia server at {}:{}", pending_address_, pending_port_);
            client_.connect(pending_address_, pending_port_);

            pending_address_.clear();
            pending_port_ = 65535;
        })
    );

    handles_.emplace_back(
        dispatcher_,
        event::Type::ClientDisconnect,
        dispatcher_.appendListener(event::Type::ClientDisconnect, [this](const event::Event&) {
            if (!client_.is_connected()) {
                return;
            }

            client_.disconnect();
            spdlog::info("Gracefully disconnect Growtopia client from proxy server");
        })
    );

    handles_.emplace_back(
        dispatcher_,
        event::Type::ServerDisconnect,
        dispatcher_.appendListener(event::Type::ServerDisconnect, [this](const event::Event&) {
            if (!server_.is_connected()) {
                return;
            }

            server_.disconnect();
            spdlog::info("Gracefully disconnect Growtopia client from proxy server");
        })
    );
}

void ConnectionHandler::setup_on_send_to_server_handler()
{
    constexpr auto on_send_to_server_type = event::packet_event_type(packet::PacketId::OnSendToServer);
    handles_.emplace_back(
        dispatcher_,
        on_send_to_server_type,
        dispatcher_.appendListener(on_send_to_server_type, [this](const event::Event& event) {
            const auto* evt = dynamic_cast<const event::TypedPacketEvent<packet::PacketId::OnSendToServer>*>(&event);
            if (!evt) {
                return;
            }

            const auto pkt{ evt->get<packet::game::OnSendToServer>() };
            if (!pkt) {
                return;
            }

            pending_address_ = pkt->address;
            pending_port_ = pkt->port;

            const auto modified_pkt = std::make_shared<packet::game::OnSendToServer>(*pkt);
            modified_pkt->address = "127.0.0.1";
            modified_pkt->port = config_.get_server_config().port;

            std::ignore = packet::PacketHelper::write(*modified_pkt, server_);
            evt->cancel();
        })
    );
}

void ConnectionHandler::setup_quit_handler()
{
    constexpr auto quit_type{ event::packet_event_type(packet::PacketId::Quit) };
    handles_.emplace_back(
        dispatcher_,
        quit_type,
        dispatcher_.appendListener(quit_type, [this](const event::Event& event) {
            if (
                const auto evt{ dynamic_cast<const event::TypedPacketEvent<packet::PacketId::Quit>*>(&event) };
                !evt || evt->direction != event::Direction::ServerBound
            ) {
                return;
            }

            server_.disconnect();
            client_.disconnect_now();
            spdlog::info("Forced disconnect proxy client from Growtopia server");
        })
    );
}

void ConnectionHandler::setup_disconnect_handler()
{
    constexpr auto disconnect_type{ event::packet_event_type(packet::PacketId::Disconnect) };
    handles_.emplace_back(
        dispatcher_,
        disconnect_type,
        dispatcher_.appendListener(disconnect_type, [this](const event::Event& event) {
            if (
                const auto evt{ dynamic_cast<const event::TypedPacketEvent<packet::PacketId::Disconnect>*>(&event) };
                !evt || evt->direction != event::Direction::ServerBound
            ) {
                return;
            }

            server_.disconnect_now();
            spdlog::info("Forced disconnect proxy server from Growtopia client");
            client_.disconnect_now();
            spdlog::info("Forced disconnect proxy client from Growtopia server");
        })
    );
}

void ConnectionHandler::setup_send_item_database_data_handler()
{
    constexpr auto send_item_database_data_type{ event::packet_event_type(packet::PacketId::SendItemDatabaseData) };
    handles_.emplace_back(
        dispatcher_,
        send_item_database_data_type,
        dispatcher_.appendListener(send_item_database_data_type, [](const event::Event& event) {
            const auto evt{ dynamic_cast<const event::TypedPacketEvent<packet::PacketId::SendItemDatabaseData>*>(&event) };
            if (!evt || evt->direction != event::Direction::ClientBound) {
                return;
            }

            const auto pkt{ evt->get<packet::game::SendItemDatabaseData>() };
            if (!pkt || pkt->items_dat.empty()) {
                spdlog::warn("No data to parse");
                return;
            }

            const auto parsed{ item::ItemDatabase::instance().parse(pkt->items_dat) };
            if (!parsed) {
                spdlog::error(
                    "Failed to parse items.dat (version {}, count {})",
                    item::ItemDatabase::instance().get_version(),
                    item::ItemDatabase::instance().get_count()
                );
                return;
            }

            spdlog::info(
                "Successfully parsed items.dat (version {}, count {})",
                item::ItemDatabase::instance().get_version(),
                item::ItemDatabase::instance().get_count()
            );

            // ReSharper disable once CppVariableCanBeMadeConstexpr
            const std::string cache_path{ "resources/items.dat" };
            try {
                std::ofstream out{ cache_path, std::ios::binary };
                if (!out) {
                    spdlog::warn("Failed to open items.dat for writing: {}", cache_path);
                    return;
                }

                out.write(reinterpret_cast<const char*>(pkt->items_dat.data()), pkt->items_dat.size());
                spdlog::info("Saved items.dat to {} ({} bytes)", cache_path, pkt->items_dat.size());
            } catch (const std::exception& e) {
                spdlog::error("Failed to save items.dat: {}", e.what());
            }
        })
    );
}

void ConnectionHandler::setup_on_super_main_start_handler()
{
    constexpr auto on_send_to_server_type = event::packet_event_type(packet::PacketId::OnSuperMainStartAcceptLogonHrdxs47254722215a);
    handles_.emplace_back(
        dispatcher_,
        on_send_to_server_type,
        dispatcher_.appendListener(on_send_to_server_type, [](const event::Event& event) {
            const auto* evt = dynamic_cast<const event::TypedPacketEvent<packet::PacketId::OnSuperMainStartAcceptLogonHrdxs47254722215a>*>(&event);
            if (!evt) {
                return;
            }

            const auto pkt{ evt->get<packet::game::OnSuperMainStartAcceptLogonHrdxs47254722215a>() };
            if (!pkt) {
                return;
            }

            // ReSharper disable once CppVariableCanBeMadeConstexpr
            const std::string cache_path{ "resources/items.dat" };
            const auto server_hash{ static_cast<std::uint32_t>(pkt->item_hash) };
            const auto cached_hash{ (utils::hash::proton_file(cache_path)) };

            if (server_hash != cached_hash) {
                spdlog::info("Hash mismatch: server={}, cached={}", server_hash, cached_hash);
                return;
            }

            spdlog::info("Hash matches, loading cached items.dat (hash: {})", cached_hash);
            if (!item::ItemDatabase::instance().load_from_file(cache_path)) {
                spdlog::warn("Failed to load cached items.dat");
                return;
            }

            spdlog::info(
                "Successfully loaded cached items.dat (version {}, count {})",
                item::ItemDatabase::instance().get_version(),
                item::ItemDatabase::instance().get_count()
            );
        })
    );
}

} // namespace core
