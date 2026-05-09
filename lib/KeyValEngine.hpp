#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "ErrorsHandling/Error.hpp"

namespace keyval {

enum class ValueType { kString, kList, kSet, kGeo, kNone };

enum class LInsertMode { kBefore, kAfter };

class KeyValEngine {
   public:
    std::expected<void, error::Error> Set(std::string_view key,
                                          std::string_view value);

    std::expected<std::string, error::Error> Get(std::string_view key) const;

    std::expected<std::size_t, error::Error> StrLen(std::string_view key) const;

    std::expected<std::size_t, error::Error> Append(std::string_view key,
                                             std::string_view value);

    std::expected<bool, error::Error> Expire(std::string_view key,
                                             std::chrono::seconds seconds);

    std::expected<std::int64_t, error::Error> GetTTL(std::string_view key) const;



    std::expected<std::size_t, error::Error> LPush(
        std::string_view key, std::span<const std::string_view> values);

    std::expected<std::size_t, error::Error> RPush(
        std::string_view key, std::span<const std::string_view> values);

    std::expected<std::vector<std::string>, error::Error> LPop(
        std::string_view key, std::size_t cnt = 1);

    std::expected<std::vector<std::string>, error::Error> RPop(
        std::string_view key, std::size_t cnt = 1);

    std::expected<std::size_t, error::Error> LLen(std::string_view key) const;

    std::expected<std::vector<std::string>, error::Error> LRange(
        std::string_view key, std::ptrdiff_t start, std::ptrdiff_t stop) const;

    std::expected<std::string, error::Error> LIndex(std::string_view key,
                                                    std::ptrdiff_t index) const;

    std::expected<void, error::Error> LSet(std::string_view key,
                                           std::ptrdiff_t index,
                                           std::string_view value);

    std::expected<std::int64_t, error::Error> LInsert(std::string_view key,
                                              LInsertMode mode,
                                              std::string_view pivot,
                                              std::string_view value);



    std::expected<std::size_t, error::Error> SAdd(
        std::string_view key, std::span<const std::string_view> members);

    std::expected<std::size_t, error::Error> SRem(
        std::string_view key, std::span<const std::string_view> members);

    std::expected<bool, error::Error> SIsMember(std::string_view key,
                                                std::string_view member) const;

    std::expected<std::vector<std::string>, error::Error> SMembers(
        std::string_view key) const;

    std::expected<std::size_t, error::Error> SCard(std::string_view key) const;

    std::expected<std::vector<std::string>, error::Error> SUnion(
        std::span<const std::string_view> keys) const;

    std::expected<std::vector<std::string>, error::Error> SInter(
        std::span<const std::string_view> keys) const;

    std::expected<std::vector<std::string>, error::Error> SDiff(
        std::span<const std::string_view> keys) const;

    std::expected<bool, error::Error> SMove(std::string_view source,
                                            std::string_view destination,
                                            std::string_view member);



    std::expected<ValueType, error::Error> Type(std::string_view key) const;

    std::expected<std::size_t, error::Error> Del(
        std::span<const std::string_view> keys);

    std::expected<std::size_t, error::Error> Exists(
        std::span<const std::string_view> keys) const;

    std::expected<std::vector<std::string>, error::Error> Keys(
        std::string_view pattern) const;

    void FlushDb();

    std::expected<void, error::Error> ConfigSetMaxMemory(std::uint64_t bytes);

    std::expected<std::size_t, error::Error> ConfigGetMaxMemory() const;

    std::expected<std::size_t, error::Error> DbSize() const;

    std::expected<std::size_t, error::Error> MemoryUsage(
        std::string_view key) const;
};

}  // namespace keyval
