#include <algorithm>
#include <cmath>
#include <cstddef>
#include <expected>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Engine/DBEngine.hpp"
#include "Engine/StorageTypes.hpp"
#include "Engine/TypesSizes.hpp"
#include "ErrorsHandling/Error.hpp"

namespace keyval {

namespace {
double ConvertUnitsFromKm(double distance_km, GeoUnit unit) {
    switch (unit) {
        case GeoUnit::kMeter:
            return distance_km * 1000.0;
        case GeoUnit::kKilometer:
            return distance_km;
        case GeoUnit::kMile:
            return distance_km * 0.621371;
        case GeoUnit::kFoot:
            return distance_km * 3280.84;
    }
    return -1;
}

double ToRadians(double degrees) {
    const double pi = 3.14159265358979323846;
    return degrees * pi / 180.0;
}

double HaversinFormulaKm(GeoPoint first, GeoPoint second) {
    double phi1 = ToRadians(first.latitude);
    double phi2 = ToRadians(second.latitude);

    double delta_phi = ToRadians(second.latitude - first.latitude);
    double delta_lambda = ToRadians(second.longitude - first.longitude);

    double sin_delta_phi = std::sin(delta_phi / 2.0);
    double sin_delta_lambda = std::sin(delta_lambda / 2.0);

    double expression =
        sin_delta_phi * sin_delta_phi +
        std::cos(phi1) * std::cos(phi2) * sin_delta_lambda * sin_delta_lambda;

    return 2.0 * 6372.8 * std::asin(std::sqrt(expression));
}
}  // namespace

std::expected<void, error::Error> DBEngine::GeoAdd(std::string_view key,
                                                   std::string_view member,
                                                   GeoPoint point) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));

    if (it != storage_.end()) {
        if (it->second.type != StorageType::kGeo) {
            return std::unexpected(
                error::Error{error::ErrorCode::kWrongType,
                             "DB entry already exists with different type"});
        }

        auto& geo = std::get<GeoEntry>(it->second.value);

        if (!geo.contains(std::string(member))) {
            if (max_memory_usage_.has_value() &&
                !CanAddBytes(GetSizeOfString(std::string(member)) +
                             sizeof(GeoPoint))) {
                return std::unexpected(error::Error{
                    error::ErrorCode::kMaxMemoryExceeded,
                    "Can't store GeoPoint: max memory usage exceeded"});
            }
        }

        geo[std::string(member)] = point;
        return {};
    }

    GeoEntry geo;
    geo[std::string(member)] = point;

    if (max_memory_usage_.has_value() && !CanAddBytes(GetSizeOfGeoEntry(geo))) {
        return std::unexpected(
            error::Error{error::ErrorCode::kMaxMemoryExceeded,
                         "Can't store GeoPoint: max memory usage exceeded"});
    }

    storage_[std::string(key)] =
        StorageEntry(std::move(geo), StorageType::kGeo);
    return {};
}

std::expected<GeoPoint, error::Error> DBEngine::GeoPos(
    std::string_view key, std::string_view member) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));
    if (it == storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }

    if (it->second.type != StorageType::kGeo) {
        return std::unexpected(
            error::Error{error::ErrorCode::kWrongType,
                         "DB entry already exists with different type"});
    }

    const auto& geo = std::get<GeoEntry>(it->second.value);
    auto point_it = geo.find(std::string(member));

    if (point_it == geo.end()) {
        return std::unexpected(error::Error{error::ErrorCode::kKeyNotFound,
                                            "Geo member not found"});
    }

    return point_it->second;
}

std::expected<double, error::Error> DBEngine::GeoDist(std::string_view key,
                                                      std::string_view member1,
                                                      std::string_view member2,
                                                      GeoUnit unit) {
    auto point1 = GeoPos(key, member1);
    if (!point1.has_value()) {
        return std::unexpected(point1.error());
    }

    auto point2 = GeoPos(key, member2);
    if (!point2.has_value()) {
        return std::unexpected(point2.error());
    }

    const double distance_km =
        HaversinFormulaKm(point1.value(), point2.value());
    return ConvertUnitsFromKm(distance_km, unit);
}

