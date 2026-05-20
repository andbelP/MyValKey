#include "Engine/DBEngine.hpp"
#include "Engine/TypesSizes.hpp"
#include <algorithm>
#include <ranges>


namespace keyval {

std::expected<void, error::Error> DBEngine::SAdd(std::string_view key,
                                                 std::string_view member) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));
    if (it != storage_.end()) {
        if (it->second.type != StorageType::kSet) {
            return std::unexpected(
                error::Error{error::ErrorCode::kWrongType,
                             "DB entry already exists with different type"});
        }

        if(max_memory_usage_.has_value() && !std::get<std::unordered_set<std::string>>(it->second.value).contains(std::string(member)) && !CanAddBytes(GetSizeOfString(std::string(member)))) {
            return std::unexpected(error::Error{
                error::ErrorCode::kMaxMemoryExceeded,
                "Cannot add member: max memory usage exceeded"});
        }

        std::get<std::unordered_set<std::string>>(it->second.value)
            .insert(std::string(member));
    } else {
        std::unordered_set<std::string> s;
        s.insert(std::string(member));

        if(max_memory_usage_.has_value() && !CanAddBytes(GetSizeOfSet(s))) {
            return std::unexpected(error::Error{
                error::ErrorCode::kMaxMemoryExceeded,
                "Cannot add member: max memory usage exceeded"});
        }

        storage_[std::string(key)] =
            StorageEntry(std::move(s), StorageType::kSet);
    }
    return {};
}

std::expected<void, error::Error> DBEngine::SCreate(std::string_view key) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));
    if (it != storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyAlreadyExists,
                         "DB entry already exists, cant create set"});
    }

    if(max_memory_usage_.has_value() && !CanAddBytes(GetSizeOfSet(std::unordered_set<std::string>{}))) {
        return std::unexpected(error::Error{
            error::ErrorCode::kMaxMemoryExceeded,
            "Cannot create set: max memory usage exceeded"});
    }

    storage_[std::string(key)] = StorageEntry(std::unordered_set<std::string>{}, StorageType::kSet);
    return {};
}

std::expected<void, error::Error> DBEngine::SRem(std::string_view key,
                                                 std::string_view member) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));
    if (it == storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }

    if (it->second.type != StorageType::kSet) {
        return std::unexpected(
            error::Error{error::ErrorCode::kWrongType,
                         "DB entry already exists with different type"});
    }

    auto& set = std::get<std::unordered_set<std::string>>(it->second.value);
    set.erase(std::string(member));
    return {};
}

std::expected<std::unordered_set<std::string>, error::Error> DBEngine::SUnion(
    std::span<const std::string> keys) {
    std::unordered_set<std::string> result;

    for (auto key : keys) {
        DeleteIfExpired(key);

        auto it = storage_.find(std::string(key));

        if (it == storage_.end()) {
            return std::unexpected(
                error::Error{error::ErrorCode::kKeyNotFound,
                             std::string(key) + ": Key not found"});
        }

        if (it->second.type != StorageType::kSet) {
            return std::unexpected(error::Error{
                error::ErrorCode::kWrongType,
                std::string(key) +
                    ": DB entry already exists with different type"});
        }

        const auto& set =
            std::get<std::unordered_set<std::string>>(it->second.value);

        result.insert(set.begin(), set.end());
    }

    return result;
}

std::expected<std::unordered_set<std::string>, error::Error> DBEngine::SInter(
    std::span<const std::string> keys) {
    std::unordered_set<std::string> result;

    if (keys.empty()) {
        return result;
    }

    DeleteIfExpired(keys[0]);

    auto first_it = storage_.find(std::string(keys[0]));

    if (first_it == storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound,
                         std::string(keys[0]) + ": Key not found"});
    }

    if (first_it->second.type != StorageType::kSet) {
        return std::unexpected(
            error::Error{error::ErrorCode::kWrongType,
                         std::string(keys[0]) +
                             ": DB entry already exists with different type"});
    }

    result = std::get<std::unordered_set<std::string>>(first_it->second.value);

    for (std::size_t i = 1; i < keys.size(); ++i) {
        DeleteIfExpired(keys[i]);
        auto it = storage_.find(std::string(keys[i]));

        if (it == storage_.end()) {
            return std::unexpected(
                error::Error{error::ErrorCode::kKeyNotFound,
                             std::string(keys[i]) + ": Key not found"});
        }

        if (it->second.type != StorageType::kSet) {
            return std::unexpected(error::Error{
                error::ErrorCode::kWrongType,
                std::string(keys[i]) +
                    ": DB entry already exists with different type"});
        }

        const auto& current =
            std::get<std::unordered_set<std::string>>(it->second.value);

        for (auto result_it = result.begin(); result_it != result.end();) {
            if (!current.contains(*result_it)) {
                result_it = result.erase(result_it);
            } else {
                ++result_it;
            }
        }
    }

    return result;
}

