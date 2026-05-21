#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <list>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include "Engine/StorageTypes.hpp"
#include "ErrorsHandling/Error.hpp"

namespace keyval {

struct GeoPoint {
    double latitude;
    double longitude;
};

 struct GeoPointWithDetails {
        std::string member;
        GeoPoint point;
        double distance;
    };

enum class GeoUnit { kMeter, kKilometer, kMile, kFoot };

using GeoEntry = std::unordered_map<std::string, GeoPoint>;

using ValueType = std::variant<std::string, std::list<std::string>,
                               std::unordered_set<std::string>, GeoEntry>;

struct StorageEntry {
    ValueType value;
    StorageType type;
    std::optional<std::chrono::steady_clock::time_point> expire_time;

    StorageEntry() = default;

    template<typename ValueTypeT>
    StorageEntry(ValueTypeT&& val, StorageType t)
        : value(std::forward<ValueTypeT>(val)),
          type(t) {}
};

class DBEngine {
    std::unordered_map<std::string, StorageEntry> storage_;

    std::optional<std::size_t> max_memory_usage_;

    bool DeleteIfExpired(std::string_view it);
    void DeleteIfExpiredAll();

    std::expected<std::size_t, error::Error> MemoryUsageOfEntry(
        const StorageEntry& entry);

    bool CanAddBytes(std::size_t cnt);

   public:
    std::expected<void, error::Error> Set(std::string_view key,
                                          std::string_view value);

    std::expected<std::string, error::Error> Get(std::string_view key);

    std::expected<void, error::Error> Append(std::string_view key,
                                             std::string_view value);

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

    std::expected<void, error::Error> SCreate(std::string_view key);

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
        std::span<const std::string> keys);

    std::expected<std::unordered_set<std::string>, error::Error> SInter(
        std::span<const std::string> keys);

    std::expected<std::unordered_set<std::string>, error::Error> SDiff(
        std::span<const std::string> keys);

    std::expected<void, error::Error> SMove(std::string_view source,
                                            std::string_view destination,
                                            std::string_view member);

    std::expected<void, error::Error> GeoAdd(std::string_view key,
                                             std::string_view member,
                                             GeoPoint point);

    std::expected<GeoPoint, error::Error> GeoPos(std::string_view key,
                                                 std::string_view member);

    std::expected<double, error::Error> GeoDist(std::string_view key,
                                                std::string_view member1,
                                                std::string_view member2,
                                                GeoUnit unit);

    std::expected<std::vector<GeoPointWithDetails>, error::Error> GeoSearch(
        std::string_view key, GeoPoint center, double radius, GeoUnit unit,
        std::size_t count, bool ascending = true);

    std::expected<std::vector<GeoPointWithDetails>, error::Error> GeoSearchStore(
        std::string_view dest, std::string_view source, GeoPoint center,
        double radius, GeoUnit unit, std::size_t count, bool ascending = true);

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
    std::expected<std::size_t, error::Error> MemoryUsageOfKey(
        std::string_view key);

    void FlushDb();

    std::expected<void, error::Error> SetMaxMemoryUsage(
        std::size_t max_memory_usage);

    std::optional<std::size_t> GetMaxMemoryUsage() const;

    std::size_t GetCurrentMemoryUsage();
};

}  // namespace keyval
