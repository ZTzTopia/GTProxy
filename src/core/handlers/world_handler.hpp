#pragma once
#include "../../event/event.hpp"
#include "../../packet/game/world.hpp"

namespace core::handlers {
class WorldHandler {
public:
    explicit WorldHandler(event::Dispatcher& dispatcher);

    WorldHandler(const WorldHandler&) = delete;
    WorldHandler& operator=(const WorldHandler&) = delete;
    WorldHandler(WorldHandler&&) noexcept = delete;
    WorldHandler& operator=(WorldHandler&&) noexcept = delete;
    ~WorldHandler() = default;

private:
    void setup_join_request_handler();
    void setup_on_spawn_handler();
    void setup_on_remove_handler();
    void setup_send_map_data_handler();
    void setup_send_tile_update_data_handler();
    void setup_tile_change_request_handler();
    void setup_item_change_object_handler();

    event::Dispatcher& dispatcher_;
    std::vector<event::ScopedHandle> handles_;
};
}
