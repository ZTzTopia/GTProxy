local function test_local_player()
    local local_player = world:get_local_player()
    if local_player then
        logger.info("[Test 1] Local player found!")
        logger.info("[Test 1]   Name: " .. local_player.name)
        logger.info("[Test 1]   Net ID: " .. local_player.net_id)
        logger.info("[Test 1]   User ID: " .. local_player.user_id)
        logger.info("[Test 1]   Country: " .. local_player.country_code)
        logger.info("[Test 1]   Position: (" .. local_player.position.x .. ", " .. local_player.position.y .. ")")
        logger.info("[Test 1]   Is Local: " .. tostring(local_player.is_local))
    else
        logger.warn("[Test 1] No local player found (not spawned yet?)")
    end
end

local function test_local_net_id()
    local local_net_id = world:get_local_net_id()
    logger.info("[Test 2] Local Net ID: " .. local_net_id)
end

local function test_list_players()
    local players = world:get_players()
    logger.info("[Test 3] Players in world: " .. #players)

    for net_id, player in pairs(players) do
        logger.info("[Test 3]   Player " .. player.net_id .. " - " .. player.name .. " (Pos: " .. player.position.x .. ", " .. player.position.y .. ")")
    end
end

local function test_find_player()
    local local_net_id = world:get_local_net_id()
    if local_net_id >= 0 then
        local player = world:get_player(local_net_id)
        if player then
            logger.info("[Test 4] Found player by net_id " .. local_net_id .. ": " .. player.name)
        else
            logger.warn("[Test 4] Could not find player by net_id " .. local_net_id)
        end
    else
        logger.warn("[Test 4] Cannot test - no local net_id")
    end
end

local function test_collision_data()
    local local_player = world:get_local_player()
    if local_player then
        local col = local_player.collision
        logger.info("[Test 5] Collision: x=" .. col.x .. ", y=" .. col.y .. ", z=" .. col.z .. ", w=" .. col.w)
        logger.info("[Test 5] Invisible: " .. local_player.invisible .. ", Mod State: " .. local_player.mod_state)
    end
end

test_local_player()
test_local_net_id()
test_list_players()
test_find_player()
test_collision_data()

event.on("OnSpawn", function(ctx)
    if ctx:has_packet() then
        local pkt = ctx:get_packet()
        logger.info("[Event] Player spawned: " .. pkt.name)

        test_local_player()
        test_list_players()
    end
end)

scheduler.schedule_periodic(5000, function()
    logger.info("[Test 7] Periodic player check...")
    test_local_player()
    test_list_players()
    return true
end)

logger.info("World test script loaded")
