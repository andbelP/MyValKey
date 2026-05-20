#pragma once

#include <string>
#include <expected>
#include <cstdint>
#include "ValKeyInterpreter.hpp"


std::string ToUpper(std::string value);

std::expected<std::int64_t, std::string> ParseInt(const std::string& text);

std::expected<std::size_t, keyval::ValKeyError> ParseMemorySize(std::string_view text);
