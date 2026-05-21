#include <climits>
#include <cstddef>
#include <span>
#include <string>
#include <vector>

#include "CommandResult/CommandResult.hpp"
#include "Engine/DBEngine.hpp"
#include "Utils.hpp"
#include "ValKeyInterpreter.hpp"

namespace keyval {

ValKeyResult ValKeyInterpreter::GeoAdd(std::span<const std::string> args) {
    if (args.size() < 4) {
        return ValKeyError{"wrong number of arguments"};
    }

    std::string key = args[0];

    for (int i = 1; i < args.size(); i += 3) {
        if (i + 2 >= args.size()) {
            return ValKeyError{"wrong number of arguments"};
        }

        auto longitude_res = ParseDouble(args[i]);
        if (!longitude_res.has_value()) {
            return ValKeyError{"invalid longitude value"};
        }
        double longitude = longitude_res.value();

        auto latitude_res = ParseDouble(args[i + 1]);
        if (!latitude_res.has_value()) {
            return ValKeyError{"invalid latitude value"};
        }
        double latitude = latitude_res.value();

        std::string member = args[i + 2];

        auto res = engine_.GeoAdd(key, member, GeoPoint{latitude, longitude});
        if (!res.has_value()) {
            return ValKeyError{res.error().description};
        }
    }

    return Ok{};
}

ValKeyResult ValKeyInterpreter::GeoPos(std::span<const std::string> args) {
    if (args.size() < 2) {
        return ValKeyError{"wrong number of arguments"};
    }

    std::string key = args[0];
    std::vector<std::string> members(args.begin() + 1, args.end());

    std::vector<std::string> result;
    for (const auto& member : members) {
        auto res = engine_.GeoPos(key, member);
        if (!res.has_value()) {
            return ValKeyError{res.error().description};
        }
        result.push_back(std::to_string(res.value().latitude) + "," +
                         std::to_string(res.value().longitude));
    }

    return result;
}

ValKeyResult ValKeyInterpreter::GeoDist(std::span<const std::string> args) {
    if (args.size() < 3) {
        return ValKeyError{"wrong number of arguments"};
    }

    std::string key = args[0];
    std::string member1 = args[1];
    std::string member2 = args[2];
    std::string unit_str;
    if (args.size() == 4) {
        unit_str = args[3];
    } else {
        unit_str = "m";
    }

    GeoUnit unit;
    if (unit_str == "m") {
        unit = GeoUnit::kMeter;
    } else if (unit_str == "km") {
        unit = GeoUnit::kKilometer;
    } else if (unit_str == "mi") {
        unit = GeoUnit::kMile;
    } else if (unit_str == "ft") {
        unit = GeoUnit::kFoot;
    } else {
        return ValKeyError{"invalid unit"};
    }

    auto res = engine_.GeoDist(key, member1, member2, unit);
    if (!res.has_value()) {
        return ValKeyError{res.error().description};
    }

    return std::to_string(res.value());
}

ValKeyResult ValKeyInterpreter::GeoSearch(std::span<const std::string> args) {
    if (args.size() < 7) {
        return ValKeyError{"wrong number of arguments"};
    }
    std::string key = args[0];

    if (args[1] != "FROMLONLAT") {
        return ValKeyError{"invalid syntax"};
    }

    auto center_res = ParseDouble(args[2]);
    if (!center_res.has_value()) {
        return ValKeyError{"invalid longitude value"};
    }
    double longitude = center_res.value();

    center_res = ParseDouble(args[3]);
    if (!center_res.has_value()) {
        return ValKeyError{"invalid latitude value"};
    }
    double latitude = center_res.value();

    if (args[4] != "BYRADIUS") {
        return ValKeyError{"invalid syntax"};
    }

    auto radius_res = ParseDouble(args[5]);
    if (!radius_res.has_value()) {
        return ValKeyError{"invalid radius value"};
    }
    double radius = radius_res.value();

    std::string unit_str = args[6];
    GeoUnit unit;
    if (unit_str == "m") {
        unit = GeoUnit::kMeter;
    } else if (unit_str == "km") {
        unit = GeoUnit::kKilometer;
    } else if (unit_str == "mi") {
        unit = GeoUnit::kMile;
    } else if (unit_str == "ft") {
        unit = GeoUnit::kFoot;
    } else {
        return ValKeyError{"invalid unit"};
    }

    bool ascending = true;
    std::size_t count = LLONG_MAX;

    if (args.size() == 8) {
        if (ToUpper(args[7]) == "ASC") {
            ascending = true;
        } else if (ToUpper(args[7]) == "DESC") {
            ascending = false;
        } else {
            return ValKeyError{"invalid syntax"};
        }
    }

    if (args.size() == 9) {
        if (ToUpper(args[7]) != "COUNT") {
            return ValKeyError{"invalid syntax"};
        }
        auto count_res = ParseInt(args[8]);
        if (!count_res.has_value()) {
            return ValKeyError{"invalid count value"};
        }
        count = count_res.value();
    }

    if (args.size() == 10) {
        if (ToUpper(args[7]) == "ASC") {
            ascending = true;
        } else if (ToUpper(args[7]) == "DESC") {
            ascending = false;
        } else {
            return ValKeyError{"invalid syntax"};
        }
        if (ToUpper(args[8]) != "COUNT") {
            return ValKeyError{"invalid syntax"};
        }
        auto count_res = ParseInt(args[9]);
        if (!count_res.has_value()) {
            return ValKeyError{"invalid count value"};
        }
        count = count_res.value();
    }

    if (count < 0) {
        return ValKeyError{"invalid count value"};
    }

    auto res = engine_.GeoSearch(key, GeoPoint{latitude, longitude}, radius,
                                 unit, count, ascending);
    if (!res.has_value()) {
        return ValKeyError{res.error().description};
    }

    std::vector<std::string> ans;
    for (const auto& point : res.value()) {
        ans.push_back(std::to_string(point.latitude) + "," +
                      std::to_string(point.longitude));
    }

    return ans;
}

ValKeyResult ValKeyInterpreter::GeoSearchStore(
    std::span<const std::string> args) {
    if (args.size() < 8) {
        return ValKeyError{"wrong number of arguments"};
    }
    std::string dest = args[0];
    std::string source = args[1];

    if (args[2] != "FROMLONLAT") {
        return ValKeyError{"invalid syntax"};
    }

    auto center_res = ParseDouble(args[3]);
    if (!center_res.has_value()) {
        return ValKeyError{"invalid longitude value"};
    }
    double longitude = center_res.value();

    center_res = ParseDouble(args[4]);
    if (!center_res.has_value()) {
        return ValKeyError{"invalid latitude value"};
    }
    double latitude = center_res.value();

    if (ToUpper(args[5]) != "BYRADIUS") {
        return ValKeyError{"invalid syntax"};
    }

    auto radius_res = ParseDouble(args[6]);
    if (!radius_res.has_value()) {
        return ValKeyError{"invalid radius value"};
    }
    double radius = radius_res.value();

    std::string unit_str = args[7];
    GeoUnit unit;
    if (unit_str == "m") {
        unit = GeoUnit::kMeter;
    } else if (unit_str == "km") {
        unit = GeoUnit::kKilometer;
    } else if (unit_str == "mi") {
        unit = GeoUnit::kMile;
    } else if (unit_str == "ft") {
        unit = GeoUnit::kFoot;
    } else {
        return ValKeyError{"invalid unit"};
    }

    bool ascending = true;
    std::size_t count = LLONG_MAX;

    if (args.size() == 9) {
        if (ToUpper(args[8]) == "ASC") {
            ascending = true;
        } else if (ToUpper(args[8]) == "DESC") {
            ascending = false;
        } else {
            return ValKeyError{"invalid syntax"};
        }
    }

    if (args.size() == 10) {
        if (ToUpper(args[8]) != "COUNT") {
            return ValKeyError{"invalid syntax"};
        }
        auto count_res = ParseInt(args[9]);
        if (!count_res.has_value()) {
            return ValKeyError{"invalid count value"};
        }
        count = count_res.value();
    }

    if (args.size() == 11) {
        if (ToUpper(args[8]) == "ASC") {
            ascending = true;
        } else if (ToUpper(args[8]) == "DESC") {
            ascending = false;
        } else {
            return ValKeyError{"invalid syntax"};
        }
        if (ToUpper(args[9]) != "COUNT") {
            return ValKeyError{"invalid syntax"};
        }
        auto count_res = ParseInt(args[10]);
        if (!count_res.has_value()) {
            return ValKeyError{"invalid count value"};
        }
        count = count_res.value();
    }

    if (count < 0) {
        return ValKeyError{"invalid count value"};
    }

    auto res =
        engine_.GeoSearchStore(dest, source, GeoPoint{latitude, longitude},
                               radius, unit, count, ascending);
    if (!res.has_value()) {
        return ValKeyError{res.error().description};
    }

    return Ok{};
}

}  // namespace keyval
