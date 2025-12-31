#pragma once
#include <eventpp/eventdispatcher.h>
#include <span>

#include "../packet/packet_variant.hpp"
#include "../utils/text_parse.hpp"

namespace event {
struct Priority {
    static constexpr int8_t Highest = std::numeric_limits<int8_t>::min();
    static constexpr int8_t FairlyHigh = Highest / 2;
    static constexpr int8_t Normal = 0;
    static constexpr int8_t FairlyLow = std::numeric_limits<int8_t>::max() / 2;
    static constexpr int8_t Lowest = std::numeric_limits<int8_t>::max();
};

enum class Type {
    ClientConnect,
    ServerConnect,

    ClientDisconnect,
    ServerDisconnect,

    ClientBoundPacket,
    ServerBoundPacket,
};

struct Event {
    Type type;
    mutable bool canceled{false};

    explicit Event(const Type t) : type{ t } { }
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

template<typename T>
struct PacketEvent : Event {
    std::shared_ptr<T> packet;

    PacketEvent(
        const Type t,
        std::shared_ptr<T> pkt
    )
        : Event{ t }
        , packet{ std::move(pkt) }
    { }
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
}
