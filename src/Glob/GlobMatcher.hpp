#pragma once

#include <cstddef>
#include <string_view>

namespace keyval {

class GlobMatcher {
   public:
    static bool Match(std::string_view pattern, std::string_view text);

   private:
    static bool MatchImpl(std::string_view pattern, std::string_view text,
                          std::size_t pattern_pos, std::size_t text_pos);

    static bool ProcessStar(std::string_view pattern, std::string_view text,
                            std::size_t pattern_pos, std::size_t text_pos);

    static bool ProcessChar(std::string_view pattern, std::size_t& pattern_pos,
                            char text_char);
};

}  // namespace keyval
