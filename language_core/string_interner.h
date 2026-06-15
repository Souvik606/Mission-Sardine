#pragma once
#include <string>
#include <unordered_set>
#include <mutex>

class StringInterner {
public:
    static const std::string* intern(const std::string& str) {
        static StringInterner instance;
        std::lock_guard<std::mutex> lock(instance.mutex_);
        auto [it, inserted] = instance.pool_.insert(str);
        return &(*it);
    }

private:
    std::unordered_set<std::string> pool_;
    std::mutex mutex_;
};
