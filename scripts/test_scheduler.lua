local tasks = {}

logger.info("[Test 1] Creating one-shot timer (500ms)")
tasks.test1 = scheduler.schedule(500, function()
    logger.info("[Test 1] One-shot fired after 500ms")
end)

logger.info("[Test 2] Creating one-shot timer with closure data (1000ms)")
tasks.test2 = scheduler.schedule(1000, function()
    logger.info("[Test 2] Closure data test: message received")
end)

local counter = 0
logger.info("[Test 3] Creating periodic timer (300ms interval, stops after 5 runs)")
tasks.test3 = scheduler.schedule_periodic(300, function()
    counter = counter + 1
    logger.info("[Test 3] Periodic tick: " .. counter .. "/5")
    if counter >= 5 then
        logger.info("[Test 3] Stopping periodic timer")
        return false  -- Stop the periodic timer
    end
    return true
end)

logger.info("[Test 4] Creating periodic timer with 2s initial delay, 500ms interval")
tasks.test4 = scheduler.schedule_periodic(500, function()
    logger.info("[Test 4] Periodic with initial delay fired")
end, 2000)

local initial_count = scheduler.pending_count()
logger.info("[Test 5] Pending task count: " .. initial_count)

logger.info("[Test 6] Scheduling cancellation of Test 1 after 1500ms")
scheduler.schedule(1500, function()
    if scheduler.is_pending(tasks.test1) then
        logger.info("[Test 6] Test 1 is still pending, cancelling...")
        scheduler.cancel(tasks.test1)
        local count_after_cancel = scheduler.pending_count()
        logger.info("[Test 6] Pending count after cancellation: " .. count_after_cancel)
    else
        logger.info("[Test 6] Test 1 already completed")
    end
end)

logger.info("[Test 7] Scheduling cancel_all() after 4000ms")
scheduler.schedule(4000, function()
    local count_before = scheduler.pending_count()
    logger.info("[Test 7] Cancelling all " .. count_before .. " pending tasks")
    scheduler.cancel_all()
    local count_after = scheduler.pending_count()
    logger.info("[Test 7] Pending count after cancel_all: " .. count_after)
end)

logger.info("Scheduler test script loaded")
