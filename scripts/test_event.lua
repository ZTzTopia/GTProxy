event.on("ServerBoundPacket", function(ctx)
    local pkt = ctx:get_packet()
    if not pkt then return end

    if pkt:has_raw_data() then
        logger.debug("[Raw] " .. #pkt.raw .. " bytes, modified=" .. tostring(pkt:is_modified()))
    end

    if pkt.text_parse then
        local action = pkt.text_parse:get("action")
        if action ~= "" then
            logger.info("[Text] Action: " .. action)
        end
    elseif pkt.game_packet then
        logger.info("[Game] Type: " .. tostring(pkt.game_packet.type) .. " NetID: " .. pkt.game_packet.net_id)

        if pkt.game_packet.type == packet.PacketType.PACKET_STATE then
            logger.info(string.format("[State] Pos: %.1f, %.1f", pkt.game_packet.pos_x, pkt.game_packet.pos_y))
        end

        if pkt.game_packet.flags & packet.PacketFlag.PACKET_FLAG_ON_JUMP ~= 0 then
            logger.info("[Game] Player Jumped!")
        end
    elseif pkt.variant then
        logger.info("[Variant] Function: " .. tostring(pkt.variant:get(0)))
    end
end)

event.on("OnSendToServer", function(ctx)
    local pkt = ctx:get_packet()
    if not pkt then return end

    logger.info("[OnSendToServer] Port: " .. tostring(pkt.port) .. " Token: " .. tostring(pkt.token))
    logger.info("[OnSendToServer] Address: " .. pkt.address .. " User: " .. tostring(pkt.user))
end)

event.on("Input", function(ctx)
    local pkt = ctx:get_packet()
    if not pkt then return end

    logger.info("[Chat] " .. pkt.text)

    if pkt.text == "!hello" then
        ctx:cancel()
        local response = LogPacket.new()
        response.msg = "Hello from Lua!"
        send.to_client(response)
    elseif pkt.text == "!rawtest" then
        if pkt:has_raw_data() then
            logger.info("[Raw] " .. #pkt.raw .. " bytes, modified=" .. tostring(pkt:is_modified()))
        end
        ctx:cancel()
    elseif pkt.text == "!modifytest" then
        pkt.text = "modified message"
        pkt:mark_modified()
        logger.info("[Modified] Packet marked as modified")
    end
end)

logger.info("Event test script loaded")
