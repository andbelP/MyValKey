#include <cstddef>
#include <expected>
#include <optional>
#include <string>
#include <string_view>

#include "Engine/DBEngine.hpp"
#include "Engine/StorageTypes.hpp"
#include "ErrorsHandling/Error.hpp"
#include "TypesSizes.hpp"

namespace keyval {

std::expected<void, error::Error> DBEngine::Set(std::string_view key,
                                                std::string_view value) {
    DeleteIfExpired(key);
    auto it = storage_.find(std::string(key));

    if (it != storage_.end()) {
        if (it->second.type != StorageType::kString) {
            return std::unexpected(
                error::Error{error::ErrorCode::kWrongType,
                             "DB entry already exists with different type"});
        }

        std::string old_value = std::get<std::string>(it->second.value);
        if (max_memory_usage_.has_value() &&
            GetSizeOfString(old_value) < GetSizeOfString(std::string(value)) &&
            !CanAddBytes(GetSizeOfString(std::string(value)) -
                         GetSizeOfString(old_value))) {
            return std::unexpected(
                error::Error{error::ErrorCode::kMaxMemoryExceeded,
                             "Cannot set value: max memory usage exceeded"});
        }

        it->second.value = std::string(value);
        it->second.expire_time = std::nullopt;

    } else {
        if (max_memory_usage_.has_value() &&
            !CanAddBytes(GetSizeOfString(std::string(value)))) {
            return std::unexpected(
                error::Error{error::ErrorCode::kMaxMemoryExceeded,
                             "Cannot set value: max memory usage exceeded"});
        }
        storage_[std::string(key)] =
            StorageEntry(std::string(value), StorageType::kString);
    }

    return {};
}

std::expected<std::string, error::Error> DBEngine::Get(std::string_view key) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));

    if (it == storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }

    if (it->second.type != StorageType::kString) {
        return std::unexpected(
            error::Error{error::ErrorCode::kWrongType,
                         "DB entry already exists with different type"});
    }

    return std::get<std::string>(it->second.value);
}

std::expected<void, error::Error> DBEngine::Append(std::string_view key,
                                                   std::string_view value) {
    DeleteIfExpired(key);
    auto it = storage_.find(std::string(key));

    if (it != storage_.end()) {
        if (it->second.type != StorageType::kString) {
            return std::unexpected(
                error::Error{error::ErrorCode::kWrongType,
                             "DB entry already exists with different type"});
        }

        std::string old_value = std::get<std::string>(it->second.value);
        if (max_memory_usage_.has_value() &&
            GetSizeOfString(old_value) <
                GetSizeOfString(old_value + std::string(value)) &&
            !CanAddBytes(GetSizeOfString(old_value + std::string(value)) -
                         GetSizeOfString(old_value))) {
            return std::unexpected(
                error::Error{error::ErrorCode::kMaxMemoryExceeded,
                             "Cannot append value: max memory usage exceeded"});
        }

        it->second.value = old_value + std::string(value);

    } else {
        if (max_memory_usage_.has_value() &&
            !CanAddBytes(GetSizeOfString(std::string(value)))) {
            return std::unexpected(
                error::Error{error::ErrorCode::kMaxMemoryExceeded,
                             "Cannot append value: max memory usage exceeded"});
        }
        storage_[std::string(key)] =
            StorageEntry(std::string(value), StorageType::kString);
    }

    return {};
}

std::expected<std::size_t, error::Error> DBEngine::StrLen(
    std::string_view key) {
    DeleteIfExpired(key);
    auto it = storage_.find(std::string(key));

    if (it == storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }

    if (it->second.type != StorageType::kString) {
        return std::unexpected(
            error::Error{error::ErrorCode::kWrongType,
                         "DB entry already exists with different type"});
    }

    return std::get<std::string>(it->second.value).size();
}

}  // namespace keyval
