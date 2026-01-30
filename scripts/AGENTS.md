# GTProxy - Lua Scripting Agent Guidelines

## Overview

Scripts in `scripts/` are loaded automatically at startup.
The scripting engine uses sol2 for C++/Lua bindings.

## Global APIs

| API       | Purpose                                   |
|-----------|-------------------------------------------|
| `logger`  | Logging functions                         |
| `event`   | Event system for packet/connection events |
| `send`    | Send packet structs to client or server   |
| `packet`  | Packet types, enums, and raw send helpers |
| `command` | Register custom proxy commands            |

## Logger

```lua
logger.info("Message")
logger.debug("Debug info")
logger.warn("Warning message")
logger.error("Error message")
```

## Events

### Registering Event Handlers

```lua
event.on("server:BoundPacket", function(ctx)
    if ctx:has_packet() then
        local pkt = ctx:parse_packet()
        -- Process decoded packet
    else
        local data = ctx:get_data()
        -- Process raw packet bytes
    end
    return true  -- Return false or call ctx:cancel() to block
end)
```

### Available Events

| Event                  | Description                    |
|------------------------|--------------------------------|
| `"client:Connect"`     | Game client connected to proxy |
| `"client:Disconnect"`  | Game client disconnected       |
| `"server:Connect"`     | Proxy connected to game server |
| `"server:Disconnect"`  | Proxy disconnected from server |
| `"client:BoundPacket"` | Packet from server to client   |
| `"server:BoundPacket"` | Packet from client to server   |

## Command Registration

You can register commands from scripts in two ways. The `description` parameter is optional.

```lua
-- With description (name, description, callback)
command.register("say", "Echo a message", function(ctx)
    if #ctx.args < 1 then
        logger.error("Usage: /say <message>")
        return false
    end
    logger.info(table.concat(ctx.args, " "))
    return true
end)

-- Without description (name, callback) — a default description is used
command.register("echo", function(ctx)
    if #ctx.args < 1 then
        ctx:reply("Usage: /echo <message>")
        return false
    end

    local message = table.concat(ctx.args, " ")
    local log = LogPacket.new()
    log.msg = message
    send.to_client(log)
    return true
end)
```

### Command Context

| Field      | Type   | Description              |
|------------|--------|--------------------------|
| `ctx.args` | table  | Parsed command arguments |
| `ctx.raw`  | string | Raw input string         |

## Sending Packets

### Using Packet Structs

```lua
-- Send packet struct to client
local log = LogPacket.new()
log.msg = "Hello from Lua!"
send.to_client(log)

-- Send packet struct to server
local join = JoinRequestPacket.new()
join.world_name = "START"
send.to_server(join)
```

### Using Raw/Text Functions

```lua
-- Send raw bytes (table of integers 0-255)
packet.send_raw({ 0x03, 0x00, 0x00, 0x00 }, true)  -- to server
packet.send_raw({ 0x03, 0x00, 0x00, 0x00 }, false) -- to client

-- Send text packet (uses NET_MESSAGE_GAME_MESSAGE by default)
packet.send_text("action|respawn", true)

-- Send text packet with specific message type
packet.send_text("action|respawn", true, packet.NET_MESSAGE_GENERIC_TEXT)

-- Send TextParse as packet
local tp = TextParse.new()
tp:add("action", "respawn")
packet.send_text_parse(tp, true)
```

### Message Type Constants

```lua
packet.NET_MESSAGE_GENERIC_TEXT  -- Generic text message
packet.NET_MESSAGE_GAME_MESSAGE  -- Game message (default)
packet.NET_MESSAGE_GAME_PACKET   -- Game packet
```
