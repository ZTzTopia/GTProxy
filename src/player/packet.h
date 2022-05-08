#pragma once
#include <cstdint>
#include <cassert>
#include <memory>
#include <enet/enet.h>
#include <spdlog/spdlog.h>

namespace player {
    enum eNetMessageType {
        NET_MESSAGE_UNKNOWN = 0,
        NET_MESSAGE_SERVER_HELLO,
        NET_MESSAGE_GENERIC_TEXT,
        NET_MESSAGE_GAME_MESSAGE,
        NET_MESSAGE_GAME_PACKET,
        NET_MESSAGE_ERROR,
        NET_MESSAGE_TRACK,
        NET_MESSAGE_CLIENT_LOG_REQUEST,
        NET_MESSAGE_CLIENT_LOG_RESPONSE,
        NET_MESSAGE_MAX
    };

    enum ePacketType {
        PACKET_STATE = 0,
        PACKET_CALL_FUNCTION,
        PACKET_UPDATE_STATUS,
        PACKET_TILE_CHANGE_REQUEST,
        PACKET_SEND_MAP_DATA,
        PACKET_SEND_TILE_UPDATE_DATA,
        PACKET_SEND_TILE_UPDATE_DATA_MULTIPLE,
        PACKET_TILE_ACTIVATE_REQUEST,
        PACKET_TILE_APPLY_DAMAGE,
        PACKET_SEND_INVENTORY_STATE,
        PACKET_ITEM_ACTIVATE_REQUEST,
        PACKET_ITEM_ACTIVATE_OBJECT_REQUEST,
        PACKET_SEND_TILE_TREE_STATE,
        PACKET_MODIFY_ITEM_INVENTORY,
        PACKET_ITEM_CHANGE_OBJECT,
        PACKET_SEND_LOCK,
        PACKET_SEND_ITEM_DATABASE_DATA,
        PACKET_SEND_PARTICLE_EFFECT,
        PACKET_SET_ICON_STATE,
        PACKET_ITEM_EFFECT,
        PACKET_SET_CHARACTER_STATE,
        PACKET_PING_REPLY,
        PACKET_PING_REQUEST,
        PACKET_GOT_PUNCHED,
        PACKET_APP_CHECK_RESPONSE,
        PACKET_APP_INTEGRITY_FAIL,
        PACKET_DISCONNECT,
        PACKET_BATTLE_JOIN,
        PACKET_BATTLE_EVENT,
        PACKET_USE_DOOR,
        PACKET_SEND_PARENTAL,
        PACKET_GONE_FISHIN,
        PACKET_STEAM,
        PACKET_PET_BATTLE,
        PACKET_NPC,
        PACKET_SPECIAL,
        PACKET_SEND_PARTICLE_EFFECT_V2,
        PACKET_ACTIVE_ARROW_TO_ITEM,
        PACKET_SELECT_TILE_INDEX,
        PACKET_SEND_PLAYER_TRIBUTE_DATA,
        PACKET_PVE_UNK1,
        PACKET_PVE_UNK2,
        PACKET_PVE_UNK3,
        PACKET_PVE_UNK4,
        PACKET_PVE_UNK5,
        PACKET_SET_EXTRA_MODS,
        PACKET_ON_STEP_ON_TILE_MOD,
        PACKET_MAX
    };

