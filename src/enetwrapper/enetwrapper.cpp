#include <enet/enet.h>

#include "enetwrapper.h"

namespace enetwrapper {
    bool ENetWrapper::one_time_init() {
#ifdef _WIN32
        if (enet_initialize() != 0)
            return false;

        atexit(enet_deinitialize);
#endif
        return true;
    }
}