#pragma once

#include <expected>
#include <string>
#include <cstddef>
#include <chrono>

#include "ErrorsHandling/Error.hpp"

namespace keyval{

    class KeyValEngine{

    public:

        std::expected<void, error::Error> Set(std::string_view key, std::string_view value);

        std::expected<std::string, error::Error> Get(std::string_view key) const;

        std::expected<std::size_t, error::Error> StrLen(std::string_view key) const;

        std::expected<void, error::Error> Append(std::string_view key, std::string_view value);

        std::expected<void, error::Error> Expire(std::string_view key, std::chrono::seconds seconds);
        
        std::expected<std::size_t, error::Error> GetTTL(std::string_view key) const;
        
    };

}
