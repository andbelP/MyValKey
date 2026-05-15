#include "Engine/DBEngine.hpp"

#include <algorithm>
#include <ranges>

#include "Glob/GlobMatcher.hpp"

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
        it->second.value = std::string(value);
        it->second.expire_time = std::nullopt;
    } else {
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
        std::get<std::list<std::string>>(it->second.value)
            .push_front(std::string(value));
    } else {
        std::list<std::string> tmp;
        tmp.push_front(std::string(value));
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
        std::get<std::list<std::string>>(it->second.value)
            .push_back(std::string(value));
    } else {
        std::list<std::string> tmp;
        tmp.push_back(std::string(value));
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

    if (index < 0 || index >= size) {
        return std::unexpected(
            error::Error{error::ErrorCode::kInvalidCommand, "Invalid index"});
    }

    auto list_it = list.begin();
    std::advance(list_it, index);
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

    if (index < 0 || index >= size) {
        return std::unexpected(
            error::Error{error::ErrorCode::kInvalidCommand, "Invalid index"});
    }

    auto list_it = list.begin();
    std::advance(list_it, index);
    *list_it = std::string(value);

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

    if (index < 0 || index > size) {
        return std::unexpected(
            error::Error{error::ErrorCode::kInvalidCommand, "Invalid index"});
    }

    auto list_it = list.begin();
    std::advance(list_it, index);
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

    if (start < 0 || stop < 0 || start >= size || stop >= size ||
        start > stop) {
        return std::unexpected(
            error::Error{error::ErrorCode::kInvalidCommand, "Invalid index"});
    }

    auto view =
        list | std::views::drop(start) | std::views::take(stop + 1 - start);
    return std::vector<std::string>(std::from_range, view);
}

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
        std::get<std::unordered_set<std::string>>(it->second.value)
            .insert(std::string(member));
    } else {
        std::unordered_set<std::string> s;
        s.insert(std::string(member));
        storage_[std::string(key)] =
            StorageEntry(std::move(s), StorageType::kSet);
    }
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

std::expected<StorageType, error::Error> DBEngine::Type(
    std::string_view key) {
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

size_t DBEngine::EntryCount() {
    DeleteIfExpiredAll();
    return storage_.size();
}

namespace {
size_t GetSizeOfString(const std::string& str) {
    return sizeof(str) + str.capacity();
}
size_t GetSizeOfList(const std::list<std::string>& lst) {
    size_t size = sizeof(lst);
    for (const auto& str : lst) {
        size += GetSizeOfString(str);
    }
    return size;
}
size_t GetSizeOfSet(const std::unordered_set<std::string>& s) {
    size_t size = sizeof(s);
    for (const auto& str : s) {
        size += GetSizeOfString(str);
    }
    return size;
}
}  // namespace

std::expected<size_t, error::Error> DBEngine::MemoryUsageOfKey(
    std::string_view key) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));
    if (it == storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }
    auto& entry = it->second;
    switch (entry.type) {
        case StorageType::kString:
            return GetSizeOfString(std::get<std::string>(entry.value));
        case StorageType::kList:
            return GetSizeOfList(std::get<std::list<std::string>>(entry.value));
        case StorageType::kSet:
            return GetSizeOfSet(
                std::get<std::unordered_set<std::string>>(entry.value));
    }
    return std::unexpected(
        error::Error{error::ErrorCode::kEngineError, "Unknown storage type"});
}

bool DBEngine::DeleteIfExpired(
    std::string_view key) {
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
        if(it->second.expire_time.has_value() && it->second.expire_time.value() <= std::chrono::steady_clock::now()){
            it=storage_.erase(it);
        }
        else{
            ++it;
        }
    }
}

}  // namespace keyval
