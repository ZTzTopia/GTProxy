#pragma once
#include "config.hpp"
#include "scheduler.hpp"
#include "web_server.hpp"
#include "handlers/connection_handler.hpp"
#include "handlers/forwarding_handler.hpp"
#include "handlers/world_handler.hpp"
#include "../command/command_handler.hpp"
#include "../network/client.hpp"
#include "../network/server.hpp"
#include "../scripting/lua_engine.hpp"
#include "../scripting/script_event_bridge.hpp"
#include "../scripting/script_loader.hpp"
#include "../scripting/script_scheduler.hpp"
#include "../utils/types.hpp"

namespace core {
class Core final : public utils::types::Immobile {
public:
    Core();
    ~Core();

    void run() const;
    void stop() { running_ = false; }

    [[nodiscard]] Config& get_config() { return config_; }

private:
    Config config_;
    bool running_;

    event::Dispatcher dispatcher_;
    std::shared_ptr<Scheduler> scheduler_;

    std::unique_ptr<network::Server> server_;
    std::unique_ptr<network::Client> client_;
    std::unique_ptr<WebServer> web_server_;

    std::unique_ptr<ConnectionHandler> connection_handler_;
    std::unique_ptr<ForwardingHandler> forwarding_handler_;
    std::unique_ptr<WorldHandler> world_handler_;
    std::unique_ptr<command::CommandHandler> command_handler_;

    std::unique_ptr<scripting::LuaEngine> script_engine_;
    std::unique_ptr<scripting::ScriptScheduler> script_scheduler_;
    std::unique_ptr<scripting::ScriptEventBridge> script_event_bridge_;
    std::unique_ptr<scripting::ScriptLoader> script_loader_;
};
}
