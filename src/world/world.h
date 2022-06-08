#pragma once
#include <cstdint>
#include <chrono>

#include "world_object_map.h"
#include "world_tile_map.h"
#include "../items/items.h"
#include "../utils/binary_reader.h"
#include "../utils/math.h"

#pragma pack(push, 1)
struct Node {
    utils::math::Vec2<int32_t> pos;
    Node* came_from;
    std::vector<Node*> neighbors;
    int32_t g;
    int32_t h;
    int32_t f;

    Node() : pos(), came_from(nullptr), g(0), h(0), f(0) {}
    ~Node()
    {
        neighbors.clear();
    }

    bool operator==(const Node& other) const
    {
        return pos == other.pos;
    }
};

struct World {
    uint16_t version;
    uint32_t unk;
    uint16_t name_len;
    std::string name;
    WorldTileMap tile_map;
    WorldObjectMap object_map;

    uint32_t world_owner_id;

    std::vector<Node*> nodes;
    std::vector<Node*> open_set;

    World() : version(0), unk(0), name_len(0), name(), tile_map(), object_map(), world_owner_id(0) {}
    ~World()
    {
        tile_map.tiles.clear();
        object_map.objects.clear();

        for (auto& node : nodes) {
            delete node;
        }

        nodes.clear();
    }

    void serialize(void* buffer)
    {
        BinaryReader br{ buffer };

        version = br.read_u16();
        unk = br.read_u32();
        name_len = br.read_u16();
        br.back(sizeof(uint16_t));
        name = br.read_string();

        std::size_t position = br.position();
        tile_map.serialize(buffer, position, version);
        object_map.serialize(buffer, position);

        if (nodes.size() < tile_map.size.x * tile_map.size.y) {
            for (int i = 0; i < tile_map.size.x * tile_map.size.y; i++) {
                auto x = static_cast<uint8_t>(i % tile_map.size.x);
                auto y = static_cast<uint8_t>(std::floor(i / tile_map.size.x));

                if (nodes.size() <= i) {
                    Node node{};
                    node.pos = { x, y };
                    nodes.emplace_back(new Node{ node });
                }
            }

            for (auto &node: nodes) {
                if (!node->neighbors.empty())
                    continue;

                node->neighbors.reserve(4);
                if (node->pos.x > 0) {
                    node->neighbors.emplace_back(nodes[node->pos.y * tile_map.size.x + node->pos.x - 1]);
                }

                if (node->pos.x < tile_map.size.x - 1) {
                    node->neighbors.emplace_back(nodes[node->pos.y * tile_map.size.x + node->pos.x + 1]);
                }

                if (node->pos.y > 0) {
                    node->neighbors.emplace_back(nodes[(node->pos.y - 1) * tile_map.size.x + node->pos.x]);
                }

                if (node->pos.y < tile_map.size.y - 1) {
                    node->neighbors.emplace_back(nodes[(node->pos.y + 1) * tile_map.size.x + node->pos.x]);
                }
            }
        }
    }

    static std::vector<utils::math::Vec2<int32_t>> reconstruct_path(Node* node)
    {
        std::vector<utils::math::Vec2<int32_t>> path{};
        while (node->came_from) {
            path.emplace_back(node->pos);
            node = node->came_from;
        }

        return path;
    }

