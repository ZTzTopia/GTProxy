command.register("say", "Echo a message to the client", function(ctx)
    if #ctx.args < 1 then
        ctx:reply("Usage: /say <message>")
        return false
    end

    local message = table.concat(ctx.args, " ")

    local log = LogPacket.new()
    log.msg = message
    send.to_client(log)

    return true
end)

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

logger.info("Command test script loaded")
