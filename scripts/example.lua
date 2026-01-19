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

logger.info("Example Lua Scripting Loaded")
