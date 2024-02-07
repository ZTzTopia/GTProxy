#pragma once

namespace packet {
enum NetMessageType : uint32_t {
    NET_MESSAGE_UNKNOWN,
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

enum PacketType : uint8_t {
    PACKET_STATE,
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
    PACKET_FTUE_SET_ITEM_TO_QUICK_INVENTORY,
    PACKET_PVE_NPC,
    PACKET_PVP_CARD_BATTLE,
    PACKET_PVE_APPLY_PLAYER_DAMAGE,
    PACKET_PVE_NPC_POSITION_DAMAGE,
    PACKET_SET_EXTRA_MODS,
    PACKET_ON_STEP_ON_TILE_MOD,
    PACKET_MAX,
};

enum PacketFlag : uint32_t {
    PACKET_FLAG_NONE = 0,
    PACKET_FLAG_UNK = 1 << 1,
    PACKET_FLAG_RESET_VISUAL_STATE = 1 << 2,
    PACKET_FLAG_EXTENDED = 1 << 3,
    PACKET_FLAG_ROTATE_LEFT = 1 << 4,
    PACKET_FLAG_ON_SOLID = 1 << 5,
    PACKET_FLAG_ON_FIRE_DAMAGE = 1 << 6,
    PACKET_FLAG_ON_JUMP = 1 << 7,
    PACKET_FLAG_ON_KILLED = 1 << 8,
    PACKET_FLAG_ON_PUNCHED = 1 << 9,
    PACKET_FLAG_ON_PLACED = 1 << 10,
    PACKET_FLAG_ON_TILE_ACTION = 1 << 11,
    PACKET_FLAG_ON_GOT_PUNCHED = 1 << 12,
    PACKET_FLAG_ON_RESPAWNED = 1 << 13,
    PACKET_FLAG_ON_COLLECT_OBJECT = 1 << 14,
    PACKET_FLAG_ON_TRAMPOLINE = 1 << 15,
    PACKET_FLAG_ON_DAMAGE = 1 << 16,
    PACKET_FLAG_ON_SLIDE = 1 << 17,
    PACKET_FLAG_ON_WALL_HANG = 1 << 21,
    PACKET_FLAG_ON_ACID_DAMAGE = 1 << 26
};

#pragma pack(push, 1)
struct GameUpdatePacket {
    PacketType type;
    uint8_t pad[3];
    uint32_t net_id;
    uint8_t pad_2[4];

    union {
        PacketFlag value;
        struct {
            uint32_t none : 1;
            uint32_t unk : 1;
            uint32_t reset_visual_state : 1;
            uint32_t extended : 1;
            uint32_t rotate_left : 1;
            uint32_t on_solid : 1;
            uint32_t on_fire_damage : 1;
            uint32_t on_jump : 1;
            uint32_t on_killed : 1;
            uint32_t on_punched : 1;
            uint32_t on_placed : 1;
            uint32_t on_tile_action : 1;
            uint32_t on_got_punched : 1;
            uint32_t on_respawned : 1;
            uint32_t on_collect_object : 1;
            uint32_t on_trampoline : 1;
            uint32_t on_damage : 1;
            uint32_t on_slide : 1;
            uint32_t pad_1 : 3;
            uint32_t on_wall_hang : 1;
            uint32_t pad_2 : 3;
            uint32_t on_acid_damage : 1;
            uint32_t pad_3 : 6;
        };
    } flags;

    uint8_t pad_3[4];
    uint32_t decompressed_data_size;
    uint8_t pad_4[28];
    uint32_t data_size;
};
#pragma pack(pop)
}
