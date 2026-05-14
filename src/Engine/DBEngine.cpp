#include <algorithm>
#include <ranges>

#include "Engine/DBEngine.hpp"
#include "Glob/GlobMatcher.hpp"

namespace keyval {

std::expected<void, error::Error> DBEngine::Set(std::string_view key, std::string_view value) {
    auto it = storage_.find(std::string(key));
    
    if(it != storage_.end()){
        if(it->second.type != StorageType::kString){
            return std::unexpected(error::Error{error::ErrorCode::kWrongType, "DB entry already exists with different type"});
        }
        it->second.value = std::string(value);
    }
    else {
        storage_[std::string(key)] = StorageEntry(std::string(value), StorageType::kString);
    }

    return {};
}

std::expected<std::string, error::Error> DBEngine::Get(std::string_view key) {
    auto it = storage_.find(std::string(key));

    if (it == storage_.end()) {
        return std::unexpected(error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }

    if (it->second.type != StorageType::kString) {
        return std::unexpected(error::Error{error::ErrorCode::kWrongType, "DB entry already exists with different type"});
    }

    return std::get<std::string>(it->second.value);
}

std::expected<void, error::Error> DBEngine::LPush(std::string_view key, std::string_view value) {
    auto it = storage_.find(std::string(key));
    if(it!= storage_.end()){
        if(it->second.type != StorageType::kList){
            return std::unexpected(error::Error{error::ErrorCode::kWrongType, "DB entry already exists with different type"});
        }
        std::get<std::list<std::string>>(it->second.value).push_front(std::string(value));
    }
    else {
        std::list<std::string> tmp;
        tmp.push_front(std::string(value));
        storage_[std::string(key)] = StorageEntry(std::move(tmp), StorageType::kList);
    }
    return {};
}

std::expected<void, error::Error> DBEngine::RPush(std::string_view key, std::string_view value) {
    auto it = storage_.find(std::string(key));
    if(it!= storage_.end()){
        if(it->second.type != StorageType::kList){
            return std::unexpected(error::Error{error::ErrorCode::kWrongType, "DB entry already exists with different type"});
        }
        std::get<std::list<std::string>>(it->second.value).push_back(std::string(value));
    }
    else {
        std::list<std::string> tmp;
        tmp.push_back(std::string(value));
        storage_[std::string(key)] = StorageEntry(std::move(tmp), StorageType::kList);
    }
    return {};
}

std::expected<std::string, error::Error> DBEngine::LPop(std::string_view key) {
    auto it = storage_.find(std::string(key));
    if(it== storage_.end()){
        return std::unexpected(error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }
    if(it->second.type != StorageType::kList){
        return std::unexpected(error::Error{error::ErrorCode::kWrongType, "DB entry already exists with different type"});
    }
    auto& list = std::get<std::list<std::string>>(it->second.value);
    if(list.empty()){
        return std::unexpected(error::Error(error::ErrorCode::kInvalidCommand, "List is empty"));
    }
    std::string value = list.front();
    list.pop_front();
    if(list.empty()){
        storage_.erase(it);
    }
    return value;
}

std::expected<std::string, error::Error> DBEngine::RPop(std::string_view key) {
    auto it = storage_.find(std::string(key));
    if(it== storage_.end()){
        return std::unexpected(error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }
    if(it->second.type != StorageType::kList){
        return std::unexpected(error::Error{error::ErrorCode::kWrongType, "DB entry already exists with different type"});
    }
    auto& list = std::get<std::list<std::string>>(it->second.value);
    if(list.empty()){
        return std::unexpected(error::Error(error::ErrorCode::kInvalidCommand, "List is empty"));
    }
    std::string value = list.back();
    list.pop_back();
    if(list.empty()){
        storage_.erase(it);
    }
    return value;
}

std::expected<std::size_t, error::Error> DBEngine::LLen(std::string_view key) const {
    auto it = storage_.find(std::string(key));
    if (it == storage_.end()){
        return std::unexpected(error::Error(error::ErrorCode::kKeyNotFound, "Key not found"));
    }
    if(it->second.type != StorageType::kList){
        return std::unexpected(error::Error{error::ErrorCode::kWrongType, "DB entry already exists with different type"});
    }
    return std::get<std::list<std::string>>(it->second.value).size();
}

std::expected<std::string, error::Error> DBEngine::LIndex(std::string_view key, std::ptrdiff_t index) const {
    auto it = storage_.find(std::string(key));
    if (it == storage_.end()){
        return std::unexpected(error::Error(error::ErrorCode::kKeyNotFound, "Key not found"));
    }
    if(it->second.type != StorageType::kList){
        return std::unexpected(error::Error{error::ErrorCode::kWrongType, "DB entry already exists with different type"});
    }
    const auto& list = std::get<std::list<std::string>>(it->second.value);
    auto size = list.size();
    if(size == 0 && index!=0){
        return std::unexpected(error::Error(error::ErrorCode::kInvalidCommand, "Invalid indexes"));
    }
    if(size == 0){
        return {};
    }
    while(index < 0) index += size;
    if(index >= size){
        return std::unexpected(error::Error(error::ErrorCode::kInvalidCommand, "Invalid index"));
    }
    auto view = list | std::views::drop(index) | std::views::take(1);
    return *view.begin();
}

std::expected<std::vector<std::string>, error::Error> DBEngine::LRange(std::string_view key, std::ptrdiff_t start, std::ptrdiff_t stop) const {
    auto it = storage_.find(std::string(key));
    if(it==storage_.end()){
        return std::unexpected(error::Error(error::ErrorCode::kKeyNotFound, "Key not found"));
    }
    if(it->second.type != StorageType::kList){
        return std::unexpected(error::Error{error::ErrorCode::kWrongType, "DB entry already exists with different type"});
    }
    const auto& list = std::get<std::list<std::string>>(it->second.value);
    auto size = list.size();
    if(size == 0 && (start!=0 || stop!=0)){
        return std::unexpected(error::Error(error::ErrorCode::kInvalidCommand, "Invalid indexes"));
    }
    if(size == 0){
        return {};
    }
    while(start < 0) start += size;
    while(stop < 0) stop += size;

    if(start > stop || start >= size || stop >= size){
        return std::unexpected(error::Error(error::ErrorCode::kInvalidCommand, "Invalid indexes"));
    }

    auto view = list | std::views::drop(start) | std::views::take(stop+1-start);
    return std::vector<std::string>(std::from_range, view);
    
}

std::expected<void, error::Error> DBEngine::SAdd(std::string_view key, std::string_view member) {
    auto it = storage_.find(std::string(key));
    if (it != storage_.end()){
        if(it->second.type != StorageType::kSet){
            return std::unexpected(error::Error{error::ErrorCode::kWrongType, "DB entry already exists with different type"});
        }
        std::get<std::unordered_set<std::string>>(it->second.value).insert(std::string(member));
    } else {
        std::unordered_set<std::string> s;
        s.insert(std::string(member));
        storage_[std::string(key)] = StorageEntry(std::move(s), StorageType::kSet);
    }
    return {};
}

std::expected<void, error::Error> DBEngine::SRem(std::string_view key, std::string_view member) {
    auto it = storage_.find(std::string(key));
    if (it == storage_.end()){
        return std::unexpected(error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }

    if(it->second.type != StorageType::kSet){
        return std::unexpected(error::Error{error::ErrorCode::kWrongType, "DB entry already exists with different type"});
    }

    auto& set = std::get<std::unordered_set<std::string>>(it->second.value);
    set.erase(std::string(member));
    return {};
}

std::expected<bool, error::Error> DBEngine::SIsMember(std::string_view key, std::string_view member) const {
    auto it = storage_.find(std::string(key));
    if (it == storage_.end()){
        return std::unexpected(error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }
    if(it->second.type != StorageType::kSet){
        return std::unexpected(error::Error{error::ErrorCode::kWrongType, "DB entry already exists with different type"});
    }
    const auto& set = std::get<std::unordered_set<std::string>>(it->second.value);
    return set.contains(std::string(member));
}

std::expected<std::unordered_set<std::string>, error::Error> DBEngine::SMembers(std::string_view key) const {
    auto it = storage_.find(std::string(key));
    if (it == storage_.end()){
        return std::unexpected(error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }
    if(it->second.type != StorageType::kSet){
        return std::unexpected(error::Error{error::ErrorCode::kWrongType, "DB entry already exists with different type"});
    }

    return std::get<std::unordered_set<std::string>>(it->second.value);
}

std::expected<std::size_t, error::Error> DBEngine::SCard(std::string_view key) const {
    auto it = storage_.find(std::string(key));
    if (it == storage_.end()){
        return std::unexpected(error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }
    if(it->second.type != StorageType::kSet){
        return std::unexpected(error::Error{error::ErrorCode::kWrongType, "DB entry already exists with different type"});
    }
    return std::get<std::unordered_set<std::string>>(it->second.value).size();
}

std::expected<void, error::Error> DBEngine::Del(std::string_view key) {
    auto it = storage_.find(std::string(key));
    if (it == storage_.end()){
        return std::unexpected(error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }
    storage_.erase(it);
    return {};
}

std::expected<bool, error::Error> DBEngine::Exists(std::string_view key) const {
    return storage_.contains(std::string(key));
}

std::expected<StorageType, error::Error> DBEngine::Type(std::string_view key) const {
    auto it = storage_.find(std::string(key));
    if (it == storage_.end()){
        return std::unexpected(error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }
    return it->second.type;
}

std::expected<void, error::Error> DBEngine::Expire(std::string_view key, std::chrono::seconds seconds) {
    auto it = storage_.find(std::string(key));
    if (it == storage_.end()){
        return std::unexpected(error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }
    it->second.expire_time = std::chrono::steady_clock::now() + seconds;
    return {};
}

std::expected<std::optional<std::size_t>, error::Error> DBEngine::GetTTL(std::string_view key) const {
    auto it = storage_.find(std::string(key));
    if (it == storage_.end() || (it->second.expire_time.has_value() && it->second.expire_time.value() <= std::chrono::steady_clock::now())){
        return std::unexpected(error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }
    if(!it->second.expire_time.has_value()){
        return std::nullopt;

    }
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(it->second.expire_time.value() - now).count();
}

std::expected<std::vector<std::string>, error::Error> DBEngine::Keys(std::string_view pattern) const {
    std::vector<std::string> result;
    for (const auto& entry : storage_) {
        if (GlobMatcher::Match(pattern, entry.first)) {
            result.push_back(entry.first);
        }
    }
    return result;
}

void DBEngine::FlushDb() {
    storage_.clear();
}

} // namespace keyval
