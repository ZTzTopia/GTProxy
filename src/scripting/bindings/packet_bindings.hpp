#pragma once
#include "../binding_module.hpp"
#include "../../network/client.hpp"
#include "../../network/server.hpp"
#include "../../packet/packet_helper.hpp"
#include "../../packet/generic_packets.hpp"
#include "../../packet/message/chat.hpp"
#include "../../packet/message/exit.hpp"
#include "../../packet/message/input.hpp"
#include "../../packet/message/server_hello.hpp"
#include "../../packet/game/server.hpp"
#include "../../packet/game/player.hpp"
#include "../../packet/game/world.hpp"
#include "../../packet/packet_variant.hpp"
#include "../../utils/text_parse.hpp"
#include "../../packet/packet_types.hpp"

#include <sol/sol.hpp>
#include <spdlog/spdlog.h>

namespace scripting::bindings {
class PacketBindings final : public IBindingModule {
public:
    explicit PacketBindings(network::Client& client, network::Server& server)
        : client_{ client }
        , server_{ server }
    {
    }

    [[nodiscard]] std::string_view name() const override { return "packet"; }

    void bind(sol::state& lua) override;

private:
    void bind_enums(sol::state& lua);
    void bind_text_parse(sol::state& lua);
    void bind_base_packet(sol::state& lua);
    void bind_message_packets(sol::state& lua);
    void bind_game_packets(sol::state& lua);
    void bind_generic_packets(sol::state& lua);
    void bind_game_update_packet(sol::state& lua);
    void bind_packet_variant(sol::state& lua);
    void bind_send_functions(sol::state& lua);
    bool send_to_direction(const std::vector<std::byte>& data, const bool to_server);
    bool send_text_packet(
        const std::string& text,
        const bool to_server,
        const sol::optional<int> msg_type_opt
    );

    network::Client& client_;
    network::Server& server_;
};
}
