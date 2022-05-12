#include <fmt/chrono.h>
#include <fmt/ranges.h>

#include "command_handler.h"
#include "../config.h"
#include "../server/server.h"
#include "../utils/dialog_builder.h"
#include "../utils/hash.h"
#include "../utils/textparse.h"

namespace command {
    CommandHandler::CommandHandler(server::Server *server)
        : m_server(server)
    {
        m_commands.push_back(
            new Command({ "help", { "?" }, "Displays this help message" },
                [this](const CommandCallContext& command_call_context, const std::vector<std::string>& args)
            {
                if (!args.empty()) {
                    auto it = std::find_if(m_commands.begin(), m_commands.end(), [args](const Command* command) {
                        if (command->get_name() != args[0]) {
                            std::vector<std::string> aliases = command->get_aliases();
                            return std::find(aliases.cbegin(), aliases.cend(), args[0]) != aliases.cend();
                        }

                        return command->get_name() == args[0];
                    });

                    if (it != m_commands.end())
                        command_call_context.local_peer->send_log((*it)->get_description());
                    else
                        command_call_context.local_peer->send_log("`4Unknown command. ``Enter `$!help`` for a list of valid commands.");
                    return;
                }

                std::string commands{ ">> Commands: " };
                for (auto &command : m_commands) {
                    commands.append(Config::get().config()["command"]["prefix"]);
                    commands.append(command->get_name());
                    commands.push_back(' ');
                }

                commands.pop_back();
                command_call_context.local_peer->send_log(commands);
            })
        );
        m_commands.push_back(
            new Command({ "warp", {}, "Warps you to a world" },
                [](const CommandCallContext& command_call_context, const std::vector<std::string> &args)
            {
                if (args.empty()) {
                    command_call_context.local_peer->send_log("`4Usage: ``!warp <world name>");
                    return;
                }

                if (args[0] == "exit") {
                    command_call_context.local_peer->send_log("`4You cannot warp to the exit world.");
                    return;
                }

                if (args[0].size() > 23) {
                    command_call_context.local_peer->send_log("`4World name too long, try again.");
                    return;
                }

                command_call_context.server_peer->send_packet(player::NET_MESSAGE_GAME_MESSAGE, "action|quit_to_exit");
                command_call_context.local_peer->send_log(fmt::format("Warping to {}...", args[0]));
                command_call_context.server_peer->send_packet(
                    player::NET_MESSAGE_GAME_MESSAGE,
                    fmt::format(
                        "action|join_request\n"
                        "name|{}\n"
                        "invitedWorld|0", args[0]));
            })
        );
        m_commands.push_back(
            new Command({ "list", {}, "Used for debugging" },
                [](const CommandCallContext& command_call_context, const std::vector<std::string> &args)
            {
                if (args.empty()) {
                    command_call_context.local_peer->send_log("`4Usage: ``!list <player|inventory|world|tile|tileextra|object>");
                    return;
                }

                std::string lowercase_args{ args[0] };
                std::transform(lowercase_args.begin(), lowercase_args.end(), lowercase_args.begin(), ::tolower);

                dialog_builder db;
                db.set_default_color('o')
                    ->add_label_with_icon(fmt::format("`wList {}``", lowercase_args), 18, dialog_builder::LEFT, dialog_builder::BIG)
                    ->add_spacer();

                switch (utils::fnv1a_hash(lowercase_args)) {
                    case "player"_fh:
                        db.add_smalltext(fmt::format("total: `w{}``", command_call_context.remote_player.size()));
                        for (auto& player : command_call_context.remote_player) {
                            db.add_smalltext(fmt::format("net id: `w{}``", player.second->get_net_id()));
                        }
                        break;
                    case "inv"_fh:
                    case "inventory"_fh: {
                        PlayerItems* player_items{ command_call_context.local_player->get_items() };
                        db.add_smalltext(fmt::format(
                            "version: `w{}``, max: `w{}``, total: `w{}``",
                            player_items->version, player_items->max_size, player_items->size));
                        for (auto& inventory : player_items->items) {
                            db.add_smalltext(fmt::format(
                                "id: `w{}``, [count, unused]: `w{}``", inventory.first, inventory.second));
                        }
                        break;
                    }
                    case "world"_fh: {
                        World* world{ command_call_context.local_player->get_world() };
                        db.add_smalltext(fmt::format(
                            "version: `w{}``, unknown: `w{}``, name: `w{}`` (`w{}``)",
                            world->version, world->unk, world->name, world->name_len));
                        db.add_smalltext(fmt::format(
                            "tile size: `w{}``, tile count: `w{}``",
                            world->tile_map.size.to_pair(), world->tile_map.count));
                        db.add_smalltext(fmt::format(
                            "object count: `w{}``, drop id: `w{}``",
                            world->object_map.count, world->object_map.drop_id));
                        break;
                    }
                    case "tile"_fh: {
                        World* world{ command_call_context.local_player->get_world() };
                        for (auto& tile : world->tile_map.tiles) {
                            db.add_smalltext(fmt::format(
                                "foreground: `w{}``, background: `w{}``, parent tile: `w{}``, flag: `w{}``",
                                tile.foreground, tile.background, tile.parent_tile, tile.flag));
                        }
                        break;
                    }
                    case "extratile"_fh:
                    case "tileextra"_fh: {
                        World* world{ command_call_context.local_player->get_world() };
                        for (auto& tile : world->tile_map.tiles) {
                            if (tile.foreground == 0 && tile.background == 0)
                                continue;

                            if ((tile.flag & Tile::EXTRA) != Tile::EXTRA)
                                continue;

                            db.add_smalltext(fmt::format(
                                "foreground: `w{}``, background: `w{}``",
                                tile.foreground, tile.background));

                            db.add_smalltext(fmt::format(
                                "extra type: `w[{}] ({})``",
                                tile.tile_extra.type_to_string(), tile.tile_extra.type));
                        }
                        break;
                    }
                    case "object"_fh: {
                        World* world{ command_call_context.local_player->get_world() };
                        for (auto& object : world->object_map.objects) {
                            db.add_smalltext(fmt::format(
                                "item id: `w{}``, [x, y]: `w{}``, amount: `w{}``, flags: `w{}``",
                                object.item_id, object.pos.to_pair(), object.amount, object.flags));
                        }
                        break;
                    }
                    default:
                        command_call_context.local_peer->send_log("`4Usage: ``!list <player|inventory|world|tile|tileextra|object>");
                        return;
                }

                db.end_dialog("", "", "Close");
                command_call_context.local_peer->send_variant({ "OnDialogRequest", db.get() });
            })
        );
        m_commands.push_back(
            new Command({ "fastdrop", { "fd" }, "When `Drop` Button Clicked instantly drop!" },
                [](const CommandCallContext& command_call_context, const std::vector<std::string> &args)
            {
                command_call_context.local_player->toggle_flags(player::eFlag::FAST_DROP);
                if (command_call_context.local_player->has_flags(player::eFlag::FAST_DROP))
                    command_call_context.local_peer->send_log("Fast drop: `2enabled``!");
                else
                    command_call_context.local_peer->send_log("Fast drop: `4disabled``!");
            })
        );
        m_commands.push_back(
            new Command({ "fastwrench", { "fw", "wrench" }, "Fast pull, kick, ban when wrench clicked" },
                [](const CommandCallContext& command_call_context, const std::vector<std::string> &args)
            {
                if (args.empty()) {
                    command_call_context.local_peer->send_log("`4Usage: ``!fastwrench <pull|kick|ban>");

                    command_call_context.local_player->unset_flags(player::eFlag::FAST_WRENCH_PULL);
                    command_call_context.local_player->unset_flags(player::eFlag::FAST_WRENCH_KICK);
                    command_call_context.local_player->unset_flags(player::eFlag::FAST_WRENCH_BAN);

                    command_call_context.local_peer->send_log("Fast wrench: `4disabled``!");
                    return;
                }

                switch (utils::fnv1a_hash(args[0])) {
                    case "pull"_fh:
                        command_call_context.local_player->set_flags(player::eFlag::FAST_WRENCH_PULL);
                        break;
                    case "kick"_fh:
                        command_call_context.local_player->set_flags(player::eFlag::FAST_WRENCH_KICK);
                        break;
                    case "ban"_fh:
                        command_call_context.local_player->set_flags(player::eFlag::FAST_WRENCH_BAN);
                        break;
                    default:
                        command_call_context.local_peer->send_log("`4Usage: ``!fastwrench <pull|kick|ban>");
                        return;
                }

                command_call_context.local_peer->send_log(fmt::format("Fast wrench: `2{}``!", args[0]));
            })
        );
    }

    CommandHandler::~CommandHandler() {
        for (auto &command : m_commands)
            delete command;
        m_commands.clear();
    }

    bool CommandHandler::handle(const std::string &string) {
        std::vector<std::string> args = utils::TextParse::string_tokenize(string, " ");
        if (args.empty())
            return false;

        if (!args[0].starts_with(Config::get().config()["command"]["prefix"].get<std::string>()))
            return false;

        std::string command_name = args[0].substr(1);
        std::transform(command_name.cbegin(), command_name.cend(), command_name.begin(), ::tolower);
        args.erase(args.cbegin());

        for (auto &command : m_commands) {
            if (command->get_name() != command_name) {
                std::vector<std::string> aliases = command->get_aliases();
                auto it = std::find(aliases.cbegin(), aliases.cend(), command_name);
                if (it == aliases.cend())
                    continue;
            }

            m_server->get_player()->send_log(fmt::format("`6{}``", string));
            command->call({
                m_server->get_player(),
                m_server->get_client_player(),
                m_server->get_client()->get_local_player(),
                m_server->get_client()->get_remote_players() }, args);
            return true;
        }
        return false;
    }
}
