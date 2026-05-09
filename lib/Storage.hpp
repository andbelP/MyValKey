#pragma once

#include <chrono>
#include <cstdint>
#include <expected>
#include <unordered_map>
#include <variant>

#include "ErrorsHandling/Error.hpp"
#include "StorageTypes/StorageTypes.hpp"

namespace keyval {

using ValueType = std::variant<std::string>;

struct StorageEntry {
    ValueType value;
    std::optional<std::chrono::steady_clock::time_point> expire_time;
};

class Storage {
    std::unordered_map<std::string, StorageEntry> storage_;

   public:
   
    std::expected<std::string&, error::Error> GetString(std::string_view key);

    std::expected<const std::string&, error::Error> GetString(
        std::string_view key) const;

    std::expected<void, error::Error> CreateString(std::string_view key, std::string_view value);


        
};


}  // namespace keyval