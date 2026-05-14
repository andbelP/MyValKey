#pragma once

#include <chrono>
#include <cstdint>
#include <expected>
#include <list>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "Engine/StorageTypes.hpp"
#include "ErrorsHandling/Error.hpp"

namespace keyval {

using ValueType = std::variant<std::string, std::list<std::string>,
                               std::unordered_set<std::string>>;

struct StorageEntry {
    ValueType value;
    StorageType type;
    std::optional<std::chrono::steady_clock::time_point> expire_time;

    StorageEntry() = default;

    StorageEntry(ValueType&& val, StorageType t)
        : value(std::move(val)),
          type(t) {}  // TODO: add forwarding reference to constructor
};

class DBEngine {
    std::unordered_map<std::string, StorageEntry> storage_;

    bool DeleteIfExpired(std::string_view it);
    void DeleteIfExpiredAll();


   public:
    std::expected<void, error::Error> Set(std::string_view key,
                                          std::string_view value);

    std::expected<std::string, error::Error> Get(std::string_view key);

    std::expected<std::size_t, error::Error> StrLen(std::string_view key);

    std::expected<void, error::Error> LPush(std::string_view key,
                                            std::string_view value);

    std::expected<void, error::Error> RPush(std::string_view key,
                                            std::string_view value);

    std::expected<std::string, error::Error> LPop(std::string_view key);

    std::expected<std::string, error::Error> RPop(std::string_view key);

    std::expected<std::size_t, error::Error> LLen(std::string_view key);

    std::expected<std::vector<std::string>, error::Error> LRange(
        std::string_view key, std::ptrdiff_t start, std::ptrdiff_t stop);

    std::expected<std::string, error::Error> LIndex(std::string_view key,
                                                    std::ptrdiff_t index);

    std::expected<void, error::Error> LSet(std::string_view key,
                                           std::ptrdiff_t index,
                                           std::string_view value);

    std::expected<void, error::Error> LInsert(std::string_view key,
                                                     std::ptrdiff_t index,
                                                     std::string_view value);

    std::expected<void, error::Error> SAdd(std::string_view key,
                                           std::string_view member);

    std::expected<void, error::Error> SRem(std::string_view key,
                                           std::string_view member);

    std::expected<bool, error::Error> SIsMember(std::string_view key,
                                                std::string_view member);

    std::expected<std::unordered_set<std::string>, error::Error> SMembers(
        std::string_view key);

    std::expected<std::size_t, error::Error> SCard(std::string_view key);

    std::expected<std::unordered_set<std::string>, error::Error> SUnion(
        std::span<const std::string_view> keys);

    std::expected<std::unordered_set<std::string>, error::Error> SInter(
        std::span<const std::string_view> keys);

    std::expected<std::unordered_set<std::string>, error::Error> SDiff(
        std::span<const std::string_view> keys);

    std::expected<void, error::Error> SMove(std::string_view source,
                                            std::string_view destination,
                                            std::string_view member);

    std::expected<void, error::Error> Del(std::string_view key);

    std::expected<bool, error::Error> Exists(std::string_view key);

    std::expected<StorageType, error::Error> Type(std::string_view key);

    std::expected<void, error::Error> Expire(std::string_view key,
                                             std::chrono::seconds seconds);

    std::expected<std::optional<std::size_t>, error::Error> GetTTL(
        std::string_view key);

    std::expected<std::vector<std::string>, error::Error> Keys(
        std::string_view pattern);

    std::size_t EntryCount();
    std::expected<size_t, error::Error> MemoryUsageOfKey(std::string_view key);

    void FlushDb();
};

}  // namespace keyval
