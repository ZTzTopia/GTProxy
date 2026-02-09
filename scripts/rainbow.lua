local rainbow = {}
local is_active = false
local current_task_id = nil
local current_color_index = 0

local RAINBOW_COLORS = {
    0xFF0000FF, -- Red
    0xFF3300FF, -- Red-Orange
    0xFF6600FF, -- Orange
    0xFF9900FF,
    0xFFCC00FF, -- Yellow-Orange
    0xFFFF00FF, -- Yellow
    0x99FF00FF,
    0x00FF00FF, -- Green
    0x00FF99FF,
    0x00FFFFFF, -- Cyan
    0x0099FFFF,
    0x0000FFFF, -- Blue
    0x000099FF,
    0x6600FFFF, -- Indigo
    0x9900FFFF  -- Purple
}

local DEFAULT_INTERVAL = 200
local MIN_INTERVAL = 50
local MAX_INTERVAL = 5000

local function send_next_color()
    local net_id = world:get_local_net_id()
    if net_id < 0 then
        if is_active then
            stop_rainbow()
            logger.info("Rainbow effect stopped (exited world)")
        end
        return
    end

    local color = RAINBOW_COLORS[current_color_index + 1]

    local pkt = OnChangeSkinPacket.new()
    pkt.net_id = net_id
    pkt.skin = color
    send.to_client(pkt)

    current_color_index = (current_color_index + 1) % #RAINBOW_COLORS
end

local function toggle_on(ctx, interval)
    current_task_id = scheduler.schedule_periodic(interval, function()
        send_next_color()
        return true
    end)

    is_active = true
    current_color_index = 0

    send_next_color()

    ctx:reply("`2Rainbow effect started ``(speed: {}ms)", interval)
end

local function stop_rainbow()
    if current_task_id ~= nil and scheduler.is_pending(current_task_id) then
        scheduler.cancel(current_task_id)
        current_task_id = nil
    end

    is_active = false
end

local function toggle_off(ctx)
    stop_rainbow()
    ctx:reply("`2Rainbow effect stopped")
end

local function parse_interval(args)
    if #args == 0 then
        return DEFAULT_INTERVAL, nil
    end

    local speed_str = args[1]
    local speed_ms = tonumber(speed_str)

    if speed_ms == nil then
        return DEFAULT_INTERVAL, "Invalid speed value '" .. speed_str .. "'"
    end

    if speed_ms < MIN_INTERVAL then
        return DEFAULT_INTERVAL, "Speed too slow (min " .. MIN_INTERVAL .. "ms)"
    end

    if speed_ms > MAX_INTERVAL then
        return DEFAULT_INTERVAL, "Speed too fast (max " .. MAX_INTERVAL .. "ms)"
    end

    return speed_ms, nil
end

command.register("rainbow", "Toggle rainbow skin effect", function(ctx)
    local net_id = world:get_local_net_id()
    if net_id < 0 then
        ctx:reply("`4Error: ``You are not in a world")
        return false
    end

    if is_active then
        toggle_off(ctx)
        return true
    end

    local interval, error_msg = parse_interval(ctx.args)
    if error_msg then
        ctx:reply("`4Error: ``" .. error_msg)
        return false
    end

    toggle_on(ctx, interval)
    return true
end)

logger.info("Rainbow command loaded")
