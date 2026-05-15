#pragma once

#include <unordered_map>
#include <span>
#include <string>

#include "Engine/DBEngine.hpp"
#include "Command/Command.hpp"

namespace keyval {

class ValKeyInterpreter {

    DBEngine& engine_;

    ValKeyResult Set(std::span<const std::string> args);

    ValKeyResult Get(std::span<const std::string> args);

    ValKeyResult StrLen(std::span<const std::string> args);

    ValKeyResult LPush(std::span<const std::string> args);

    ValKeyResult RPush(std::span<const std::string> args);

    ValKeyResult LPop(std::span<const std::string> args);

    ValKeyResult RPop(std::span<const std::string> args);

    ValKeyResult LLen(std::span<const std::string> args);

    ValKeyResult LRange(std::span<const std::string> args);

    ValKeyResult LIndex(std::span<const std::string> args);

    ValKeyResult LSet(std::span<const std::string> args);

    ValKeyResult LInsert(std::span<const std::string> args);

    ValKeyResult SAdd(std::span<const std::string> args);

    ValKeyResult SRem(std::span<const std::string> args);

    ValKeyResult SIsMember(std::span<const std::string> args);

    ValKeyResult SMembers(std::span<const std::string> args);

    ValKeyResult SCard(std::span<const std::string> args);

    ValKeyResult SUnion(std::span<const std::string> args);

    ValKeyResult SInter(std::span<const std::string> args);

    ValKeyResult SDiff(std::span<const std::string> args);

    ValKeyResult SMove(std::span<const std::string> args);

    ValKeyResult Del(std::span<const std::string> args);

    ValKeyResult Exists(std::span<const std::string> args);

    ValKeyResult Type(std::span<const std::string> args);

    inline static const std::unordered_map<std::string, ValKeyResult (ValKeyInterpreter::*)(std::span<const std::string>)> methods_{
        {"SET", &ValKeyInterpreter::Set},
        {"GET", &ValKeyInterpreter::Get},
        {"STRLEN", &ValKeyInterpreter::StrLen},
        {"LPUSH", &ValKeyInterpreter::LPush},
        {"RPUSH", &ValKeyInterpreter::RPush},
        {"LPOP", &ValKeyInterpreter::LPop},
        {"RPOP", &ValKeyInterpreter::RPop},
        {"LLEN", &ValKeyInterpreter::LLen},
        {"LRANGE", &ValKeyInterpreter::LRange},
        {"LINDEX", &ValKeyInterpreter::LIndex},
        {"LSET", &ValKeyInterpreter::LSet},
        {"LINSERT", &ValKeyInterpreter::LInsert},
        {"SADD", &ValKeyInterpreter::SAdd},
        {"SREM", &ValKeyInterpreter::SRem},
        {"SISMEMBER", &ValKeyInterpreter::SIsMember},
        {"SMEMBERS", &ValKeyInterpreter::SMembers},
        {"SCARD", &ValKeyInterpreter::SCard},
        {"SUNION", &ValKeyInterpreter::SUnion},
        {"SINTER", &ValKeyInterpreter::SInter},
        {"SDIFF", &ValKeyInterpreter::SDiff},
        {"SMOVE", &ValKeyInterpreter::SMove},
        {"DEL", &ValKeyInterpreter::Del},
        {"EXISTS", &ValKeyInterpreter::Exists},
        {"TYPE", &ValKeyInterpreter::Type}
    };

public:

    ValKeyResult Interpret(const ValKeyCommand& command);

    ValKeyInterpreter(DBEngine& engine) : engine_(engine) {}

};

}