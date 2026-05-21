#include "Utils.hpp"

#include <cstdint>
#include <expected>
#include <string>

#include "ValKeyInterpreter.hpp"

std::string ToUpper(std::string value) {
    for (auto& ch : value) {
        ch = std::toupper(ch);
    }
    return value;
}

std::expected<std::int64_t, std::string> ParseInt(const std::string& text) {
    try {
        std::size_t pos = 0;
        auto value = std::stoll(text, &pos);

        if (pos != text.size()) {
            return std::unexpected("invalid number");
        }

        return value;
    } catch (const std::exception&) {
        return std::unexpected("invalid number");
    }
}

std::expected<std::size_t, keyval::ValKeyError> ParseMemorySize(
    std::string_view text) {
    if (text.empty()) {
        return std::unexpected(keyval::ValKeyError{"invalid memory size"});
    }

    if (text.size() == 1) {
        auto res = ParseInt(std::string("") + text[0]);
        if (!res.has_value()) {
            return std::unexpected(keyval::ValKeyError{res.error()});
        }
        return res.value();
    }

    if (text.size() == 2) {
        if (text[1] == 'b') {
            auto res = ParseInt(std::string("") + text[0]);
            if (!res.has_value()) {
                return std::unexpected(keyval::ValKeyError{res.error()});
            }
            return res.value();
        } else {
            auto res = ParseInt(std::string(text));
            if (!res.has_value()) {
                return std::unexpected(keyval::ValKeyError{res.error()});
            }
            if (res.value() < 0) {
                return std::unexpected(
                    keyval::ValKeyError{"memory size must be positive"});
            }
            return res.value();
        }
    }

    if (text.back() == 'b') {
        if (text.at(text.size() - 2) == 'k') {
            auto res = ParseInt(std::string(text.substr(0, text.size() - 2)));
            if (!res.has_value()) {
                return std::unexpected(keyval::ValKeyError{res.error()});
            }
            if (res.value() < 0) {
                return std::unexpected(
                    keyval::ValKeyError{"memory size must be positive"});
            }
            return res.value() * 1024;
        } else if (text.at(text.size() - 2) == 'm') {
            auto res = ParseInt(std::string(text.substr(0, text.size() - 2)));
            if (!res.has_value()) {
                return std::unexpected(keyval::ValKeyError{res.error()});
            }
            if (res.value() < 0) {
                return std::unexpected(
                    keyval::ValKeyError{"memory size must be positive"});
            }
            return res.value() * 1024 * 1024;
        } else if (text.at(text.size() - 2) == 'g') {
            auto res = ParseInt(std::string(text.substr(0, text.size() - 2)));
            if (!res.has_value()) {
                return std::unexpected(keyval::ValKeyError{res.error()});
            }
            if (res.value() < 0) {
                return std::unexpected(
                    keyval::ValKeyError{"memory size must be positive"});
            }
            return res.value() * 1024 * 1024 * 1024;
        } else {
            auto res = ParseInt(std::string(text.substr(0, text.size() - 1)));
            if (!res.has_value()) {
                return std::unexpected(keyval::ValKeyError{res.error()});
            }
            if (res.value() < 0) {
                return std::unexpected(
                    keyval::ValKeyError{"memory size must be positive"});
            }
            return res.value();
        }
    }
    auto res = ParseInt(std::string(text));
    if (!res.has_value()) {
        return std::unexpected(keyval::ValKeyError{res.error()});
    }
    if (res.value() < 0) {
        return std::unexpected(
            keyval::ValKeyError{"memory size must be positive"});
    }
    return res.value();
}

std::expected<double, std::string> ParseDouble(const std::string& text) {
    try {
        std::size_t pos = 0;
        auto value = std::stod(text, &pos);

        if (pos != text.size()) {
            return std::unexpected("invalid number");
        }

        return value;
    } catch (const std::exception&) {
        return std::unexpected("invalid number");
    }
}