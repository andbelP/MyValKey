#pragma once

#include <cstddef>
#include <list>
#include <string>
#include <unordered_set>

#include "Engine/DBEngine.hpp"

inline std::size_t GetSizeOfString(const std::string& str) {
    return sizeof(str) + str.capacity();
}
inline std::size_t GetSizeOfList(const std::list<std::string>& lst) {
    std::size_t size = sizeof(lst);
    for (const auto& str : lst) {
        size += GetSizeOfString(str);
    }
    return size;
}
inline std::size_t GetSizeOfSet(const std::unordered_set<std::string>& s) {
    std::size_t size = sizeof(s);
    for (const auto& str : s) {
        size += GetSizeOfString(str);
    }
    return size;
}

inline std::size_t GetSizeOfGeoEntry(const keyval::GeoEntry& geo) {
    std::size_t size = sizeof(geo);
    for (const auto& el : geo) {
        size += GetSizeOfString(el.first) + sizeof(el.second);
    }
    return size;
}
