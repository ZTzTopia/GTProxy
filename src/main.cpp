#include "core/core.hpp"
#include "core/logger.hpp"

int main()
{
    try {
        core::Logger logger{};

        spdlog::info(
            "Starting GTProxy ({}.{}.{})",
            GTPROXY_VERSION_MAJOR,
            GTPROXY_VERSION_MINOR,
            GTPROXY_VERSION_PATCH
        );

        core::Core core{};
        core.run();
    }
    catch (const spdlog::spdlog_ex& ex) {
        spdlog::error("Log initialization failed: {}", ex.what());
        return 1;
    }
    catch (const std::runtime_error& err) {
        spdlog::error("Runtime error: {}", err.what());
        return 1;
    }
    catch (const std::exception& ex) {
        spdlog::error("Exception: {}", ex.what());
        return 1;
    }

    return 0;
}