std::expected<std::vector<GeoPoint>, error::Error> DBEngine::GeoSearch(
    std::string_view key, GeoPoint center, double radius, GeoUnit unit,
    std::size_t count, bool ascending) {
    DeleteIfExpired(key);

    auto it = storage_.find(std::string(key));
    if (it == storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }

    if (it->second.type != StorageType::kGeo) {
        return std::unexpected(
            error::Error{error::ErrorCode::kWrongType,
                         "DB entry already exists with different type"});
    }

    const auto& geo = std::get<GeoEntry>(it->second.value);

    struct GeoPointWithDistance {
        GeoPoint point;
        double distance;
    };

    std::vector<GeoPointWithDistance> items;
    for (auto& el : geo) {
        double distance =
            ConvertUnitsFromKm(HaversinFormulaKm(center, el.second), unit);
        if (distance <= radius) {
            items.push_back({el.second, distance});
        }
    }

    std::ranges::sort(items, [ascending](const GeoPointWithDistance& first,
                                         const GeoPointWithDistance& second) {
        if (ascending) {
            return first.distance < second.distance;
        }
        return first.distance > second.distance;
    });

    std::vector<GeoPoint> ans;
    ans.reserve(items.size());
    for (auto& el : items) {
        if (ans.size() >= count) {
            break;
        }
        ans.push_back(el.point);
    }

    return ans;
}

std::expected<std::vector<GeoPoint>, error::Error> DBEngine::GeoSearchStore(
    std::string_view dest, std::string_view source, GeoPoint center,
    double radius, GeoUnit unit, std::size_t count, bool ascending) {
    DeleteIfExpired(source);
    DeleteIfExpired(dest);

    auto it = storage_.find(std::string(source));
    if (it == storage_.end()) {
        return std::unexpected(
            error::Error{error::ErrorCode::kKeyNotFound, "Key not found"});
    }

    if (it->second.type != StorageType::kGeo) {
        return std::unexpected(
            error::Error{error::ErrorCode::kWrongType,
                         "DB entry already exists with different type"});
    }

    const auto& geo = std::get<GeoEntry>(it->second.value);

    struct GeoPointWithDistance {
        std::string member;
        GeoPoint point;
        double distance;
    };

    std::vector<GeoPointWithDistance> items;
    for (auto& el : geo) {
        double distance =
            ConvertUnitsFromKm(HaversinFormulaKm(center, el.second), unit);
        if (distance <= radius) {
            items.push_back({el.first, el.second, distance});
        }
    }

    std::ranges::sort(items, [ascending](const GeoPointWithDistance& first,
                                         const GeoPointWithDistance& second) {
        if (ascending) {
            return first.distance < second.distance;
        }
        return first.distance > second.distance;
    });

    auto dest_it = storage_.find(std::string(dest));
    if (dest_it != storage_.end()) {
        return std::unexpected(error::Error{error::ErrorCode::kKeyAlreadyExists,
                                            "Destination key already exists"});
    }

    GeoEntry result;
    for (std::size_t i = 0; i < items.size() && i < count; i++) {
        result[items[i].member] = items[i].point;
    }

    if (max_memory_usage_.has_value() &&
        !CanAddBytes(GetSizeOfGeoEntry(result))) {
        return std::unexpected(
            error::Error{error::ErrorCode::kMaxMemoryExceeded,
                         "Cant store GeoPoint: max memory usage exceeded"});
    }
    storage_[std::string(dest)] =
        StorageEntry(std::move(result), StorageType::kGeo);

    std::vector<GeoPoint> ans;
    ans.reserve(items.size());
    for (auto& el : items) {
        if (ans.size() >= count) {
            break;
        }
        ans.push_back(el.point);
    }

    return ans;
}

}  // namespace keyval
