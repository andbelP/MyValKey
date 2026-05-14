#include "Glob/GlobMatcher.hpp"

namespace keyval {

bool GlobMatcher::Match(std::string_view pattern, std::string_view text) {
    return MatchImpl(pattern, text, 0, 0);
}

bool GlobMatcher::MatchImpl(std::string_view pattern,
                            std::string_view text,
                            std::size_t pattern_pos,
                            std::size_t text_pos) {
    while (pattern_pos < pattern.size()) {
        char pattern_char = pattern[pattern_pos];

        if (pattern_char == '*') {
            return ProcessStar(pattern, text, pattern_pos, text_pos);
        }

        if (text_pos == text.size()) {
            return false;
        }

        if (pattern_char == '?') {
            ++pattern_pos;
            ++text_pos;
            continue;
        }

        if (pattern_char == '[') {
            if (!ProcessChar(pattern, pattern_pos, text[text_pos])) {
                return false;
            }

            ++text_pos;
            continue;
        }

        if (pattern_char == '\\') {
            ++pattern_pos;

            if (pattern_pos == pattern.size()) {
                return false;
            }

            pattern_char = pattern[pattern_pos];
        }

        if (pattern_char != text[text_pos]) {
            return false;
        }

        ++pattern_pos;
        ++text_pos;
    }

    return text_pos == text.size();
}

bool GlobMatcher::ProcessStar(std::string_view pattern,
                              std::string_view text,
                              std::size_t pattern_pos,
                              std::size_t text_pos) {
    while (pattern_pos < pattern.size() && pattern[pattern_pos] == '*') {
        ++pattern_pos;
    }

    if (pattern_pos == pattern.size()) {
        return true;
    }

    for (std::size_t i = text_pos; i <= text.size(); ++i) {
        if (MatchImpl(pattern, text, pattern_pos, i)) {
            return true;
        }
    }

    return false;
}

bool GlobMatcher::ProcessChar(std::string_view pattern,
                              std::size_t& pattern_pos,
                              char text_char) {
    ++pattern_pos;

    if (pattern_pos >= pattern.size()) {
        return false;
    }

    bool is_negated = false;
    if (pattern[pattern_pos] == '^') {
        is_negated = true;
        ++pattern_pos;
    }

    bool is_matched = false;

    while (pattern_pos < pattern.size()) {
        if (pattern[pattern_pos] == ']') {
            ++pattern_pos;
            return is_negated ? !is_matched : is_matched;
        }

        if (pattern_pos + 2 < pattern.size() &&
            pattern[pattern_pos + 1] == '-' &&
            pattern[pattern_pos + 2] != ']') {
            char from = pattern[pattern_pos];
            char to = pattern[pattern_pos + 2];

            if (from <= text_char && text_char <= to) {
                is_matched = true;
            }

            pattern_pos += 3;
            continue;
        }

        char pattern_char = pattern[pattern_pos];

        if (pattern_char == '\\' && pattern_pos + 1 < pattern.size()) {
            ++pattern_pos;
            pattern_char = pattern[pattern_pos];
        }

        if (pattern_char == text_char) {
            is_matched = true;
        }

        ++pattern_pos;
    }

    return false;
}

} // namespace keyval
