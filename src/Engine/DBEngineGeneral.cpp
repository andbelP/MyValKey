#include <chrono>
#include <cstddef>
#include <expected>
#include <list>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "Engine/DBEngine.hpp"
#include "Engine/StorageTypes.hpp"
#include "Engine/TypesSizes.hpp"
#include "ErrorsHandling/Error.hpp"
#include "Glob/GlobMatcher.hpp"

namespace keyval {

std::expected<void, error::Error> DBEngine::Del(std::string_view key) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));
    if (it == storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }
    storage_.erase(it);
    return {};
}

std::expected<bool, error::Error> DBEngine::Exists(std::string_view key) {
    DeleteIfExpired(key);

    return storage_.contains(std::string(key));
}

std::expected<StorageType, error::Error> DBEngine::Type(std::string_view key) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));
    if (it == storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }
    return it->second.type;
}

std::expected<void, error::Error> DBEngine::Expire(
    std::string_view key, std::chrono::seconds seconds) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));
    if (it == storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }
    it->second.expire_time = std::chrono::steady_clock::now() + seconds;
    return {};
}

std::expected<std::optional<std::size_t>, error::Error> DBEngine::GetTTL(
    std::string_view key) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));
    if (it == storage_.end() ||
        (it->second.expire_time.has_value() &&
         it->second.expire_time.value() <= std::chrono::steady_clock::now())) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }
    if (!it->second.expire_time.has_value()) {
        return std::nullopt;
    }
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(
               it->second.expire_time.value() - now)
        .count();
}

std::expected<std::vector<std::string>, error::Error> DBEngine::Keys(
    std::string_view pattern) {
    std::vector<std::string> result;
    DeleteIfExpiredAll();

    for (const auto& entry : storage_) {
        if (GlobMatcher::Match(pattern, entry.first)) {
            result.push_back(entry.first);
        }
    }
    return result;
}

void DBEngine::FlushDb() { storage_.clear(); }

std::size_t DBEngine::EntryCount() {
    DeleteIfExpiredAll();
    return storage_.size();
}

std::expected<std::size_t, error::Error> DBEngine::MemoryUsageOfKey(
    std::string_view key) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));
    if (it == storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }
    auto& entry = it->second;
    return MemoryUsageOfEntry(entry);
}

std::expected<std::size_t, error::Error> DBEngine::MemoryUsageOfEntry(
    const StorageEntry& entry) {
    switch (entry.type) {
        case StorageType::kString:
            return GetSizeOfString(std::get<std::string>(entry.value));
        case StorageType::kList:
            return GetSizeOfList(std::get<std::list<std::string>>(entry.value));
        case StorageType::kSet:
            return GetSizeOfSet(
                std::get<std::unordered_set<std::string>>(entry.value));
        case StorageType::kGeo:
            return GetSizeOfGeoEntry(std::get<GeoEntry>(entry.value));
    }
    return std::unexpected(error::Error{error::ErrorCode::kUndefinedError,
                                        "Unknown storage type"});
}

bool DBEngine::DeleteIfExpired(std::string_view key) {
    auto it = storage_.find(std::string(key));
    if (it == storage_.end()) {
        return false;
    }
    auto& entry = it->second;
    if (entry.expire_time.has_value() &&
        entry.expire_time.value() <= std::chrono::steady_clock::now()) {
        storage_.erase(it);
        return true;
    }
    return false;
}

void DBEngine::DeleteIfExpiredAll() {
    for (auto it = storage_.begin(); it != storage_.end();) {
        if (it->second.expire_time.has_value() &&
            it->second.expire_time.value() <=
                std::chrono::steady_clock::now()) {
            it = storage_.erase(it);
        } else {
            ++it;
        }
    }
}

std::expected<void, error::Error> DBEngine::SetMaxMemoryUsage(
    std::size_t bytes) {
    if (bytes == 0) {
        max_memory_usage_ = std::nullopt;
        return {};
    }
    if (bytes < GetCurrentMemoryUsage()) {
        return std::unexpected(error::Error{
            error::ErrorCode::kInvalidCommand,
            "New max memory usage is less than current memory usage"});
    }
    max_memory_usage_ = bytes;
    return {};
}

std::optional<std::size_t> DBEngine::GetMaxMemoryUsage() const {
    return max_memory_usage_;
}

std::size_t DBEngine::GetCurrentMemoryUsage() {

    DeleteIfExpiredAll();

    std::size_t current_memory_usage = 0;
    for (auto& el : storage_) {
        auto result = MemoryUsageOfEntry(el.second);
        if (result) {
            current_memory_usage += result.value();
        }
    }
    return current_memory_usage;
}

bool DBEngine::CanAddBytes(std::size_t cnt) {
    if (!max_memory_usage_.has_value()) {
        return true;
    }
    auto current_usage = GetCurrentMemoryUsage();
    if (current_usage + cnt > max_memory_usage_.value()) {
        return false;
    }
    return true;
}

}  // namespace keyval