    enum ePacketFlag {
        PACKET_FLAG_NONE = 0,
        PACKET_FLAG_UNK = (1 << 1),
        PACKET_FLAG_RESET_VISUAL_STATE = (1 << 2), // 4
        PACKET_FLAG_EXTENDED = (1 << 3), // 8
        PACKET_FLAG_ROTATE_LEFT = (1 << 4), // 16
        PACKET_FLAG_ON_SOLID = (1 << 5), // 32
        PACKET_FLAG_ON_FIRE_DAMAGE = (1 << 6), // 64
        PACKET_FLAG_ON_JUMP = (1 << 7), // 128
        PACKET_FLAG_ON_KILLED = (1 << 8), // 256
        PACKET_FLAG_ON_PUNCHED = (1 << 9), // 512
        PACKET_FLAG_ON_PLACED = (1 << 10), // 1024
        PACKET_FLAG_ON_TILE_ACTION = (1 << 11), // 2048
        PACKET_FLAG_ON_GOT_PUNCHED = (1 << 12), // 4096
        PACKET_FLAG_ON_RESPAWNED = (1 << 13), // 8192
        PACKET_FLAG_ON_COLLECT_OBJECT = (1 << 14), // 16384
        PACKET_FLAG_ON_TRAMPOLINE = (1 << 15), // 32768
        PACKET_FLAG_ON_DAMAGE = (1 << 16), // 65536
        PACKET_FLAG_ON_SLIDE = (1 << 17), // 131072
        PACKET_FLAG_ON_WALL_HANG = (1 << 21), // 2097152
        PACKET_FLAG_ON_ACID_DAMAGE = (1 << 26), // 67108864
        PACKET_FLAG_MAX = 31,
    };

#pragma pack(push, 1)
    struct GameUpdatePacket { //thanks to Inzernal project
        uint8_t type{};
        union {
            uint8_t object_type{};
            uint8_t punch_id;
            uint8_t npc_type;
        };
        union {
            uint8_t count_1{};
            uint8_t jump_count;
            uint8_t build_range;
            uint8_t npc_id;
            uint8_t lost_item_count;
        };
        union {
            uint8_t count_2{};
            uint8_t animation_type;
            uint8_t punch_range;
            uint8_t npc_action;
            uint8_t particle_id;
            uint8_t gained_item_count;
            uint8_t dice_result;
            uint8_t fruit_count;
        };
        union {
            int32_t net_id{};
            int32_t effect_flags_check;
            int32_t object_change_type;
            int32_t particle_emitter_id;
        };
        union {
            int32_t item{};
            int32_t ping_hash;
            int32_t item_net_id;
            int32_t pupil_color;
            int32_t tiles_length;
        };
        int32_t flags{};
        union {
            float float_var{};
            float water_speed;
            float obj_alt_count;
        };
        union {
            int32_t int_data{};
            int32_t ping_item;
            int32_t elapsed_ms;
            int32_t delay;
            int32_t tile_damage;
            int32_t item_id;
            int32_t item_speed;
            int32_t effect_flags;
            int32_t object_id;
            int32_t hash;
            int32_t verify_pos;
            int32_t client_hack_type;
        };
        union {
            float vec_x{};
            float pos_x;
            float accel;
            float punch_range_in;
        };
        union {
            float vec_y{};
            float pos_y;
            float build_range_in;
            float punch_strength;
        };
        union {
            float vec2_x{};
            float dest_x;
            float gravity_in;
            float speed_out;
            float velocity_x;
            float particle_variable;
            float pos2_x;
            int hack_type;
        };
        union {
            float vec2_y{};
            float dest_y;
            float speed_in;
            float gravity_out;
            float velocity_y;
            float particle_alt_id;
            float pos2_y;
            int hack_type2;
        };
        union {
            float particle_rotation{};
            float npc_variable;
        };
        union {
            uint32_t int_x{};
            uint32_t item_id_alt;
            uint32_t eye_shade_color;
        };
        union {
            uint32_t int_y{};
            uint32_t item_count;
            uint32_t eyecolor;
            uint32_t npc_speed;
            uint32_t particle_size_alt;
        };
        uint32_t data_size;
    };
#pragma pack(pop)
    static_assert((sizeof(GameUpdatePacket) == 56) && "Invalid GameUpdatePacket size.");

    inline std::string message_type_to_string(uint8_t type) {
        std::array<std::string, NET_MESSAGE_MAX> types{
            "NET_MESSAGE_UNKNOWN", "NET_MESSAGE_SERVER_HELLO", "NET_MESSAGE_GENERIC_TEXT", "NET_MESSAGE_GAME_MESSAGE", "NET_MESSAGE_GAME_PACKET",
            "NET_MESSAGE_ERROR", "NET_MESSAGE_TRACK", "NET_MESSAGE_CLIENT_LOG_REQUEST", "NET_MESSAGE_CLIENT_LOG_RESPONSE" };

        return types[type];
    }

    inline std::string packet_type_to_string(uint8_t type) {
        std::array<std::string, PACKET_MAX> types{
            "PACKET_STATE", "PACKET_CALL_FUNCTION", "PACKET_UPDATE_STATUS", "PACKET_TILE_CHANGE_REQUEST", "PACKET_SEND_MAP_DATA",
            "PACKET_SEND_TILE_UPDATE_DATA", "PACKET_SEND_TILE_UPDATE_DATA_MULTIPLE", "PACKET_TILE_ACTIVATE_REQUEST", "PACKET_TILE_APPLY_DAMAGE",
            "PACKET_SEND_INVENTORY_STATE", "PACKET_ITEM_ACTIVATE_REQUEST", "PACKET_ITEM_ACTIVATE_OBJECT_REQUEST", "PACKET_SEND_TILE_TREE_STATE",
            "PACKET_MODIFY_ITEM_INVENTORY", "PACKET_ITEM_CHANGE_OBJECT", "PACKET_SEND_LOCK", "PACKET_SEND_ITEM_DATABASE_DATA", "PACKET_SEND_PARTICLE_EFFECT",
            "PACKET_SET_ICON_STATE", "PACKET_ITEM_EFFECT", "PACKET_SET_CHARACTER_STATE", "PACKET_PING_REPLY", "PACKET_PING_REQUEST", "PACKET_GOT_PUNCHED",
            "PACKET_APP_CHECK_RESPONSE", "PACKET_APP_INTEGRITY_FAIL", "PACKET_DISCONNECT", "PACKET_BATTLE_JOIN", "PACKET_BATTLE_EVENT", "PACKET_USE_DOOR",
            "PACKET_SEND_PARENTAL", "PACKET_GONE_FISHIN", "PACKET_STEAM", "PACKET_PET_BATTLE", "PACKET_NPC", "PACKET_SPECIAL", "PACKET_SEND_PARTICLE_EFFECT_V2",
            "PACKET_ACTIVE_ARROW_TO_ITEM", "PACKET_SELECT_TILE_INDEX", "PACKET_SEND_PLAYER_TRIBUTE_DATA", "PACKET_PVE_UNK1", "PACKET_PVE_UNK2", "PACKET_PVE_UNK3"
            "PACKET_PVE_UNK4", "PACKET_PVE_UNK5", "PACKET_SET_EXTRA_MODS", "PACKET_ON_STEP_ON_TILE_MOD" };
        
        return types[type];
    }

