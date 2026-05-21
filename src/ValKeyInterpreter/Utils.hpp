#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

#include "CommandResult/CommandResult.hpp"

std::string ToUpper(std::string value);

std::expected<std::int64_t, std::string> ParseInt(const std::string& text);

std::expected<double, std::string> ParseDouble(const std::string& text);

std::expected<std::size_t, keyval::ValKeyError> ParseMemorySize(
    std::string_view text);
