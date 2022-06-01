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
                uint8_t x = i % tile_map.size.x;
                uint8_t y = std::floor(i / tile_map.size.x);

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
        const utils::math::Vec2<int>& start, const utils::math::Vec2<int>& end, items::Items* items)
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

                Tile tile = tile_map.tiles[neighbor->pos.y * tile_map.size.x + neighbor->pos.x];
                if (items->get_item(tile.foreground)->collision_type == 1) {
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

        return reconstruct_path(best_node);
    }
};
#pragma pack(pop)
