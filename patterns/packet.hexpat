// Enter your pattern code here and click on the Play button
// below to execute your code. The results will then be visible
// in the Pattern Data view.
// 
// More information can be found in the documentation.
// 
// 
// Simple example:
// 
// import std.io;
// 
// struct Pattern {
//     u32 int;
//     float floating_point;
// };
// 
// Pattern my_pattern @ 0x00;
// std::print("0x{:08X}", my_pattern.int);

#pragma endian little
#include <std/mem.pat>
#include <std/sys.pat>

enum NetMessageType : u32 {
    UNKNOWN = 0,
    SERVER_HELLO = 1,
    GENERIC_TEXT = 2,
    GAME_MESSAGE = 3,
    GAME_PACKET = 4,
    ERROR = 5,
    TRACK = 6,
    CLIENT_LOG_REQUEST = 7,
    CLIENT_LOG_RESPONSE = 8,
};

enum PacketType : u8 {
    STATE = 0,
    CALL_FUNCTION = 1,
    UPDATE_STATUS = 2,
    TILE_CHANGE_REQUEST = 3,
    SEND_MAP_DATA = 4,
    SEND_TILE_UPDATE_DATA = 5,
    SEND_TILE_UPDATE_DATA_MULTIPLE = 6,
    TILE_ACTIVATE_REQUEST = 7,
    TILE_APPLY_DAMAGE = 8,
    SEND_INVENTORY_STATE = 9,
    ITEM_ACTIVATE_REQUEST = 10,
    ITEM_ACTIVATE_OBJECT_REQUEST = 11,
    SEND_TILE_TREE_STATE = 12,
    MODIFY_ITEM_INVENTORY = 13,
    ITEM_CHANGE_OBJECT = 14,
    SEND_LOCK = 15,
    SEND_ITEM_DATABASE_DATA = 16,
    SEND_PARTICLE_EFFECT = 17,
    SET_ICON_STATE = 18,
    ITEM_EFFECT = 19,
    SET_CHARACTER_STATE = 20,
    PING_REPLY = 21,
    PING_REQUEST = 22,
    GOT_PUNCHED = 23,
    APP_CHECK_RESPONSE = 24,
    APP_INTEGRITY_FAIL = 25,
    DISCONNECT = 26,
    BATTLE_JOIN = 27,
    BATTLE_EVENT = 28,
    USE_DOOR = 29,
    SEND_PARENTAL = 30,
    GONE_FISHIN = 31,
    STEAM = 32,
    PET_BATTLE = 33,
    NPC = 34,
    SPECIAL = 35,
    SEND_PARTICLE_EFFECT_V2 = 36,
    ACTIVE_ARROW_TO_ITEM = 37,
    SELECT_TILE_INDEX = 38,
    SEND_PLAYER_TRIBUTE_DATA = 39,
    FTUE_SET_ITEM_TO_QUICK_INVENTORY = 40,
    PVE_NPC = 41,
    PVP_CARD_BATTLE = 42,
    PVE_APPLY_PLAYER_DAMAGE = 43,
    PVE_NPC_POSITION_DAMAGE = 44,
    SET_EXTRA_MODS = 45,
    ON_STEP_ON_TILE_MOD = 46,
};

bitfield PacketFlag {
    none : 1;
    unk : 1;
    reset_visual_state : 1;
    extended : 1;
    rotate_left : 1;
    on_solid : 1;
    on_fire_damage : 1;
    on_jump : 1;
    on_killed : 1;
    on_punched : 1;
    on_placed : 1;
    on_tile_action : 1;
    on_got_punched : 1;
    on_respawned : 1;
    on_collect_object : 1;
    on_trampoline : 1;
    on_damage : 1;
    on_slide : 1;
    pad_1 : 3;
    on_wall_hang : 1;
    pad_2 : 3;
    on_acid_damage : 1;
    pad_3 : 6;
};

enum VariantType : u8 {
    UNKNOWN = 0,
    FLOAT = 1,
    STRING = 2,
    VEC2 = 3,
    VEC3 = 4,
    UNSIGNED = 5,
    SIGNED = 9
};

struct Variant {
    u8 index;
    VariantType type;

    if (type == VariantType::FLOAT) {
        float value;
    } else if (type == VariantType::STRING) {
        u32 len;
        char value[len];
    } else if (type == VariantType::VEC2) {
        float x;
        float y;
    } else if (type == VariantType::VEC3) {
        float x;
        float y;
        float z;
    } else if (type == VariantType::UNSIGNED) {
        u32 value;
    } else if (type == VariantType::SIGNED) {
        s32 value;
    }
};

struct GameUpdatePacket {
    PacketType type;
    u8 pad[3];
    u32 net_id;
    u8 pad_2[4];
    PacketFlag flags;
    u8 pad_3[4];
    u32 decompressed_data_size;
    u8 pad_4[28];
    u32 data_size;

    // Why the fuck Growtopia client still parse variant even on zero data size?
    if (/*data_size > 0 && */type == PacketType::CALL_FUNCTION) {
        u8 variant_count;
        Variant variants[variant_count];
    } else if (data_size > 0) {
        char data[data_size];
    } else {
        char data[while(!std::mem::eof())];
    }
};

struct Packet {
    NetMessageType type;
    if (type == NetMessageType::GAME_PACKET) {
        GameUpdatePacket packet;
    } else if (type == NetMessageType::GENERIC_TEXT || type == NetMessageType::GAME_MESSAGE) {
        char message[while(!std::mem::eof())];
    } else {
        char data[while(!std::mem::eof())];
    }
};

Packet packet @ 0x00;
