#include <cstddef>
#include <cstdint>
#include <expected>
#include <iterator>
#include <list>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Engine/DBEngine.hpp"
#include "Engine/StorageTypes.hpp"
#include "Engine/TypesSizes.hpp"
#include "ErrorsHandling/Error.hpp"

namespace keyval {

namespace {

std::int64_t NormalizeIndex(std::int64_t index, std::size_t size) {
    if (index < 0) {
        index += size;
    }
    return index;
}

}  // namespace

std::expected<void, error::Error> DBEngine::LPush(std::string_view key,
                                                  std::string_view value) {
    DeleteIfExpired(key);
    auto it = storage_.find(std::string(key));
    if (it != storage_.end()) {
        if (it->second.type != StorageType::kList) {
            return std::unexpected(
                error::Error{error::ErrorCode::kWrongType,
                             "DB entry already exists with different type"});
        }

        if (max_memory_usage_.has_value() &&
            !CanAddBytes(GetSizeOfString(std::string(value)))) {
            return std::unexpected(
                error::Error{error::ErrorCode::kMaxMemoryExceeded,
                             "Cannot add entry: max memory usage exceeded"});
        }

        std::get<std::list<std::string>>(it->second.value)
            .push_front(std::string(value));
    } else {
        std::list<std::string> tmp;
        tmp.push_front(std::string(value));

        if (max_memory_usage_.has_value() && !CanAddBytes(GetSizeOfList(tmp))) {
            return std::unexpected(
                error::Error{error::ErrorCode::kMaxMemoryExceeded,
                             "Cannot add entry: max memory usage exceeded"});
        }
        storage_[std::string(key)] =
            StorageEntry(std::move(tmp), StorageType::kList);
    }
    return {};
}

std::expected<void, error::Error> DBEngine::RPush(std::string_view key,
                                                  std::string_view value) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));
    if (it != storage_.end()) {
        if (it->second.type != StorageType::kList) {
            return std::unexpected(
                error::Error{error::ErrorCode::kWrongType,
                             "DB entry already exists with different type"});
        }

        if (max_memory_usage_.has_value() &&
            !CanAddBytes(GetSizeOfString(std::string(value)))) {
            return std::unexpected(
                error::Error{error::ErrorCode::kMaxMemoryExceeded,
                             "Cannot add entry: max memory usage exceeded"});
        }

        std::get<std::list<std::string>>(it->second.value)
            .push_back(std::string(value));
    } else {
        std::list<std::string> tmp;
        tmp.push_back(std::string(value));

        if (max_memory_usage_.has_value() && !CanAddBytes(GetSizeOfList(tmp))) {
            return std::unexpected(
                error::Error{error::ErrorCode::kMaxMemoryExceeded,
                             "Cannot add entry: max memory usage exceeded"});
        }

        storage_[std::string(key)] =
            StorageEntry(std::move(tmp), StorageType::kList);
    }
    return {};
}

std::expected<std::string, error::Error> DBEngine::LPop(std::string_view key) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));
    if (it == storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }
    if (it->second.type != StorageType::kList) {
        return std::unexpected(
            error::Error{error::ErrorCode::kWrongType,
                         "DB entry already exists with different type"});
    }
    auto& list = std::get<std::list<std::string>>(it->second.value);
    if (list.empty()) {
        return std::unexpected(
            error::Error(error::ErrorCode::kInvalidCommand, "List is empty"));
    }
    std::string value = list.front();
    list.pop_front();
    if (list.empty()) {
        storage_.erase(it);
    }
    return value;
}

std::expected<std::string, error::Error> DBEngine::RPop(std::string_view key) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));
    if (it == storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }
    if (it->second.type != StorageType::kList) {
        return std::unexpected(
            error::Error{error::ErrorCode::kWrongType,
                         "DB entry already exists with different type"});
    }
    auto& list = std::get<std::list<std::string>>(it->second.value);
    if (list.empty()) {
        return std::unexpected(
            error::Error(error::ErrorCode::kInvalidCommand, "List is empty"));
    }
    std::string value = list.back();
    list.pop_back();
    if (list.empty()) {
        storage_.erase(it);
    }
    return value;
}

std::expected<std::size_t, error::Error> DBEngine::LLen(std::string_view key) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));
    if (it == storage_.end()) {
        return std::unexpected(
            error::Error(error::ErrorCode::kKeyNotFound, "Key not found"));
    }
    if (it->second.type != StorageType::kList) {
        return std::unexpected(
            error::Error{error::ErrorCode::kWrongType,
                         "DB entry already exists with different type"});
    }
    return std::get<std::list<std::string>>(it->second.value).size();
}

