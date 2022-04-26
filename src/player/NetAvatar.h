#pragma once
#include <string>

#pragma pack(push, 1)
class NetAvatar {
public:
    CL_Vec2f pos;
    std::string name;
    std::string world_name;
    uint8_t facing_left;
    bool is_invis;
    bool is_mod;
    bool is_supermod;

    struct NetAvatarData {
        uint8_t unk_1;
        uint8_t punch_id;
        uint8_t punch_range;
        uint8_t build_range;
        int32_t net_id;
        int32_t pupil_color;
        int32_t flags;
        float water_speed;
        int32_t effect_flags;
        float acceleration;
        float punch_strength;
        float speed;
        float gravity;
        char unk_2[4];
        uint32_t hair_color;
        uint32_t eye_color;
    } AvatarData;
};
#pragma pack(pop)