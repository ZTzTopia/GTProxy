#include <magic_enum.hpp>
#include <spdlog/fmt/bin_to_hex.h>

#include "client.hpp"
#include "../server/server.hpp"
#include "../utils/hash.hpp"
#include "../utils/text_parse.hpp"

namespace client {
Client::Client(core::Core* core)
    : ENetWrapper{ 1 }
    , core_{ core }
{

}

Client::~Client()
{

}

void Client::process()
{
    // Perform client processing here
    ENetWrapper::process();
}

void Client::on_connect(ENetPeer* peer)
{

}

void Client::on_receive(ENetPeer* peer, ENetPacket* packet)
{

}

void Client::on_disconnect(ENetPeer* peer)
{

}
}
