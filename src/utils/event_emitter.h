#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <functional>
#include <algorithm>

namespace utils {
    class EventEmitter {
    public:
        EventEmitter() = default;
        ~EventEmitter() { unload(); }

        template<typename Callback>
        void load(const std::string& name, Callback cb, bool once = false) {
            auto f = to_function(cb);
            auto fn = new decltype(f)(to_function(cb));
            m_events[name].emplace_back(static_cast<void*>(fn), once);
        }

        template<typename Callback>
        void load_once(const std::string& name, Callback cb) {
            load(name, cb, true);
        }

        void unload() {
            for (auto& [key, value] : m_events) {
                for (auto& v : value) ::operator delete(v.first);
                value.clear();
            }

            m_events.clear();
        }

        void unload(const std::string& name) {
            auto it = m_events.find(name);
            if (it != m_events.end()) {
                auto& listeners = it->second;
                for (auto& v : listeners) ::operator delete(v.first);
                listeners.clear();
                m_events.erase(it);
            }
        }

        template<typename... Args>
        bool execute(const std::string& name, Args... args) {
            auto it = m_events.find(name);
            if (it != m_events.end()) {
                auto& listeners = it->second;
                if (listeners.empty()) {
                    m_events.erase(it);
                    return false;
                }

                for (auto& v : listeners) {
                    auto fn = static_cast<std::function<void(Args...)>*>(v.first);
                    (*fn)(args...);

                    // TODO: Remove once.
                }

                return true;
            }

            return false;
        }

    private:
        template<typename Callback>
        struct traits : public traits<decltype(&Callback::operator())> {};

        template<typename ClassType, typename R, typename... Args>
        struct traits<R(ClassType:: *)(Args...) const> {
            using fn = std::function<R (Args...)>;
        };

        template<typename Callback>
        typename traits<Callback>::fn to_function(Callback& cb) {
            return static_cast<typename traits<Callback>::fn>(cb);
        }

    private:
        std::unordered_map<std::string, std::vector<std::pair<void *, bool>>> m_events;
    };
}