std::expected<std::string, error::Error> DBEngine::LIndex(
    std::string_view key, std::ptrdiff_t index) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));
    if (it == storage_.end()) {
        return std::unexpected(
            error::Error(error::ErrorCode::kKeyNotFound, "Key not found"));
    }
    if (it->second.type != StorageType::kList) {
        return std::unexpected(
            error::Error{error::ErrorCode::kWrongType,
                         "DB entry already exists with different type"});
    }
    const auto& list = std::get<std::list<std::string>>(it->second.value);
    auto size = list.size();

    auto normalized_index = NormalizeIndex(index, size);

    if (normalized_index >= size || normalized_index < 0) {
        return std::unexpected(
            error::Error{error::ErrorCode::kInvalidCommand, "Invalid index"});
    }

    auto list_it = list.begin();
    std::advance(list_it, normalized_index);
    return *list_it;
}

std::expected<void, error::Error> DBEngine::LSet(std::string_view key,
                                                 std::ptrdiff_t index,
                                                 std::string_view value) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));

    if (it == storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }

    if (it->second.type != StorageType::kList) {
        return std::unexpected(
            error::Error{error::ErrorCode::kWrongType,
                         "DB entry already exists with different type"});
    }

    auto& list = std::get<std::list<std::string>>(it->second.value);
    auto size = list.size();

    auto normalized_index = NormalizeIndex(index, size);

    if (normalized_index >= size || normalized_index < 0) {
        return std::unexpected(
            error::Error{error::ErrorCode::kInvalidCommand, "Invalid index"});
    }

    auto list_it = list.begin();
    std::advance(list_it, normalized_index);

    std::string new_value(value);
    const auto new_value_size = GetSizeOfString(new_value);
    const auto old_value_size = GetSizeOfString(*list_it);
    if (max_memory_usage_.has_value() && new_value_size > old_value_size &&
        !CanAddBytes(new_value_size - old_value_size)) {
        return std::unexpected(
            error::Error{error::ErrorCode::kMaxMemoryExceeded,
                         "Cannot set value: max memory usage exceeded"});
    }

    *list_it = std::move(new_value);

    return {};
}

std::expected<void, error::Error> DBEngine::LInsert(std::string_view key,
                                                    std::ptrdiff_t index,
                                                    std::string_view value) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));

    if (it == storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }

    if (it->second.type != StorageType::kList) {
        return std::unexpected(
            error::Error{error::ErrorCode::kWrongType,
                         "DB entry already exists with different type"});
    }

    auto& list = std::get<std::list<std::string>>(it->second.value);
    auto size = list.size();

    auto normalized_index = NormalizeIndex(index, size);

    if (normalized_index > size || normalized_index < 0) {
        return std::unexpected(
            error::Error{error::ErrorCode::kInvalidCommand, "Invalid index"});
    }

    auto list_it = list.begin();
    std::advance(list_it, normalized_index);

    if (max_memory_usage_.has_value() &&
        !CanAddBytes(GetSizeOfString(std::string(value)))) {
        return std::unexpected(
            error::Error{error::ErrorCode::kMaxMemoryExceeded,
                         "Cannot set value: max memory usage exceeded"});
    }

    list.insert(list_it, std::string(value));

    return {};
}

std::expected<std::vector<std::string>, error::Error> DBEngine::LRange(
    std::string_view key, std::ptrdiff_t start, std::ptrdiff_t stop) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));
    if (it == storage_.end()) {
        return std::unexpected(
            error::Error(error::ErrorCode::kKeyNotFound, "Key not found"));
    }
    if (it->second.type != StorageType::kList) {
        return std::unexpected(
            error::Error{error::ErrorCode::kWrongType,
                         "DB entry already exists with different type"});
    }
    const auto& list = std::get<std::list<std::string>>(it->second.value);
    auto size = list.size();
    if (size == 0 && (start != 0 || stop != 0)) {
        return std::unexpected(
            error::Error(error::ErrorCode::kInvalidCommand, "Invalid indexes"));
    }
    if (size == 0) {
        return {};
    }

    auto normalized_start = NormalizeIndex(start, size);
    auto normalized_stop = NormalizeIndex(stop, size);

    if (normalized_start >= size || normalized_stop >= size ||
        normalized_start > normalized_stop || normalized_start < 0 ||
        normalized_stop < 0) {
        return std::unexpected(
            error::Error{error::ErrorCode::kInvalidCommand, "Invalid index"});
    }

    auto view = list | std::views::drop(normalized_start) |
                std::views::take(normalized_stop + 1 - normalized_start);
    return std::vector<std::string>(std::from_range, view);
}

}  // namespace keyval
