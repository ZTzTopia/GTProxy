#pragma once
#include "../packet/message/chat.hpp"
#include "../packet/packet_helper.hpp"
#include "../player/player.hpp"
#include "../core/logger.hpp"

namespace utils {
class PacketUtils {
public:
    static void send_chat_message(player::Player* player, const std::string& message) {
        if (!player || !player->is_connected()) {
            spdlog::error("Cannot send message: player is null or not connected.");
            return;
        }

        packet::message::Log message_packet{};
        message_packet.msg = message;

        if (!packet::PacketHelper::send(message_packet, *player)) {
            spdlog::error("Failed to send chat message packet to player.");
        }
    }
};
}