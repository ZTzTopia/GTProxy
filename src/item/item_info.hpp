#pragma once
#include <cstdint>
#include <string>

namespace item {
enum class MaterialType : std::uint8_t
{
    Wooden,
    Glass,
    Rock,
    Metal,
};

struct ItemInfoFlag
{
    std::uint16_t flippable : 1;
    std::uint16_t editable : 1;
    std::uint16_t seedless : 1;
    std::uint16_t permanent : 1;
    std::uint16_t dropless : 1;
    std::uint16_t no_self : 1;
    std::uint16_t no_shadow : 1;
    std::uint16_t world_locked : 1;
    std::uint16_t beta : 1;
    std::uint16_t auto_pickup : 1;
    std::uint16_t mod : 1;
    std::uint16_t random_grow : 1;
    std::uint16_t is_public : 1;
    std::uint16_t foreground : 1;
    std::uint16_t holiday : 1;
    std::uint16_t untradeable : 1;
};

struct FXFlags
{
    std::uint32_t multi_anim_start : 1;
    std::uint32_t multi_anim_2_start : 1;
    std::uint32_t ping_pong_anim_start : 1;
    std::uint32_t overlay_object : 1;
    std::uint32_t render_fx_variant_version : 1;
    std::uint32_t offset_up : 1;
    std::uint32_t dual_layer : 1;
    std::uint32_t use_skin_tint : 1;
    std::uint32_t seed_tint_layer_1 : 1;
    std::uint32_t seed_tint_layer_2 : 1;
    std::uint32_t rainbow_tint_layer_1 : 1;
    std::uint32_t rainbow_tint_layer_2 : 1;
    std::uint32_t glow : 1;
    std::uint32_t no_arms : 1;
    std::uint32_t render_off_hand : 1;
    std::uint32_t front_arm_punch : 1;
    std::uint32_t slow_fall_object : 1;
    std::uint32_t replacement_sprite : 1;
    std::uint32_t orb_float : 1;
    std::uint32_t padding : 13;
};

struct ItemInfoFlag2
{
    std::uint32_t robot_deadly : 1;
    std::uint32_t robot_shoot_left : 1;
    std::uint32_t robot_shoot_right : 1;
    std::uint32_t robot_shoot_down : 1;
    std::uint32_t robot_shoot_up : 1;
    std::uint32_t robot_can_shoot : 1;
    std::uint32_t robot_lava : 1;
    std::uint32_t robot_pointy : 1;
    std::uint32_t robot_shoot_deadly : 1;
    std::uint32_t guild_item : 1;
    std::uint32_t guild_flag : 1;
    std::uint32_t starship_helm : 1;
    std::uint32_t starship_reactor : 1;
    std::uint32_t starship_view_screen : 1;
    std::uint32_t super_mod : 1;
    std::uint32_t tile_deadly_if_on : 1;
    std::uint32_t long_hand_item_64x32 : 1;
    std::uint32_t gemless : 1;
    std::uint32_t transmutable : 1;
    std::uint32_t dungeon_item : 1;
    std::uint32_t pve_melee : 1;
    std::uint32_t pve_ranged : 1;
    std::uint32_t pve_auto_aim : 1;
    std::uint32_t one_in_world : 1;
    std::uint32_t only_for_world_owner : 1;
    std::uint32_t no_upgrade : 1;
    std::uint32_t extinguish_fire : 1;
    std::uint32_t extinguish_fire_no_damage : 1;
    std::uint32_t need_reception_desk : 1;
    std::uint32_t use_paint : 1;
    std::uint32_t padding : 2;
};

enum class ItemType : std::uint8_t
{
    AnyButSeed = static_cast<std::uint8_t>(-2),
    Any = static_cast<std::uint8_t>(-1),
    Fist = 0,
    Wrench = 1,
    UserDoor = 2,
    Lock = 3,
    Gems = 4,
    Treasure = 5,
    Deadly = 6,
    Trampoline = 7,
    Consumable = 8,
    Gateway = 9,
    Sign = 10,
    SfxWithExtraFrame = 11,
    Boombox = 12,
    Door = 13,
    Platform = 14,
    Bedrock = 15,
    Lava = 16,
    Normal = 17,
    Background = 18,
    Seed = 19,
    Clothes = 20,
    NormalWithExtraFrame = 21,
    BackgdSfxExtraFrame = 22,
    BackBoombox = 23,
    Bouncy = 24,
    Pointy = 25,
    Portal = 26,
    Checkpoint = 27,
    Musicnote = 28,
    Ice = 29,
    RaceFlag = 30,
    Switcheroo = 31,
    Chest = 32,
    Mailbox = 33,
    Bulletin = 34,
    Pinata = 35,
    Dice = 36,
    Component = 37,
    Provider = 38,
    Lab = 39,
    Achievement = 40,
    WeatherMachine = 41,
    Scoreboard = 42,
    Sungate = 43,
    Profile = 44,
    DeadlyIfOn = 45,
    HeartMonitor = 46,
    DonationBox = 47,
    Toybox = 48,
    Mannequin = 49,
    Camera = 50,
    Magicegg = 51,
    Team = 52,
    GameGen = 53,
    Xenonite = 54,
    Dressup = 55,
    Crystal = 56,
    Burglar = 57,
    Compactor = 58,
    Spotlight = 59,
    Wind = 60,
    DisplayBlock = 61,
    Vending = 62,
    Fishtank = 63,
    Petfish = 64,
    Solar = 65,
    Forge = 66,
    GivingTree = 67,
    GivingTreeStump = 68,
    Steampunk = 69,
    SteamLavaIfOn = 70,
    SteamOrgan = 71,
    Tamagotchi = 72,
    Sewing = 73,
    Flag = 74,
    LobsterTrap = 75,
    Artcanvas = 76,
    BattleCage = 77,
    PetTrainer = 78,
    SteamEngine = 79,
    LockBot = 80,
    WeatherSpecial = 81,
    SpiritStorage = 82,
    DisplayShelf = 83,
    VipDoor = 84,
    ChalTimer = 85,
    ChalFlag = 86,
    FishMount = 87,
    Portrait = 88,
    WeatherSpecial2 = 89,
    Fossil = 90,
    FossilPrep = 91,
    DnaMachine = 92,
    Blaster = 93,
    Valhowla = 94,
    Chemsynth = 95,
    Chemtank = 96,
    Storage = 97,
    Oven = 98,
    SuperMusic = 99,
    Geigercharge = 100,
    AdventureReset = 101,
    TombRobber = 102,
    Faction = 103,
    RedFaction = 104,
    GreenFaction = 105,
    BlueFaction = 106,
    Artifact = 107,
    TrampolineMomentum = 108,
    FishgotchiTank = 109,
    FishingBlock = 110,
    ItemSucker = 111,
    ItemPlanter = 112,
    Robot = 113,
    Command = 114,
    LuckyTicket = 115,
    StatsBlock = 116,
    FieldNode = 117,
    OuijaBoard = 118,
    ArchitectMachine = 119,
    Starship = 120,
    Autodelete = 121,
    Boombox2 = 122,
    AutoActionBreak = 123,
    AutoActionHarvest = 124,
    AutoActionHarvestSuck = 125,
    LightningCloud = 126,
    PhasedBlock = 127,
    Mud = 128,
    RootCutting = 129,
    PasswordStorage = 130,
    PhasedBlock2 = 131,
    Bomb = 132,
    PveNpc = 133,
    InfinityWeatherMachine = 134,
    Slime = 135,
    Acid = 136,
    Completionist = 137,
    PunchToggle = 138,
    AnzuBlock = 139,
    FeedingBlock = 140,
    KrankensBlock = 141,
    FriendsEntrance = 142,
    Pearls = 143,
};

enum class VisualEffect : std::uint8_t
{
    None = 0,
    FlameLick = 1,
    Smoking = 2,
    GlowTint1 = 3,
    Anim = 4,
    Bubbles = 5,
    Pet = 6,
    PetAnim = 7,
    NoArms = 8,
    Wavey = 9,
    WaveyAnim = 10,
    Botharms = 11,
    Lowhair = 12,
    Underface = 13,
    Skintint = 14,
    Mask = 15,
    AnimMask = 16,
    LowhairMask = 17,
    Ghost = 18,
    BubbleMachine = 46,
    Pulse = 19,
    Colorize = 20,
    ColorizeToShirt = 21,
    ColorizeAnim = 22,
    Highface = 23,
    HighfaceAnim = 24,
    RainbowShift = 25,
    Backfore = 26,
    ColorizeWithSkin = 27,
    NoRender = 28,
    Spin = 29,
    Offhand = 30,
    Winged = 31,
    Sink = 32,
    Darkness = 33,
    Lightsource = 34,
    LightIfOn = 35,
    Discolor = 36,
    StepSpin = 37,
    Petcolored = 38,
    Silkfoot = 39,
    Tilty = 40,
    TiltyDark = 41,
    NextFrameIfOn = 42,
    Wobble = 43,
    Scroll = 44,
    LightsourcePulse = 45,
    Verylowhair = 47,
    VerylowhairMask = 48,
};

enum class TextureType : std::uint8_t
{
    SingleFrameAlone = 0,
    SingleFrame = 1,
    SmartEdge = 2,
    SmartEdgeHoriz = 3,
    SmartCling = 4,
    SmartCling2 = 5,
    SmartOuter = 6,
    Random = 7,
    SmartEdgeVert = 8,
    SmartEdgeHorizCave = 9,
    SmartEdgeDiagon = 10,
};

enum class CollisionType : std::uint8_t
{
    None,
    Full,
    JumpThrough,
    Gateway,
    CollideIfOff,
    OneWay,
    VipDoor,
    JumpDown,
    Adventure,
    CollideIfOn,
    Faction,
    Guild,
    Cloud,
    FriendEntrance,
};

enum class SeedBase : std::uint8_t
{
    None,
    ShadeUpper,
    Empty,
    Rough,
    Lines,
    Empty2,
    Rough2,
    Rough3,
    Lines2,
    CircleCenter,
    CircleCenter2,
    Empty3,
    DarkBottom,
    ShadeTop,
    Empty4,
    Cracked,
};

enum class SeedOverlay : std::uint8_t
{
    Lines,
    Stripey = 2,
    PolkaDot = 4,
    ShadeHalf = 6,
    Cross = 8,
    JaggedLine = 10,
    WaveyLines = 12,
    DiagonalLines = 14,
};

enum class TreeBase : std::uint8_t
{
    BranchTree,
    Roots,
    Roots2,
    BigTrunk,
    BranchTree2,
    NoRoots,
    BentSlightly,
    VineLikePlant,
};

enum class TreeLeaves : std::uint8_t
{
    FlatLeaves = 0,
    BigTriangularLeaves = 2,
    BigCircleLeaves = 4,
    CircularLeaves = 6,
};

struct Color
{
    std::uint32_t a : 8;
    std::uint32_t r : 8;
    std::uint32_t g : 8;
    std::uint32_t b : 8;
};

enum class ClothingType : std::uint8_t
{
    None,
    Shirt,
    Pants,
    Shoes,
    Face,
    Hand,
    Back,
    Hair,
    Neck,
};

struct ItemInfo
{
    std::uint32_t item_id;
    ItemInfoFlag flags;
    ItemType item_type;
    MaterialType material;

