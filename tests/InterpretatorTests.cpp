#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <thread>
#include <variant>
#include <vector>

#include "CommandResult/CommandResult.hpp"
#include "Engine/DBEngine.hpp"
#include "ValKeyInterpreter/ValKeyInterpreter.hpp"

using namespace keyval;

TEST(ValKeyInterpreterTest, SetReturnsOk) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);

    auto result =
        interpreter.Interpret(ValKeyCommand{"SET", {"key", "value"}});

    EXPECT_TRUE(std::holds_alternative<Ok>(result));
}

TEST(ValKeyInterpreterTest, GetExistingReturnsString) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);
    interpreter.Interpret(ValKeyCommand{"SET", {"key", "value"}});

    auto result = interpreter.Interpret(ValKeyCommand{"GET", {"key"}});

    ASSERT_TRUE(std::holds_alternative<std::string>(result));
    EXPECT_EQ(std::get<std::string>(result), "value");
}

TEST(ValKeyInterpreterTest, GetMissingReturnsNull) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);

    auto result = interpreter.Interpret(ValKeyCommand{"GET", {"missing"}});

    EXPECT_TRUE(std::holds_alternative<Null>(result));
}

TEST(ValKeyInterpreterTest, StrLenMissingReturnsZeroString) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);

    auto result = interpreter.Interpret(ValKeyCommand{"STRLEN", {"missing"}});

    ASSERT_TRUE(std::holds_alternative<std::string>(result));
    EXPECT_EQ(std::get<std::string>(result), "0");
}

TEST(ValKeyInterpreterTest, AppendCreatesStringAndReturnsLength) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);

    auto result = interpreter.Interpret(ValKeyCommand{"APPEND", {"key", "abc"}});

    ASSERT_TRUE(std::holds_alternative<std::string>(result));
    EXPECT_EQ(std::get<std::string>(result), "3");
}

TEST(ValKeyInterpreterTest, LPushMultipleReturnsLength) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);

    auto result =
        interpreter.Interpret(ValKeyCommand{"LPUSH", {"list", "a", "b", "c"}});

    ASSERT_TRUE(std::holds_alternative<std::string>(result));
    EXPECT_EQ(std::get<std::string>(result), "3");
}

TEST(ValKeyInterpreterTest, RPushMultipleReturnsLength) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);

    auto result =
        interpreter.Interpret(ValKeyCommand{"RPUSH", {"list", "a", "b"}});

    ASSERT_TRUE(std::holds_alternative<std::string>(result));
    EXPECT_EQ(std::get<std::string>(result), "2");
}

TEST(ValKeyInterpreterTest, LPopMissingReturnsNull) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);

    auto result = interpreter.Interpret(ValKeyCommand{"LPOP", {"missing"}});

    EXPECT_TRUE(std::holds_alternative<Null>(result));
}

TEST(ValKeyInterpreterTest, RPopWithCountReturnsVector) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);
    interpreter.Interpret(ValKeyCommand{"RPUSH", {"list", "a", "b", "c"}});

    auto result = interpreter.Interpret(ValKeyCommand{"RPOP", {"list", "2"}});

    ASSERT_TRUE(std::holds_alternative<std::vector<std::string>>(result));
    EXPECT_EQ(std::get<std::vector<std::string>>(result),
              std::vector<std::string>({"c", "b"}));
}

TEST(ValKeyInterpreterTest, LRangeMissingReturnsEmptyVector) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);

    auto result =
        interpreter.Interpret(ValKeyCommand{"LRANGE", {"missing", "0", "10"}});

    ASSERT_TRUE(std::holds_alternative<std::vector<std::string>>(result));
    EXPECT_TRUE(std::get<std::vector<std::string>>(result).empty());
}

TEST(ValKeyInterpreterTest, LIndexOutOfRangeReturnsNull) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);
    interpreter.Interpret(ValKeyCommand{"RPUSH", {"list", "a"}});

    auto result =
        interpreter.Interpret(ValKeyCommand{"LINDEX", {"list", "10"}});

    EXPECT_TRUE(std::holds_alternative<Null>(result));
}

TEST(ValKeyInterpreterTest, SAddReturnsAddedCount) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);

    auto result =
        interpreter.Interpret(ValKeyCommand{"SADD", {"set", "a", "b"}});

    ASSERT_TRUE(std::holds_alternative<std::string>(result));
    EXPECT_EQ(std::get<std::string>(result), "2");
}

TEST(ValKeyInterpreterTest, SAddDuplicateReturnsZero) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);
    interpreter.Interpret(ValKeyCommand{"SADD", {"set", "a"}});

    auto result = interpreter.Interpret(ValKeyCommand{"SADD", {"set", "a"}});

    ASSERT_TRUE(std::holds_alternative<std::string>(result));
    EXPECT_EQ(std::get<std::string>(result), "0");
}

TEST(ValKeyInterpreterTest, SIsMemberMissingKeyReturnsZero) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);

    auto result =
        interpreter.Interpret(ValKeyCommand{"SISMEMBER", {"missing", "a"}});

    ASSERT_TRUE(std::holds_alternative<std::string>(result));
    EXPECT_EQ(std::get<std::string>(result), "0");
}

