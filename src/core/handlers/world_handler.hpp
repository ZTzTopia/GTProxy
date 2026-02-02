#pragma once
#include <vector>
#include <algorithm>

#include <glm/glm.hpp>

#include "../../event/event.hpp"
#include "../../packet/game/world.hpp"
#include "../../world/world.hpp"
#include "../../world/object.hpp"
#include "../../world/tile.hpp"
#include "../../player/player.hpp"
#include "../../utils/byte_stream.hpp"

namespace core {

class WorldHandler {
public:
    explicit WorldHandler(event::Dispatcher& dispatcher)
        : dispatcher_{ dispatcher }
    {
        setup_join_request_handler();
        setup_on_spawn_handler();
        setup_on_remove_handler();
        setup_send_map_data_handler();
        setup_send_tile_update_data_handler();
        setup_tile_change_request_handler();
        setup_item_change_object_handler();
    }

private:
    void setup_join_request_handler()
    {
        constexpr auto join_request_type{ event::packet_event_type(packet::PacketId::JoinRequest) };
        handles_.emplace_back(
            dispatcher_,
            join_request_type,
            dispatcher_.appendListener(join_request_type, [](const event::Event& event) {
                if (
                    const auto evt{ dynamic_cast<const event::TypedPacketEvent<packet::PacketId::JoinRequest>*>(&event) };
                    !evt || evt->direction != event::Direction::ServerBound
                ) {
                    return;
                }

                world::World::instance().clear();
            })
        );
    }

    void setup_on_spawn_handler()
    {
        constexpr auto on_spawn_type{ event::packet_event_type(packet::PacketId::OnSpawn) };
        handles_.emplace_back(
            dispatcher_,
            on_spawn_type,
            dispatcher_.appendListener(on_spawn_type, [](const event::Event& event) {
                const auto evt{ dynamic_cast<const event::TypedPacketEvent<packet::PacketId::OnSpawn>*>(&event) };
                if (!evt || evt->direction != event::Direction::ClientBound) {
                    return;
                }

                const auto pkt{ evt->get<packet::game::OnSpawn>() };
                if (!pkt) {
                    return;
                }

                const auto player{ player::Player::from_on_spawn(*pkt) };
                world::World::instance().add_player(player);
            })
        );
    }

    void setup_on_remove_handler()
    {
        constexpr auto on_remove_type{ event::packet_event_type(packet::PacketId::OnRemove) };
        handles_.emplace_back(
            dispatcher_,
            on_remove_type,
            dispatcher_.appendListener(on_remove_type, [](const event::Event& event) {
                const auto evt{ dynamic_cast<const event::TypedPacketEvent<packet::PacketId::OnRemove>*>(&event) };
                if (!evt || evt->direction != event::Direction::ClientBound) {
                    return;
                }

                const auto pkt{ evt->get<packet::game::OnRemove>() };
                if (!pkt) {
                    return;
                }

                world::World::instance().remove_player(pkt->net_id);
            })
        );
    }

    void setup_send_map_data_handler()
    {
        constexpr auto send_map_data_type{ event::packet_event_type(packet::PacketId::SendMapData) };
        handles_.emplace_back(
            dispatcher_,
            send_map_data_type,
            dispatcher_.appendListener(send_map_data_type, [](const event::Event& event) {
                const auto evt{ dynamic_cast<const event::TypedPacketEvent<packet::PacketId::SendMapData>*>(&event) };
                if (!evt || evt->direction != event::Direction::ClientBound) {
                    return;
                }

                const auto pkt{ evt->get<packet::game::SendMapData>() };
                if (!pkt || pkt->extra.empty()) {
                    return;
                }

                world::World::instance().serialize(reinterpret_cast<const uint8_t*>(pkt->extra.data()));
            })
        );
    }