    std::vector<utils::math::Vec2<int32_t>> find_path(
        const utils::math::Vec2<int>& start, const utils::math::Vec2<int>& end, items::Items* items, bool return_best_node = true)
    {
        open_set.clear();

        for (auto& node : nodes) {
            node->came_from = nullptr;
            node->g = std::numeric_limits<int32_t>::max();
            node->h = std::numeric_limits<int32_t>::max();
            node->f = std::numeric_limits<int32_t>::max();
        }

        Node* start_node = nodes[start.y * tile_map.size.x + start.x];
        Node* end_node = nodes[end.y * tile_map.size.x + end.x];

        start_node->g = 0;
        start_node->h = utils::math::manhattan_heuristic(start_node->pos, end_node->pos);
        start_node->f = start_node->g + start_node->h;
        open_set.push_back(start_node);

        Node* best_node{ start_node };

        auto start_time = std::chrono::high_resolution_clock::now();
        while (!open_set.empty()) {
            if (start_time + std::chrono::seconds(1) < std::chrono::high_resolution_clock::now()) {
                if (!return_best_node) {
                    std::vector<utils::math::Vec2<int32_t>> path{};
                    return path;
                }

                return reconstruct_path(best_node);
            }

            Node* current = open_set.front();
            if (current == end_node) {
                return reconstruct_path(current);
            }

            std::pop_heap(open_set.begin(), open_set.end(), [](Node* lhs, Node* rhs) {
                return lhs->f > rhs->f;
            });
            open_set.pop_back();

            for (auto& neighbor : current->neighbors) {
                if (neighbor->came_from != nullptr) {
                    continue;
                }

                Tile* tile = tile_map.tiles[neighbor->pos.y * tile_map.size.x + neighbor->pos.x];

                const auto is_collidable{
                    [tile](items::Items* items, bool apanih = false) {
                        uint8_t v19{ 0 };
                        uint8_t v20{ items->get_item(tile->foreground)->collision_type };
                        if (v20 != 5) v19 = 1;
                        uint8_t wut = (v20 != 0) & v19;

                        // From Tile::IsCollidable(Tile *this, int a2, World *a3, bool a4, float a5)
                        if (!wut) return false;
                        if (apanih) {
                            switch (v20) {
                                case 2:
                                case 5:
                                case 7:
                                    return false;
                                case 3:
                                    // goto LABEL_6;
                                    break;
                                default:
                                    goto LABEL_23;
                            }
                        }
                        if (v20 == 5) return false;
                        /*if ( v7 == 3 )
                        {
LABEL_6:
                            v9 = *(TileExtra **)(v4 + 40);
                            if ( v9 && *((_BYTE *)v9 + 4) == 3 )
                            {
                                v10 = (Tile *)v4;
                            }
                            else
                            {
                                v11 = *(unsigned __int16 *)(v4 + 104);
                                v12 = v11 == 0;
                                if ( !*(_WORD *)(v4 + 104) )
                                    v12 = *((_DWORD *)a3 + 41) == 0;
                                if ( v12 )
                                    return 0;
                                if ( !v9 || (v10 = (Tile *)v4, *((_BYTE *)v9 + 4) != 3) )
                                {
                                    if ( !*(_WORD *)(v4 + 104) )
                                    {
                                        v13 = *((_DWORD *)a3 + 41);
                                        if ( v13 )
                                            v11 = *(unsigned __int16 *)(v13 + 12);
                                        else
                                            v11 = 0;
                                    }
                                    v10 = (Tile *)(*((_DWORD *)a3 + 6) + 140 * v11);
                                    if ( !v10 )
                                    {
LABEL_21:
                                        v8 = 0;
                                        if ( *(unsigned __int8 *)(v4 + 8) >> 7 )
                                            return v8;
                                        v7 = *(_DWORD *)(v4 + 20);
                                        goto LABEL_23;
                                    }
                                    v9 = (TileExtra *)*((_DWORD *)v10 + 10);
                                }
                            }
                            if ( TileExtra::HasKeyToLock(v9, a2, v10, 1) )
                                return 0;
                            goto LABEL_21;
                        }*/
LABEL_23:
                        if (v20 != 6)
                            goto LABEL_27;
                        if ((tile->flag & Tile::eFlag::EXTRA) == Tile::eFlag::EXTRA)
                            return true;
                        /*if (TileExtra::HasKeyToLock(v14, a2, (Tile *)v4, 1))
                            return false;*/
LABEL_27:
                        if (v20 == 13) {
                            if ((tile->flag & Tile::eFlag::EXTRA) == Tile::eFlag::EXTRA)
                                return true;
                            /*if (TileExtra::IsAllowedThroughFriendsEntrance(v15, a2, (Tile *)v4))
                                return false;*/
                        }
                        /*if ( v7 == 8 )
                        {
                            if ( GetWorldRenderer()
                                 && (WorldRenderer = (WorldRenderer *)GetWorldRenderer(),
                                    WorldRenderer::IsDisguised(WorldRenderer, (const Tile *)v4))
                                 && !*(_WORD *)(v4 + 4) )
                            {
                                v18 = *(_DWORD *)(v4 + 128);
                            }
                            else
                            {
                                LOWORD(v18) = *(_WORD *)(v4 + 4);
                            }
                            if ( (unsigned __int16)v18 == 4698 )
                            {
                                GameLogic = (GameLogicComponent *)GetGameLogic();
                                if ( GameLogicComponent::GetLocalPlayer(GameLogic) )
                                {
                                    v20 = (GameLogicComponent *)GetGameLogic();
                                    LocalPlayer = GameLogicComponent::GetLocalPlayer(v20);
                                    return PlayerAdventure::HasItemID((PlayerAdventure *)(LocalPlayer + 520), 1696) == 0;
                                }
                                return 1;
                            }
                            if ( GetWorldRenderer()
                                 && (v22 = (WorldRenderer *)GetWorldRenderer(), WorldRenderer::IsDisguised(v22, (const Tile *)v4))
                                 && !*(_WORD *)(v4 + 4) )
                            {
                                v23 = *(_DWORD *)(v4 + 128);
                            }
                            else
                            {
                                LOWORD(v23) = *(_WORD *)(v4 + 4);
                            }
                            if ( (unsigned __int16)v23 == 4706 )
                            {
                                v24 = (GameLogicComponent *)GetGameLogic();
                                if ( !GameLogicComponent::GetLocalPlayer(v24) )
                                    return (*(_BYTE *)(v4 + 8) & 0x40) == 0;
                                v25 = (GameLogicComponent *)GetGameLogic();
                                if ( !*(_BYTE *)(GameLogicComponent::GetLocalPlayer(v25) + 544) )
                                    return (*(_BYTE *)(v4 + 8) & 0x40) == 0;
                                return (*(unsigned __int8 *)(v4 + 8) >> 6) & 1;
                            }
                            if ( GetWorldRenderer()
                                 && (v26 = (WorldRenderer *)GetWorldRenderer(), WorldRenderer::IsDisguised(v26, (const Tile *)v4))
                                 && !*(_WORD *)(v4 + 4) )
                            {
                                v27 = *(_DWORD *)(v4 + 128);
                            }
                            else
                            {
                                LOWORD(v27) = *(_WORD *)(v4 + 4);
                            }
                            if ( (unsigned __int16)v27 == 4710 )
                            {
                                v28 = (GameLogicComponent *)GetGameLogic();
                                if ( !GameLogicComponent::GetLocalPlayer(v28) )
                                    return (*(_BYTE *)(v4 + 8) & 0x40) == 0;
                                v29 = (GameLogicComponent *)GetGameLogic();
                                if ( !*(_BYTE *)(GameLogicComponent::GetLocalPlayer(v29) + 545) )
                                    return (*(_BYTE *)(v4 + 8) & 0x40) == 0;
                                return (*(unsigned __int8 *)(v4 + 8) >> 6) & 1;
                            }
                            if ( GetWorldRenderer()
                                 && (v30 = (WorldRenderer *)GetWorldRenderer(), WorldRenderer::IsDisguised(v30, (const Tile *)v4))
                                 && !*(_WORD *)(v4 + 4) )
                            {
                                v31 = *(_DWORD *)(v4 + 128);
                            }
                            else
                            {
                                LOWORD(v31) = *(_WORD *)(v4 + 4);
                            }
                            if ( (unsigned __int16)v31 == 4744 )
                            {
                                v32 = (GameLogicComponent *)GetGameLogic();
                                if ( !GameLogicComponent::GetLocalPlayer(v32) )
                                    return (*(_BYTE *)(v4 + 8) & 0x40) == 0;
                                v33 = (GameLogicComponent *)GetGameLogic();
                                if ( !*(_BYTE *)(GameLogicComponent::GetLocalPlayer(v33) + 546) )
                                    return (*(_BYTE *)(v4 + 8) & 0x40) == 0;
                                return (*(unsigned __int8 *)(v4 + 8) >> 6) & 1;
                            }
                            v7 = *(_DWORD *)(v4 + 20);
                        }*/
                        switch (v20) {
                            case 4:
                                return (tile->flag & 0x40) == 0;
                            case 5:
                            case 6:
                            case 7:
                            case 8:
                                return true;
                            case 9:
                                return static_cast<bool>((tile->flag >> 6) & 1);
                            /*case 10:
                                v34 = *(_WORD *)(v4 + 8);
                                v35 = (GameLogicComponent *)GetGameLogic();
                                v36 = v34 & 0x80;
                                if ( !GameLogicComponent::GetLocalPlayer(v35)
                                     || (v37 = (GameLogicComponent *)GetGameLogic(),
                                         v38 = *(unsigned __int8 *)(GameLogicComponent::GetLocalPlayer(v37) + 448),
                                         v39 = (GameLogicComponent *)GetGameLogic(),
                                         v40 = GameLogicComponent::GetLocalPlayer(v39),
                                        !v38) )
                                {
                                    if ( v36 )
                                        return 0;
                                    v53 = *(TileExtra **)(v4 + 40);
                                    if ( !v53 || *((_BYTE *)v53 + 4) != 3 )
                                    {
                                        v54 = *(unsigned __int16 *)(v4 + 104);
                                        v55 = v54 == 0;
                                        if ( !*(_WORD *)(v4 + 104) )
                                            v55 = *((_DWORD *)a3 + 41) == 0;
                                        if ( v55 )
                                            return 0;
                                        if ( !v53 || *((_BYTE *)v53 + 4) != 3 )
                                        {
                                            if ( !*(_WORD *)(v4 + 104) )
                                            {
                                                v56 = *((_DWORD *)a3 + 41);
                                                if ( v56 )
                                                    v54 = *(unsigned __int16 *)(v56 + 12);
                                                else
                                                    v54 = 0;
                                            }
                                            v4 = *((_DWORD *)a3 + 6) + 140 * v54;
                                            v8 = 1;
                                            if ( !v4 )
                                                return v8;
                                            v53 = *(TileExtra **)(v4 + 40);
                                        }
                                    }
                                    return TileExtra::HasKeyToLock(v53, a2, (Tile *)v4, 1) == 0;
                                }
                                v41 = *(TileExtra **)(v4 + 40);
                                v42 = *(_DWORD *)(v40 + 436);
                                v43 = *((unsigned __int8 *)v41 + 40);
                                if ( v36 )
                                    return v42 != v43;
                                if ( v41 && *((_BYTE *)v41 + 4) == 3 )
                                    goto LABEL_116;
                                v57 = *(unsigned __int16 *)(v4 + 104);
                                v58 = v57 == 0;
                                if ( !*(_WORD *)(v4 + 104) )
                                    v58 = *((_DWORD *)a3 + 41) == 0;
                                if ( v58 )
                                {
                                    v60 = 0;
                                }
                                else
                                {
                                    if ( v41 && *((_BYTE *)v41 + 4) == 3 )
                                        goto LABEL_116;
                                    if ( !*(_WORD *)(v4 + 104) )
                                    {
                                        v59 = *((_DWORD *)a3 + 41);
                                        if ( v59 )
                                            v57 = *(unsigned __int16 *)(v59 + 12);
                                        else
                                            v57 = 0;
                                    }
                                    v4 = *((_DWORD *)a3 + 6) + 140 * v57;
                                    if ( v4 )
                                    {
                                        v41 = *(TileExtra **)(v4 + 40);
LABEL_116:
                                        v60 = TileExtra::HasKeyToLock(v41, a2, (Tile *)v4, 1) ^ 1;
                                        return v60 | (v42 != v43);
                                    }
                                    v60 = 1;
                                }
                                return v60 | (v42 != v43);
                            case 11:
                                v44 = *(_DWORD *)(v4 + 40);
                                v45 = (GameLogicComponent *)GetGameLogic();
                                v46 = (_DWORD *)GameLogicComponent::GetLocalPlayer(v45);
                                if ( !v46 || !v44 )
                                    return 1;
                                v47 = *(unsigned __int8 *)(v44 + 5);
                                v48 = 0;
                                if ( v47 <= 0xBF )
                                {
                                    v48 = 3;
                                    if ( v47 >= 0x40 )
                                        v48 = 2 - (v47 >> 7);
                                }
                                v8 = 0;
                                if ( v46[30] >= v48 && v46[27] == *(_DWORD *)(v44 + 100) )
                                    return v8;
                                v49 = *(TileExtra **)(v4 + 40);
                                v50 = v46[57];
                                if ( v49 && *((_BYTE *)v49 + 4) == 3 )
                                {
                                    v51 = (Tile *)v4;
                                }
                                else
                                {
                                    v61 = *(unsigned __int16 *)(v4 + 104);
                                    v62 = v61 == 0;
                                    if ( !*(_WORD *)(v4 + 104) )
                                        v62 = *((_DWORD *)a3 + 41) == 0;
                                    if ( v62 )
                                        return v8;
                                    if ( !v49 || (v51 = (Tile *)v4, *((_BYTE *)v49 + 4) != 3) )
                                    {
                                        if ( !*(_WORD *)(v4 + 104) )
                                        {
                                            v63 = *((_DWORD *)a3 + 41);
                                            if ( v63 )
                                                v61 = *(unsigned __int16 *)(v63 + 12);
                                            else
                                                v61 = 0;
                                        }
                                        v51 = (Tile *)(*((_DWORD *)a3 + 6) + 140 * v61);
                                        if ( !v51 )
                                            return *(unsigned __int8 *)(v4 + 8) >> 7 == 0;
                                        v49 = (TileExtra *)*((_DWORD *)v51 + 10);
                                    }
                                }
                                if ( TileExtra::HasKeyToLock(v49, v50, v51, 1) )
                                    return v8;
                                return *(unsigned __int8 *)(v4 + 8) >> 7 == 0;*/
                            case 12:
                                if ((tile->flag & Tile::eFlag::EXTRA) == Tile::eFlag::EXTRA)
                                    return false/**(_DWORD *)(v52 + 172) > 0*/; // Timer?
                                return true;
                            default:
                                break;
                        }
                        return true;
                    }
                };

                if (is_collidable(items)) {
                    continue;
                }

                int32_t tentative_g = current->g + 1;
                if (tentative_g < neighbor->g) {
                    neighbor->came_from = current;
                    neighbor->g = tentative_g;
                    neighbor->h = utils::math::manhattan_heuristic(neighbor->pos, end_node->pos);
                    neighbor->f = neighbor->g + neighbor->h;

                    if (neighbor->h < best_node->h) {
                        best_node = neighbor;
                    }

                    auto it = std::find(open_set.begin(), open_set.end(), neighbor);
                    if (it == open_set.end()) {
                        open_set.emplace_back(neighbor);
                    }

                    std::push_heap(open_set.begin(), open_set.end(), [](Node* lhs, Node* rhs) {
                        return lhs->f > rhs->f;
                    });
                }
            }
        }

        if (!return_best_node) {
            std::vector<utils::math::Vec2<int32_t>> path{};
            return path;
        }

        return reconstruct_path(best_node);
    }
};
#pragma pack(pop)