TEST(ValKeyInterpreterTest, SMembersMissingKeyReturnsEmptyVector) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);

    auto result =
        interpreter.Interpret(ValKeyCommand{"SMEMBERS", {"missing"}});

    ASSERT_TRUE(std::holds_alternative<std::vector<std::string>>(result));
    EXPECT_TRUE(std::get<std::vector<std::string>>(result).empty());
}

TEST(ValKeyInterpreterTest, SUnionMissingKeysReturnsEmptyVector) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);

    auto result = interpreter.Interpret(ValKeyCommand{"SUNION", {"a", "b"}});

    ASSERT_TRUE(std::holds_alternative<std::vector<std::string>>(result));
    EXPECT_TRUE(std::get<std::vector<std::string>>(result).empty());
}

TEST(ValKeyInterpreterTest, TypeMissingReturnsNoneString) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);

    auto result = interpreter.Interpret(ValKeyCommand{"TYPE", {"missing"}});

    ASSERT_TRUE(std::holds_alternative<std::string>(result));
    EXPECT_EQ(std::get<std::string>(result), "none");
}

TEST(ValKeyInterpreterTest, ExistsReturnsCountString) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);
    interpreter.Interpret(ValKeyCommand{"SET", {"a", "1"}});

    auto result =
        interpreter.Interpret(ValKeyCommand{"EXISTS", {"a", "missing"}});

    ASSERT_TRUE(std::holds_alternative<std::string>(result));
    EXPECT_EQ(std::get<std::string>(result), "1");
}

TEST(ValKeyInterpreterTest, ConfigSetAndGetMaxMemory) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);

    auto set_result = interpreter.Interpret(
        ValKeyCommand{"CONFIG", {"SET", "maxmemory", "4096b"}});
    auto get_result =
        interpreter.Interpret(ValKeyCommand{"CONFIG", {"GET", "maxmemory"}});

    EXPECT_TRUE(std::holds_alternative<Ok>(set_result));

    ASSERT_TRUE(std::holds_alternative<std::string>(get_result));
    EXPECT_EQ(std::get<std::string>(get_result), "4096");
}

TEST(ValKeyInterpreterTest, MaxMemoryAllowsSmallWriteAndRejectsLargeWrite) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);

    auto config_result = interpreter.Interpret(
        ValKeyCommand{"CONFIG", {"SET", "maxmemory", "256b"}});
    auto small_result =
        interpreter.Interpret(ValKeyCommand{"SET", {"small", "x"}});
    auto large_result = interpreter.Interpret(
        ValKeyCommand{"SET", {"large", std::string(1000, 'x')}});

    EXPECT_TRUE(std::holds_alternative<Ok>(config_result));

    EXPECT_TRUE(std::holds_alternative<Ok>(small_result));

    ASSERT_TRUE(std::holds_alternative<ValKeyError>(large_result));
}

TEST(ValKeyInterpreterTest, ExpireExistingKeyReturnsOk) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);
    interpreter.Interpret(ValKeyCommand{"SET", {"key", "value"}});

    auto result =
        interpreter.Interpret(ValKeyCommand{"EXPIRE", {"key", "10"}});

    EXPECT_TRUE(std::holds_alternative<Ok>(result));
}

TEST(ValKeyInterpreterTest, TtlMissingKeyReturnsMinusTwo) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);

    auto result = interpreter.Interpret(ValKeyCommand{"TTL", {"missing"}});

    ASSERT_TRUE(std::holds_alternative<std::string>(result));
    EXPECT_EQ(std::get<std::string>(result), "-2");
}

TEST(ValKeyInterpreterTest, TtlWithoutExpireReturnsMinusOne) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);
    interpreter.Interpret(ValKeyCommand{"SET", {"key", "value"}});

    auto result = interpreter.Interpret(ValKeyCommand{"TTL", {"key"}});

    ASSERT_TRUE(std::holds_alternative<std::string>(result));
    EXPECT_EQ(std::get<std::string>(result), "-1");
}

TEST(ValKeyInterpreterTest, SetResetsTtl) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);
    interpreter.Interpret(ValKeyCommand{"SET", {"key", "value"}});
    interpreter.Interpret(ValKeyCommand{"EXPIRE", {"key", "10"}});
    interpreter.Interpret(ValKeyCommand{"SET", {"key", "new_value"}});

    auto result = interpreter.Interpret(ValKeyCommand{"TTL", {"key"}});

    ASSERT_TRUE(std::holds_alternative<std::string>(result));
    EXPECT_EQ(std::get<std::string>(result), "-1");
}

TEST(ValKeyInterpreterTest, KeyExpiresAfterTtl) {
    DBEngine engine;
    ValKeyInterpreter interpreter(engine);
    interpreter.Interpret(ValKeyCommand{"SET", {"key", "value"}});
    interpreter.Interpret(ValKeyCommand{"EXPIRE", {"key", "1"}});

    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto get_result = interpreter.Interpret(ValKeyCommand{"GET", {"key"}});
    auto ttl_result = interpreter.Interpret(ValKeyCommand{"TTL", {"key"}});

    EXPECT_TRUE(std::holds_alternative<Null>(get_result));

    ASSERT_TRUE(std::holds_alternative<std::string>(ttl_result));
    EXPECT_EQ(std::get<std::string>(ttl_result), "-2");
}