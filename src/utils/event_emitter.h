#pragma once
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

namespace utils {
class EventEmitter {
public:
    EventEmitter() = default;
    ~EventEmitter() { unload(); }

    template <typename Callback>
    void load(const std::string& name, Callback&& cb, bool once = false)
    {
        m_events[name].emplace_back(std::forward<Callback>(cb), once);
    }

    template <typename Callback>
    void load_once(const std::string& name, Callback&& cb)
    {
        load(name, std::forward<Callback>(cb), true);
    }

    void unload()
    {
        for (auto& [key, value] : m_events) {
            value.clear();
        }

        m_events.clear();
    }

    void unload(const std::string& name)
    {
        auto it = m_events.find(name);
        if (it != m_events.end()) {
            it->second.clear();
            m_events.erase(it);
        }
    }

    template <typename... Args>
    bool invoke(const std::string& name, Args... args)
    {
        auto it = m_events.find(name);
        if (it != m_events.end()) {
            auto& listeners = it->second;
            if (listeners.empty()) {
                m_events.erase(it);
                return false;
            }

            for (auto& [fn, once] : listeners) {
                std::invoke(fn, std::forward<Args>(args)...);

                if (once) {
                    listeners.erase(
                        std::remove_if(listeners.begin(), listeners.end(), [&](const auto& listener) {
                            return listener.first == fn;
                        }),
                        listeners.end()
                    );
                }
            }

            if (listeners.empty()) {
                m_events.erase(it);
            }

            return true;
        }

        return false;
    }

private:
    using listener = std::vector<std::pair<std::function<void()>, bool>>;
    std::unordered_map<std::string, listener> m_events;
};
}
