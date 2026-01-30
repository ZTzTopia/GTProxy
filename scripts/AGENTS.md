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
| `scheduler`| Schedule timed tasks and callbacks        |
| `world`   | Access world and player state             |

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

## Scheduler

The scheduler allows you to schedule tasks to run after a delay or at regular intervals. All times are in milliseconds.

### One-Shot Tasks

Schedule a callback to run once after a specified delay:

```lua
local task_id = scheduler.schedule(1000, function()
    logger.info("This runs once after 1 second")
end)
```

### Periodic Tasks

Schedule a callback to run repeatedly:

```lua
local task_id = scheduler.schedule_periodic(500, function()
    logger.info("This runs every 500ms")
    return true  -- Return false to stop the periodic task
end)
```

### Periodic with Initial Delay

Schedule a periodic task with a different initial delay:

```lua
local task_id = scheduler.schedule_periodic(1000, function()
    logger.info("Runs every 1s, starts after 2s")
    return true
end, 2000)  -- 2000ms initial delay
```

### Cancellation

Cancel individual tasks or all tasks:

```lua
scheduler.cancel(task_id)        -- Cancel specific task
scheduler.cancel_all()           -- Cancel all pending tasks
```

### Task Information

Check task status:

```lua
if scheduler.is_pending(task_id) then
    logger.info("Task is still pending")
end

local count = scheduler.pending_count()
logger.info("There are {} pending tasks", count)
```

### Coroutine Integration

Use coroutines for clean async code:

```lua
function sleep(ms)
    local co = coroutine.running()
    scheduler.schedule(ms, function()
        coroutine.resume(co)
    end)
    coroutine.yield()
end

event.on("server:Connect", function()
    logger.info("Connected!")
    sleep(1000)  -- Non-blocking delay
    logger.info("1 second later!")
end)
```

## World & Player

The `world` global provides access to the current world state and player information.

### Accessing Players

```lua
-- Get the local player (your character)
local local_player = world:get_local_player()
if local_player then
    logger.info("My name: {}", local_player.name)
    logger.info("My position: ({}, {})", local_player.position.x, local_player.position.y)
end

-- Get all players in the world
local all_players = world:get_players()
for net_id, player in pairs(all_players) do
    logger.info("Player {}: {} at ({}, {})", net_id, player.name, player.position.x, player.position.y)
end

-- Get a specific player by net_id
local player = world:get_player(12345)
if player then
    logger.info("Found player: {}", player.name)
end

-- Get the local player's net ID
local my_net_id = world:get_local_net_id()
```

### Player Properties

| Property        | Type          | Description                             |
|-----------------|---------------|-----------------------------------------|
| `net_id`       | number        | Network ID of the player                |
| `user_id`       | number        | User ID (unique ID)                   |
| `name`          | string        | Player name                            |
| `country_code`   | string        | Two-letter country code (e.g., "US")     |
| `position`       | Vec2          | {x, y} coordinates in world           |
| `collision`      | Vec4          | {x, y, z, w} collision dimensions      |
| `invisible`      | number        | Invisibility level                       |
| `mod_state`      | number        | Moderator state                        |
| `supermod_state`  | number        | Super moderator state                  |
| `is_local`       | boolean       | True if this is the local player        |

### Example: Track Player Position

```lua
local last_positions = {}

event.on("server:OnSpawn", function(ctx)
    if ctx:has_packet() then
        local pkt = ctx:get_packet()
        local player = world:get_player(pkt.net_id)
        if player then
            last_positions[pkt.net_id] = player.position
            logger.info("Player {} spawned at ({}, {})", player.name, player.position.x, player.position.y)
        end
    end
end)

scheduler.schedule_periodic(1000, function()
    for net_id, player in pairs(world:get_players()) do
        if player.is_local then
            logger.info("I am at ({}, {})", player.position.x, player.position.y)
        end
    end
    return true
end)
```