    std::string item_name;

    std::string texture_file_path;
    std::uint32_t texture_file_hash;

    VisualEffect visual_effect;
    std::uint32_t cooking_time;
    std::uint8_t texture_coord_x;
    std::uint8_t texture_coord_y;
    TextureType texture_type;
    std::uint8_t is_stripey_wallpaper;
    CollisionType collision_type;
    std::uint8_t health;
    std::uint32_t reset_time;
    ClothingType clothing_type;
    std::uint16_t rarity;
    std::uint8_t max_amount;

    std::string extra_file;
    std::uint32_t extra_file_hash;
    std::uint32_t animation_time;

    std::string pet_name;
    std::string pet_prefix;
    std::string pet_suffix;
    std::string pet_ability;

    SeedBase seed_base;
    SeedOverlay seed_overlay;
    TreeBase tree_base;
    TreeLeaves tree_leaves;
    Color seed_color;
    Color seed_overlay_color;
    std::uint32_t ingredient;
    std::uint32_t grow_time;

    FXFlags fx_flag;

    std::string animating_coordinates;
    std::string animating_texture_file;
    std::string animating_coordinates_2;

    std::uint32_t __unk_1;
    std::uint32_t __unk_2;
    ItemInfoFlag2 flags_2;

    std::uint8_t __unk_3[60];

    std::uint32_t tile_range;
    std::uint32_t vault_capacity;
    std::string punch_options;

    std::uint32_t __unk_4;
    std::uint8_t body_part_list[9];
    std::uint32_t light_range;
    std::uint32_t __unk_5;

    std::uint8_t can_sit;
    std::uint32_t player_offset_x;
    std::uint32_t player_offset_y;
    std::uint32_t chair_texture_x;
    std::uint32_t chair_texture_y;
    std::uint32_t chair_leg_offset_x;
    std::uint32_t chair_leg_offset_y;
    std::string chair_texture_file;
    std::string renderer_data_file;
    std::uint32_t __unk_6;
    std::int32_t renderer_data_file_hash;
    std::uint8_t __unk_7[9];

    std::uint16_t __unk_8;
    std::string info;
    std::uint16_t ingredients[2];
    std::uint8_t __unk_9;
};
}