    inline std::string flag_to_string(ePacketFlag flag) {
        if (flag == 0)
            return "PACKET_FLAG_NONE";

        std::array<std::string, PACKET_FLAG_MAX> flags{
            "PACKET_FLAG_NONE", "PACKET_FLAG_UNK", "PACKET_FLAG_RESET_VISUAL_STATE", "PACKET_FLAG_EXTENDED", "PACKET_FLAG_ROTATE_LEFT", "PACKET_FLAG_ON_SOLID",
            "PACKET_FLAG_ON_FIRE_DAMAGE", "PACKET_FLAG_ON_JUMP", "PACKET_FLAG_ON_KILLED", "PACKET_FLAG_ON_PUNCH", "PACKET_FLAG_ON_PLACE", "PACKET_FLAG_ON_TILE_ACTION",
            "PACKET_FLAG_ON_GOT_PUNCHED", "PACKET_FLAG_ON_RESPAWNED", "PACKET_FLAG_ON_COLLECT_OBJECT", "PACKET_FLAG_ON_TRAMPOLINE", "PACKET_FLAG_ON_DAMAGE",
            "PACKET_FLAG_ON_SLIDE", "PACKET_FLAG_UNK4", "PACKET_FLAG_UNK5", "PACKET_FLAG_UNK6", "PACKET_FLAG_UNK7", "PACKET_FLAG_ON_WALL_HANG", "PACKET_FLAG_UNK8",
            "PACKET_FLAG_UNK9", "PACKET_FLAG_UNK10", "PACKET_FLAG_UNK11", "PACKET_FLAG_UNK12", "PACKET_FLAG_ON_ACID_DAMAGE" };

        std::string flag_string;
        for (int i = 0; i < flags.size(); i++) {
            if (flag & (1 << i)) {
                flag_string.append(flags[i]);
                flag_string.push_back('|');
            }
        }

        flag_string.pop_back();
        return flag_string;
    }

    inline eNetMessageType message_type_to_string(ENetPacket* packet) {
        if (packet->dataLength > 3) {
            return static_cast<eNetMessageType>(*packet->data);
        }

        spdlog::error("Bad packet length, ignoring message");
        return NET_MESSAGE_UNKNOWN;
    }

    inline char* get_text(ENetPacket* packet){
        std::memset(packet->data + packet->dataLength - 1, 0, 1);
        return (char*)(packet->data + 4);
    }

    inline char* get_struct(ENetPacket* packet, int length) {
        if (packet->dataLength < length + 4)
            return nullptr;
        return (char*)(packet->data + 4);
    }

    inline GameUpdatePacket* get_struct(ENetPacket* packet) {
        if (packet->dataLength >= sizeof(GameUpdatePacket)) {
            auto* game_update_packet = reinterpret_cast<GameUpdatePacket*>(packet->data + 4);
            if (!(game_update_packet->flags & player::PACKET_FLAG_EXTENDED))
                return game_update_packet;

            if (packet->dataLength < game_update_packet->data_size + sizeof(GameUpdatePacket)) {
                spdlog::error("Packet too small for extended packet to be valid");
                spdlog::error("Sizeof float is {}.  TankUpdatePacket size: {}", sizeof(float), sizeof(GameUpdatePacket));
                return nullptr;
            }
            return game_update_packet;
        }
        return nullptr;
    }

    inline uint8_t* get_extended_data(GameUpdatePacket* game_update_packet) {
        if (!(game_update_packet->flags & player::PACKET_FLAG_EXTENDED))
            return nullptr;

        struct ExtendedPacket {
            uint8_t pad[sizeof(GameUpdatePacket)];
            uint32_t data;
        };

        return reinterpret_cast<uint8_t*>(&reinterpret_cast<ExtendedPacket*>(game_update_packet)->data);
    }
}