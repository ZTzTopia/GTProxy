local tasks = {}

local function schedule(key, delay, fn)
    tasks[key] = scheduler.schedule(delay, fn)
    return tasks[key]
end

local function schedule_periodic(key, interval, fn, initial_delay)
    if initial_delay then
        tasks[key] = scheduler.schedule_periodic(interval, fn, initial_delay)
    else
        tasks[key] = scheduler.schedule_periodic(interval, fn)
    end
    return tasks[key]
end

logger.info("Scheduler test script loaded")

tasks.oneshot_500 = schedule("oneshot_500", 500, function()
    logger.info("One-shot fired after 500ms")
end)

do
    local counter = 0

    tasks.periodic_300 = schedule_periodic("periodic_300", 300, function()
        counter = counter + 1
        logger.info("Periodic tick: " .. counter .. "/5")

        if counter >= 5 then
            logger.info("Stopping periodic timer")
            return false
        end

        return true
    end)
end

tasks.periodic_delayed = schedule_periodic("periodic_delayed", 500, function()
    logger.info("Periodic with initial delay fired")
    return true
end, 2000)

logger.info("Pending task count: " .. scheduler.pending_count())

schedule("cancel_oneshot", 1500, function()
    if scheduler.is_pending(tasks.oneshot_500) then
        logger.info("One-shot still pending, cancelling...")
        scheduler.cancel(tasks.oneshot_500)
        logger.info("Pending count after cancellation: " .. scheduler.pending_count())
    else
        logger.info("One-shot already completed")
    end
end)

schedule("cancel_all", 4000, function()
    logger.info("Cancelling all " .. scheduler.pending_count() .. " pending tasks")
    scheduler.cancel_all()
    logger.info("Pending count after cancel_all: " .. scheduler.pending_count())
end)

logger.info("Scheduler test script loaded")
