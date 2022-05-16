#pragma once
#include <cstdint>

#include "world_object_map.h"
#include "world_tile_map.h"
#include "../utils/binary_reader.h"
#include "../utils/math.h"

#pragma pack(push, 1)
struct World {
    uint16_t version;
    uint32_t unk;
    uint16_t name_len;
    std::string name;
    WorldTileMap tile_map;
    WorldObjectMap object_map;

    // A* stuff
    struct Node {
        utils::math::Vec2<int> pos;
        Node* came_from;
        std::vector<Node*> neighbors;
        int g;
        int f;

        bool operator==(const Node& other) const {
            return pos == other.pos;
        }
    };
    
    std::vector<Node*> nodes;
    std::vector<Node*> open_set;
    std::vector<Node*> closed_set;

    World() : version(0), unk(0), name_len(0), name(), tile_map(), object_map(), nodes(), open_set() {}
    ~World() = default;

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

        // A* stuff
        for (auto& node : nodes) {
            delete node;
        }
        nodes.clear();

        for (int i = 0; i < tile_map.size.x; i++) {
            for (int j = 0; j < tile_map.size.y; j++) {
                Node* node{ new Node{} };
                node->pos = { i, j };
                nodes.emplace_back(node);
            }
        }

        for (auto& node : nodes) {
            node->neighbors.clear();
            if (node->pos.x > 0) {
                node->neighbors.emplace_back(nodes[(node->pos.x - 1) * tile_map.size.y + node->pos.y]);
            }
            if (node->pos.x < tile_map.size.x - 1) {
                node->neighbors.emplace_back(nodes[(node->pos.x + 1) * tile_map.size.y + node->pos.y]);
            }
            if (node->pos.y > 0) {
                node->neighbors.emplace_back(nodes[node->pos.x * tile_map.size.y + node->pos.y - 1]);
            }
            if (node->pos.y < tile_map.size.y - 1) {
                node->neighbors.emplace_back(nodes[node->pos.x * tile_map.size.y + node->pos.y + 1]);
            }
        }
    }
    
    // A* stuff
    std::pair<bool, std::vector<utils::math::Vec2<int>>> astar_search(
        const utils::math::Vec2<int>& start, const utils::math::Vec2<int>& end)
    {
        // Based on new A* pseudocode.
        // https://en.wikipedia.org/wiki/A%2A_search_algorithm#Pseudocode

        open_set.clear();
        closed_set.clear();

        for (auto& node : nodes) {
            node->came_from = nullptr;
            node->g = std::numeric_limits<int>::max();
            node->f = std::numeric_limits<int>::max();
        }

        Tile tile{ tile_map.tiles[end.x + end.y * tile_map.size.x] };
        if (tile.foreground != 0) {
            return { false, {} };
        }

        nodes[start.x * tile_map.size.y + start.y]->g = 0;
        nodes[start.x * tile_map.size.y + start.y]->f = utils::math::manhattan_heuristic<int>(start, end);
        open_set.emplace_back(nodes[start.x * tile_map.size.y + start.y]);

        bool is_found{ false };
        Node *current{};

        while (!open_set.empty()) {
            current = open_set.front();
            if (current->pos == end) {
                is_found = true;
                break;
            }

            std::pop_heap(open_set.begin(), open_set.end(), [](Node* lhs, Node* rhs) {
                return lhs->f > rhs->f;
            });
            open_set.pop_back();
            closed_set.emplace_back(current);

            for (auto& neighbor : current->neighbors) {
                // I added this check because will be stuck in infinite loop, if there is no way to get to the end.
                if (std::find(closed_set.begin(), closed_set.end(), neighbor) != closed_set.end()) {
                    continue;
                }

                tile = tile_map.tiles[neighbor->pos.x + neighbor->pos.y * tile_map.size.x];

                // Is tile walkable?
                if (tile.foreground != 0) {
                    continue;
                }

                int tentative_g = current->g + 1;
                if (tentative_g < neighbor->g) {
                    neighbor->came_from = current;
                    neighbor->g = tentative_g;
                    neighbor->f = neighbor->g + utils::math::manhattan_heuristic<int>(neighbor->pos, end);

                    auto it = std::find(open_set.begin(), open_set.end(), neighbor);
                    if (it == open_set.end()) {
                        open_set.emplace_back(neighbor);
                        std::push_heap(open_set.begin(), open_set.end(), [](Node* lhs, Node* rhs) {
                            return lhs->f > rhs->f;
                        });
                    }
                }
            }
        }

        std::vector<utils::math::Vec2<int>> path;
        path.emplace_back(current->pos);
        while (current->came_from) {
            current = current->came_from;
            path.insert(path.begin(), current->pos);
        }

        return std::make_pair(is_found, path);
    }
};
#pragma pack(pop)
