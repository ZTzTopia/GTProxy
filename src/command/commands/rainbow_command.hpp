#pragma once
#include <array>
#include <atomic>
#include <chrono>
#include <string>
#include <fmt/format.h>

#include "../command.hpp"
#include "../../packet/packet_helper.hpp"
#include "../../packet/game/player.hpp"
#include "../../packet/message/chat.hpp"
#include "../../world/world.hpp"

namespace command {
class RainbowCommand final : public ICommand {
public:
    [[nodiscard]] std::string_view name() const override { return "rainbow"; }
    [[nodiscard]] std::string description() const override { return "Toggle rainbow skin effect"; }

    Result execute(const Context& ctx) override
    {
        if (
            const auto net_id{ world::World::instance().get_local_net_id() };
            net_id < 0
        ) {
            packet::message::Log log{};
            log.msg = "`4Error: ``You are not in a world";
            packet::PacketHelper::write(log, ctx.server);
            return Result::Failed;
        }

        auto parse_result{ Result::Success };
        const auto interval{ parse_interval(ctx, parse_result) };

        if (parse_result != Result::Success) {
            return parse_result;
        }

        if (is_active_.load(std::memory_order_acquire)) {
            toggle_off(ctx);
            return Result::Success;
        }

        toggle_on(ctx, interval);
        return Result::Success;
    }

private:
    static constexpr std::chrono::milliseconds DEFAULT_INTERVAL{ 200 };
    static constexpr std::chrono::milliseconds MIN_INTERVAL{ 50 };
    static constexpr std::chrono::milliseconds MAX_INTERVAL{ 5000 };

    static inline core::TaskId current_task_id_{ core::INVALID_TASK_ID };
    static inline std::atomic<bool> is_active_{ false };
    static inline std::size_t current_color_index_{ 0 };

    static constexpr std::array<std::uint32_t, 15> RAINBOW_COLORS{
        0xFF0000FF, // Red
        0xFF3300FF, // Red-Orange
        0xFF6600FF, // Orange
        0xFF9900FF,
        0xFFCC00FF, // Yellow-Orange
        0xFFFF00FF, // Yellow
        0x99FF00FF,
        0x00FF00FF, // Green
        0x00FF99FF,
        0x00FFFFFF, // Cyan
        0x0099FFFF,
        0x0000FFFF, // Blue
        0x000099FF,
        0x6600FFFF, // Indigo
        0x9900FFFF  // Purple
    };

    static std::chrono::milliseconds parse_interval(
        const Context& ctx,
        Result& result
    ) {
        result = Result::Success;

        if (ctx.args.empty()) {
            return DEFAULT_INTERVAL;
        }

        const auto& speed_str{ ctx.args[0] };

        std::uint64_t speed_ms{};
        const auto [ptr, ec]{
            std::from_chars(
                speed_str.data(),
                speed_str.data() + speed_str.size(),
                speed_ms
            )
        };

        if (ec != std::errc{}) {
            packet::message::Log log{};
            log.msg = fmt::format("`4Error: ``Invalid speed value '{}'", speed_str);
            packet::PacketHelper::write(log, ctx.server);
            result = Result::InvalidArguments;
            return DEFAULT_INTERVAL;
        }

        if (speed_ms < static_cast<std::uint64_t>(MIN_INTERVAL.count())) {
            packet::message::Log log{};
            log.msg = fmt::format(
                "`4Error: ``Speed too slow (min {}ms)",
                MIN_INTERVAL.count()
            );
            packet::PacketHelper::write(log, ctx.server);
            result = Result::InvalidArguments;
            return DEFAULT_INTERVAL;
        }

        if (speed_ms > static_cast<std::uint64_t>(MAX_INTERVAL.count())) {
            packet::message::Log log{};
            log.msg = fmt::format(
                "`4Error: ``Speed too fast (max {}ms)",
                MAX_INTERVAL.count()
            );
            packet::PacketHelper::write(log, ctx.server);
            result = Result::InvalidArguments;
            return DEFAULT_INTERVAL;
        }

        return std::chrono::milliseconds{ speed_ms };
    }

    static void toggle_on(
        const Context& ctx,
        std::chrono::milliseconds interval
    ) {
        const auto task_id{ ctx.scheduler->schedule_periodic(
            [&server = ctx.server] {
                send_next_color(server);
            },
            interval,
            "rainbow",
            core::TaskPriority::Normal
        )};

        current_task_id_ = task_id;
        is_active_.store(true, std::memory_order_release);
        current_color_index_ = 0;

        send_next_color(ctx.server);

        packet::message::Log log{};
        log.msg = fmt::format(
            "`2Rainbow effect started ``(speed: {}ms)",
            interval.count()
        );
        packet::PacketHelper::write(log, ctx.server);
    }

    static void toggle_off(const Context& ctx)
    {
        if (current_task_id_ != core::INVALID_TASK_ID) {
            ctx.scheduler->cancel(current_task_id_);
            current_task_id_ = core::INVALID_TASK_ID;
        }

        ctx.scheduler->cancel_by_tag("rainbow");

        is_active_.store(false, std::memory_order_release);

        packet::message::Log log{};
        log.msg = "`2Rainbow effect stopped";
        packet::PacketHelper::write(log, ctx.server);
    }

    static void send_next_color(network::Server& server)
    {
        const auto net_id{ world::World::instance().get_local_net_id() };
        if (net_id < 0) {
            return;
        }

        const auto color{ RAINBOW_COLORS[current_color_index_] };

        packet::game::OnChangeSkin pkt{};
        pkt.net_id = net_id;
        pkt.skin_code = color;
        packet::PacketHelper::write(pkt, server);

        current_color_index_ = (current_color_index_ + 1) % RAINBOW_COLORS.size();
    }
};
}