    void setup_send_tile_update_data_handler()
    {
        constexpr auto send_tile_update_data_type{ event::packet_event_type(packet::PacketId::SendTileUpdateData) };
        handles_.emplace_back(
            dispatcher_,
            send_tile_update_data_type,
            dispatcher_.appendListener(send_tile_update_data_type, [](const event::Event& event) {
                const auto evt{ dynamic_cast<const event::TypedPacketEvent<packet::PacketId::SendTileUpdateData>*>(&event) };
                if (!evt || evt->direction != event::Direction::ClientBound) {
                    return;
                }

                const auto pkt{ evt->get<packet::game::SendTileUpdateData>() };
                if (!pkt || pkt->extra.empty()) {
                    return;
                }

                auto& world{ world::World::instance() };
                world::Tile tile{};
                
                utils::ByteStream<> bs{ pkt->extra.data(), pkt->extra.size() };
                tile.serialize(bs, world.get_version());

                // Update tile in map if coordinates are available in game_packet
                // Note: SendTileUpdateData typically doesn't include coords in the packet itself
                // The coordinates would need to come from parsing the extended data
            })
        );
    }

    void setup_tile_change_request_handler()
    {
        constexpr auto tile_change_request_type{ event::packet_event_type(packet::PacketId::TileChangeRequest) };
        handles_.emplace_back(
            dispatcher_,
            tile_change_request_type,
            dispatcher_.appendListener(tile_change_request_type, [](const event::Event& event) {
                const auto evt{ dynamic_cast<const event::TypedPacketEvent<packet::PacketId::TileChangeRequest>*>(&event) };
                if (!evt) {
                    return;
                }

                const auto pkt{ evt->get<packet::game::TileChangeRequest>() };
                if (!pkt) {
                    return;
                }

                auto& world{ world::World::instance() };
                auto& tile_map{ world.get_tile_map() };
                
                const uint32_t index{ static_cast<uint32_t>(pkt->int_x + pkt->int_y * tile_map.get_size().x) };
                
                if (index >= tile_map.get_tiles().size()) {
                    return;
                }

                // Fist punch (item_id == 18) - toggle visibility
                if (pkt->item_id == 18) {
                    auto& tile{ tile_map.get_tiles()[index] };
                    if (tile.foreground != 0) {
                        tile.foreground = 0;
                    } else {
                        tile.background = 0;
                    }
                }
                // Note: Block placement requires ItemDatabase (ignored per instructions)
            })
        );
    }

    void setup_item_change_object_handler()
    {
        constexpr auto item_change_object_type{ event::packet_event_type(packet::PacketId::ItemChangeObject) };
        handles_.emplace_back(
            dispatcher_,
            item_change_object_type,
            dispatcher_.appendListener(item_change_object_type, [](const event::Event& event) {
                const auto evt{ dynamic_cast<const event::TypedPacketEvent<packet::PacketId::ItemChangeObject>*>(&event) };
                if (!evt || evt->direction != event::Direction::ClientBound) {
                    return;
                }

                const auto pkt{ evt->get<packet::game::ItemChangeObject>() };
                if (!pkt) {
                    return;
                }

                auto& world{ world::World::instance() };
                auto& object_map{ world.get_object_map() };

                // Spawn new object
                if (pkt->object_change_type == -1) {
                    object_map.increment_drop_id();
                    
                    world::Object obj{};
                    obj.pos = glm::vec2{ pkt->pos_x, pkt->pos_y };
                    obj.item_id = static_cast<uint16_t>(pkt->item_id);
                    obj.amount = pkt->amount;
                    obj.object_id = object_map.get_drop_id();
                    
                    object_map.get_objects().push_back(obj);
                }
                // Update existing object position
                else if (pkt->object_change_type == -3) {
                    for (auto& obj : object_map.get_objects()) {
                        if (obj.object_id == pkt->item_net_id) {
                            obj.pos = glm::vec2{ pkt->pos_x, pkt->pos_y };
                            obj.item_id = static_cast<uint16_t>(pkt->item_id);
                            obj.amount = pkt->amount;
                            break;
                        }
                    }
                }
                // Remove object (collected by player or despawned)
                else {
                    auto it = std::remove_if(
                        object_map.get_objects().begin(),
                        object_map.get_objects().end(),
                        [object_id = pkt->item_net_id](const world::Object& obj) {
                            return obj.object_id == object_id;
                        }
                    );
                    
                    if (it != object_map.get_objects().end()) {
                        object_map.get_objects().erase(it, object_map.get_objects().end());
                    }
                    
                    // Note: Inventory updates are ignored per instructions
                }
            })
        );
    }

private:
    event::Dispatcher& dispatcher_;
    std::vector<event::ScopedHandle> handles_;
};

}
