#pragma once
#include <functional>
#include <memory>
#include <regex>
#include <string>
#include <unordered_map>

#include "packet_types.hpp"
#include "packet_helper.hpp"
#include "game/on_send_to_server.hpp"
#include "message/chat.hpp"
#include "message/exit.hpp"
#include "message/server_hello.hpp"
#include "../utils/singleton.hpp"

namespace packet {
using PacketFactory = std::function<std::shared_ptr<IPacket>()>;
using MessageEntry = std::pair<std::regex, PacketFactory>;

class PacketRegistry : public utils::Singleton<PacketRegistry> {
public:
    template<typename T>
    static PacketFactory make_factory()
    {
        static_assert(is_net_packet<T>::value || is_net_message<T>::value);
        return [] { return std::make_shared<T>(); };
    }

    template<typename T>
    void register_game()
    {
        static_assert(is_net_packet<T>::value);
        game_[T::PACKET_TYPE] = make_factory<T>();
    }

    template<typename T>
    void register_variant(std::string_view name)
    {
        static_assert(is_net_packet<T>::value);
        static_assert(T::PACKET_TYPE == PACKET_CALL_FUNCTION);
        variants_.emplace(name, make_factory<T>());
    }

    template<typename T>
    void register_core()
    {
        static_assert(is_net_message<T>::value);
        core_[T::MESSAGE_TYPE] = make_factory<T>();
    }

    template<typename T>
    void register_message(std::string_view pattern)
    {
        static_assert(is_net_message<T>::value);
        messages_.emplace_back(std::regex{ pattern.data() }, make_factory<T>());
    }

    std::shared_ptr<IPacket> create(PacketType type) const
    {
        if (auto it = game_.find(type); it != game_.end()) {
            return it->second();
        }

        return nullptr;
    }

    std::shared_ptr<IPacket> create(std::string_view function) const
    {
        if (auto it = variants_.find(std::string(function)); it != variants_.end()) {
            return it->second();
        }

        return nullptr;
    }

    std::shared_ptr<IPacket> create(NetMessageType event_type) const
    {
        if (auto it = core_.find(event_type); it != core_.end()) {
            return it->second();
        }

        return nullptr;
    }

    std::shared_ptr<IPacket> create(const std::string& msg) const
    {
        for (const auto& [re, factory] : messages_) {
            if (std::regex_match(msg, re)) {
                return factory();
            }
        }

        return nullptr;
    }

private:
    std::unordered_map<PacketType, PacketFactory> game_;
    std::unordered_map<std::string, PacketFactory> variants_;
    std::unordered_map<NetMessageType, PacketFactory> core_;
    std::vector<MessageEntry> messages_;
};

inline bool register_all_packets()
{
    static bool registered = false;
    if (registered) {
        return false;
    }

    registered = true;

    PacketRegistry::instance().register_core<message::ServerHello>();
    PacketRegistry::instance().register_game<game::Disconnect>();
    PacketRegistry::instance().register_variant<game::OnSendToServer>("OnSendToServer");
    PacketRegistry::instance().register_message<message::Log>(R"(^action\|log$)");
    PacketRegistry::instance().register_message<message::Quit>(R"(^action\|quit$)");
    PacketRegistry::instance().register_message<message::QuitToExit>(R"(^action\|quit_to_exit$)");
    PacketRegistry::instance().register_message<message::JoinRequest>(R"(^action\|join_request$)");
    PacketRegistry::instance().register_message<message::ValidateWorld>(R"(^action\|validate_world$)");
    return true;
}
}
