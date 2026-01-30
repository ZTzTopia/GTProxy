#pragma once
#include <eventpp/eventdispatcher.h>
#include <span>
#include <memory>
#include <map>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <utility>
#include <algorithm>

#include "../packet/payload.hpp"
#include "../packet/packet_id.hpp"
#include "../packet/packet_helper.hpp"

namespace event {
struct Priority {
    static constexpr int8_t Highest = std::numeric_limits<int8_t>::min();
    static constexpr int8_t FairlyHigh = Highest / 2;
    static constexpr int8_t Normal = 0;
    static constexpr int8_t FairlyLow = std::numeric_limits<int8_t>::max() / 2;
    static constexpr int8_t Lowest = std::numeric_limits<int8_t>::max();
};

enum class Type : uint32_t {
    ClientConnect = 0,
    ServerConnect,

    ClientDisconnect,
    ServerDisconnect,

    ClientBoundPacket,
    ServerBoundPacket,

    PacketEventOffset = 0x1000,
    Max
};

enum class Direction {
    ClientBound,
    ServerBound,
};

constexpr uint32_t packet_event_offset() {
    return static_cast<uint32_t>(Type::PacketEventOffset);
}

constexpr bool is_packet_event(const Type t) {
    return static_cast<uint32_t>(t) >= packet_event_offset();
}

constexpr Type packet_event_type(const packet::PacketId id) {
    return static_cast<Type>(packet_event_offset() + static_cast<uint32_t>(id));
}

constexpr packet::PacketId packet_id_from_type(const Type t) {
    return static_cast<packet::PacketId>(static_cast<uint32_t>(t) - packet_event_offset());
}

struct Event {
    Type type;
    mutable bool canceled;

    explicit Event(const Type t)
        : type{ t }
        , canceled{ false }
    {

    }
    virtual ~Event() = default;

    void cancel() const { canceled = true; }
};


struct ConnectionEvent : Event {
    explicit ConnectionEvent(const Type t) : Event{ t } { }
};

struct RawPacketEvent : Event {
    std::span<const std::byte> data;

    RawPacketEvent(const Type t, std::span<const std::byte> d)
        : Event{ t }
        , data{ d }
    { }
};

struct PacketEvent : Event {
    packet::PacketId packet_id;
    std::shared_ptr<packet::IPacket> packet;

    PacketEvent(
        const Type t,
        std::shared_ptr<packet::IPacket> pkt
    )
        : Event{ t }
        , packet_id{ pkt->id() }
        , packet{ std::move(pkt) }
    { }

    [[nodiscard]] bool has_packet() const { return packet != nullptr; }

    template<typename T>
    [[nodiscard]] std::shared_ptr<T> get() const
    {
        if (packet && packet->id() == T::ID) {
            return std::static_pointer_cast<T>(packet);
        }
        return nullptr;
    }

    template<typename T>
    [[nodiscard]] bool is() const
    {
        return packet && packet->id() == T::ID;
    }
};

template<packet::PacketId PacketTypeId>
struct TypedPacketEvent : Event {
    Direction direction;
    std::shared_ptr<packet::IPacket> packet;

    TypedPacketEvent(
        Direction dir,
        std::shared_ptr<packet::IPacket> pkt
    )
        : Event{ packet_event_type(PacketTypeId) }
        , direction{ dir }
        , packet{ std::move(pkt) }
    { }

    [[nodiscard]] Direction get_direction() const { return direction; }

    [[nodiscard]] bool is_client_bound() const { return direction == Direction::ClientBound; }
    [[nodiscard]] bool is_server_bound() const { return direction == Direction::ServerBound; }

    [[nodiscard]] bool has_packet() const { return packet != nullptr; }

    template<typename T>
    [[nodiscard]] std::shared_ptr<T> get() const
    {
        if (packet && packet->id() == T::ID) {
            return std::static_pointer_cast<T>(packet);
        }
        return nullptr;
    }

    template<typename T>
    [[nodiscard]] bool is() const
    {
        return packet && packet->id() == T::ID;
    }
};


struct EventPolicies {
    static Type getEvent(const Event& e) { return e.type; }
    static bool canContinueInvoking(const Event& e) { return !e.canceled; }
};

using BaseDispatcher = eventpp::EventDispatcher<
    Type,
    void(const Event&),
    EventPolicies
>;

class PriorityEventDispatcher {
public:
    using Handle = BaseDispatcher::Handle;
    using Callback = std::function<void(const Event&)>;

    Handle appendListener(const Type event, const Callback& callback, const int8_t priority = Priority::Normal)
    {
        auto& entries = handles_[event];

        for (auto it = entries.begin(); it != entries.end(); ++it) {
            if (it->priority > priority) {
                auto handle = dispatcher_.insertListener(event, callback, it->handle);
                entries.insert(it, {priority, handle});
                return handle;
            }
        }

        auto handle = dispatcher_.appendListener(event, callback);
        entries.push_back({ priority, handle });
        return handle;
    }

    Handle prependListener(const Type event, const Callback& callback)
    {
        return appendListener(event, callback, Priority::Highest);
    }

    bool removeListener(Type event, const Handle& handle)
    {
        if (const auto it = handles_.find(event); it != handles_.end()) {
            auto& entries = it->second;
            std::erase_if(entries, [&handle](const PriorityEntry& e) { return e.handle == handle; });
        }

        return dispatcher_.removeListener(event, handle);
    }

    void dispatch(Type event, const Event& e) const
    {
        dispatcher_.dispatch(event, e);
    }

    void dispatch(const Event& e) const
    {
        dispatcher_.dispatch(e);
    }

    BaseDispatcher& base() { return dispatcher_; }
    const BaseDispatcher& base() const { return dispatcher_; }

private:
    struct PriorityEntry {
        int8_t priority;
        Handle handle;
    };

    BaseDispatcher dispatcher_;
    std::map<Type, std::vector<PriorityEntry>> handles_;
};

using Dispatcher = PriorityEventDispatcher;

class ScopedHandle {
public:
    ScopedHandle() = default;

    ScopedHandle(Dispatcher& d, const Type t, const Dispatcher::Handle h)
        : dispatcher_{ &d }
        , type_{ t }
        , handle_{ h }
    {

    }

    ~ScopedHandle()
    {
        reset();
    }

    void reset()
    {
        if (!dispatcher_) {
            return;
        }

        dispatcher_->removeListener(type_, handle_);
        dispatcher_ = nullptr;
    }

    ScopedHandle(ScopedHandle&& other) noexcept
        : dispatcher_{ other.dispatcher_ }
        , type_{ other.type_ }
        , handle_{ other.handle_ }
    {
        other.dispatcher_ = nullptr;
    }

    ScopedHandle& operator=(ScopedHandle&& other) noexcept
    {
        if (this != &other) {
            reset();
            dispatcher_ = other.dispatcher_;
            type_ = other.type_;
            handle_ = other.handle_;
            other.dispatcher_ = nullptr;
        }

        return *this;
    }

    ScopedHandle(const ScopedHandle&) = delete;
    ScopedHandle& operator=(const ScopedHandle&) = delete;

private:
    Dispatcher* dispatcher_;
    Type type_;
    Dispatcher::Handle handle_;
};
}