std::expected<std::unordered_set<std::string>, error::Error> DBEngine::SDiff(
    std::span<const std::string> keys) {
    std::unordered_set<std::string> result;

    if (keys.empty()) {
        return result;
    }

    DeleteIfExpired(keys[0]);
    auto first_it = storage_.find(std::string(keys[0]));

    if (first_it == storage_.end()) {
        return result;
    }

    if (first_it->second.type != StorageType::kSet) {
        return std::unexpected(
            error::Error{error::ErrorCode::kWrongType,
                         "DB entry already exists with different type"});
    }

    result = std::get<std::unordered_set<std::string>>(first_it->second.value);

    for (std::size_t i = 1; i < keys.size(); ++i) {
        DeleteIfExpired(keys[i]);
        auto it = storage_.find(std::string(keys[i]));

        if (it == storage_.end()) {
            continue;
        }

        if (it->second.type != StorageType::kSet) {
            return std::unexpected(
                error::Error{error::ErrorCode::kWrongType,
                             "DB entry already exists with different type"});
        }

        const auto& current =
            std::get<std::unordered_set<std::string>>(it->second.value);

        for (const auto& member : current) {
            result.erase(member);
        }
    }

    return result;
}

std::expected<void, error::Error> DBEngine::SMove(std::string_view source,
                                                  std::string_view destination,
                                                  std::string_view member) {
    DeleteIfExpired(source);
    DeleteIfExpired(destination);

    auto source_it = storage_.find(std::string(source));

    if (source_it == storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound,
                         std::string(source) + ": Key not found"});
    }

    if (source_it->second.type != StorageType::kSet) {
        return std::unexpected(
            error::Error{error::ErrorCode::kWrongType,
                         "DB entry already exists with different type"});
    }

    auto& source_set =
        std::get<std::unordered_set<std::string>>(source_it->second.value);

    auto dest_it = storage_.find(std::string(destination));
    if (dest_it == storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound,
                         std::string(destination) + ": Key not found"});
    }

    if (dest_it->second.type != StorageType::kSet) {
        return std::unexpected(
            error::Error{error::ErrorCode::kWrongType,
                         "DB entry already exists with different type"});
    }

    auto& dest_set =
        std::get<std::unordered_set<std::string>>(dest_it->second.value);

    if (!source_set.contains(std::string(member))) {
        return std::unexpected(
            error::Error{error::ErrorCode::kInvalidCommand,
                         "Source doesnt have member element"});
    }

    source_set.erase(std::string(member));
    dest_set.insert(std::string(member));
    return {};
}

std::expected<bool, error::Error> DBEngine::SIsMember(std::string_view key,
                                                      std::string_view member) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));
    if (it == storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }
    if (it->second.type != StorageType::kSet) {
        return std::unexpected(
            error::Error{error::ErrorCode::kWrongType,
                         "DB entry already exists with different type"});
    }
    const auto& set =
        std::get<std::unordered_set<std::string>>(it->second.value);
    return set.contains(std::string(member));
}

std::expected<std::unordered_set<std::string>, error::Error> DBEngine::SMembers(
    std::string_view key) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));
    if (it == storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }
    if (it->second.type != StorageType::kSet) {
        return std::unexpected(
            error::Error{error::ErrorCode::kWrongType,
                         "DB entry already exists with different type"});
    }

    return std::get<std::unordered_set<std::string>>(it->second.value);
}

std::expected<std::size_t, error::Error> DBEngine::SCard(std::string_view key) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));
    if (it == storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }
    if (it->second.type != StorageType::kSet) {
        return std::unexpected(
            error::Error{error::ErrorCode::kWrongType,
                         "DB entry already exists with different type"});
    }
    return std::get<std::unordered_set<std::string>>(it->second.value).size();
}

}