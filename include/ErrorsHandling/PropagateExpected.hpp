#pragma once

#define PROPAGATE_EXPECTED(EXPECTED)                  \
    do {                                              \
        if (!EXPECTED) {                              \
            return std::unexpected(EXPECTED.error()); \
        }                                             \
    } while (0)