#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <unordered_set>
#include <vector>

#include "Engine/DBEngine.hpp"

using namespace keyval;
using namespace std::chrono_literals;

class DBEngineTest : public ::testing::Test {
   protected:
    DBEngine db;

    void SetUp() override { db.FlushDb(); }
};

TEST_F(DBEngineTest, SetStoresString) {
    auto result = db.Set("key", "value");

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(db.Get("key").value(), "value");
}

TEST_F(DBEngineTest, SetOverwritesString) {
    db.Set("key", "old");

    auto result = db.Set("key", "new");

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(db.Get("key").value(), "new");
}

TEST_F(DBEngineTest, SetStoresEmptyString) {
    db.Set("key", "");

    EXPECT_EQ(db.Get("key").value(), "");
}

TEST_F(DBEngineTest, KeysAreCaseSensitive) {
    db.Set("key", "lower");
    db.Set("Key", "upper");

    EXPECT_EQ(db.Get("key").value(), "lower");
    EXPECT_EQ(db.Get("Key").value(), "upper");
}

TEST_F(DBEngineTest, GetMissingKeyReturnsError) {
    auto result = db.Get("missing");

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error::ErrorCode::kKeyNotFound);
}

TEST_F(DBEngineTest, GetListReturnsWrongType) {
    db.LPush("key", "value");

    auto result = db.Get("key");

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error::ErrorCode::kWrongType);
}

TEST_F(DBEngineTest, SetOnListReturnsWrongType) {
    db.LPush("key", "value");

    auto result = db.Set("key", "string");

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error::ErrorCode::kWrongType);
}

TEST_F(DBEngineTest, TypeForString) {
    db.Set("key", "value");

    auto result = db.Type("key");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), StorageType::kString);
}

TEST_F(DBEngineTest, LPushCreatesList) {
    auto result = db.LPush("list", "a");

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(db.LLen("list").value(), 1);
}

TEST_F(DBEngineTest, LPushAddsToFront) {
    db.LPush("list", "b");
    db.LPush("list", "a");

    EXPECT_EQ(db.LIndex("list", 0).value(), "a");
}

TEST_F(DBEngineTest, RPushCreatesList) {
    auto result = db.RPush("list", "a");

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(db.LLen("list").value(), 1);
}

TEST_F(DBEngineTest, RPushAddsToBack) {
    db.RPush("list", "a");
    db.RPush("list", "b");

    EXPECT_EQ(db.LIndex("list", 1).value(), "b");
}

TEST_F(DBEngineTest, MixedPushKeepsExpectedOrder) {
    db.RPush("list", "b");
    db.LPush("list", "a");
    db.RPush("list", "c");

    auto result = db.LRange("list", 0, 2);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), std::vector<std::string>({"a", "b", "c"}));
}

TEST_F(DBEngineTest, LLenReturnsListSize) {
    db.RPush("list", "a");
    db.RPush("list", "b");

    EXPECT_EQ(db.LLen("list").value(), 2);
}

TEST_F(DBEngineTest, LLenMissingKeyReturnsError) {
    auto result = db.LLen("missing");

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error::ErrorCode::kKeyNotFound);
}

TEST_F(DBEngineTest, LLenWrongTypeReturnsError) {
    db.Set("key", "value");

    auto result = db.LLen("key");

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error::ErrorCode::kWrongType);
}

TEST_F(DBEngineTest, LIndexReturnsElementByPositiveIndex) {
    db.RPush("list", "a");
    db.RPush("list", "b");

    EXPECT_EQ(db.LIndex("list", 1).value(), "b");
}

TEST_F(DBEngineTest, LIndexReturnsElementByNegativeIndex) {
    db.RPush("list", "a");
    db.RPush("list", "b");

    EXPECT_EQ(db.LIndex("list", -1).value(), "b");
}

TEST_F(DBEngineTest, LIndexOutOfRangeReturnsError) {
    db.RPush("list", "a");

    auto result = db.LIndex("list", 10);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error::ErrorCode::kInvalidCommand);
}

TEST_F(DBEngineTest, LRangeReturnsWholeList) {
    db.RPush("list", "a");
    db.RPush("list", "b");
    db.RPush("list", "c");

    auto result = db.LRange("list", 0, 2);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), std::vector<std::string>({"a", "b", "c"}));
}

TEST_F(DBEngineTest, LRangeReturnsMiddlePart) {
    db.RPush("list", "a");
    db.RPush("list", "b");
    db.RPush("list", "c");

    auto result = db.LRange("list", 1, 2);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), std::vector<std::string>({"b", "c"}));
}

TEST_F(DBEngineTest, LRangeSupportsNegativeIndexes) {
    db.RPush("list", "a");
    db.RPush("list", "b");
    db.RPush("list", "c");

    auto result = db.LRange("list", -2, -1);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), std::vector<std::string>({"b", "c"}));
}

TEST_F(DBEngineTest, LRangeInvalidRangeReturnsError) {
    db.RPush("list", "a");
    db.RPush("list", "b");

    auto result = db.LRange("list", 1, 0);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error::ErrorCode::kInvalidCommand);
}

TEST_F(DBEngineTest, LPopRemovesFrontElement) {
    db.RPush("list", "a");
    db.RPush("list", "b");

    db.LPop("list");

    EXPECT_EQ(db.LIndex("list", 0).value(), "b");
}

TEST_F(DBEngineTest, RPopRemovesBackElement) {
    db.RPush("list", "a");
    db.RPush("list", "b");

    db.RPop("list");

    EXPECT_EQ(db.LIndex("list", 0).value(), "a");
}

TEST_F(DBEngineTest, LPopDeletesKeyAfterLastElement) {
    db.RPush("list", "a");

    db.LPop("list");

    EXPECT_FALSE(db.Exists("list").value());
}

TEST_F(DBEngineTest, RPopMissingKeyReturnsError) {
    auto result = db.RPop("missing");

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error::ErrorCode::kKeyNotFound);
}

TEST_F(DBEngineTest, TypeForList) {
    db.LPush("list", "a");

    EXPECT_EQ(db.Type("list").value(), StorageType::kList);
}

TEST_F(DBEngineTest, SAddCreatesSet) {
    auto result = db.SAdd("set", "a");

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(db.SCard("set").value(), 1);
}

TEST_F(DBEngineTest, SAddIgnoresDuplicateMembers) {
    db.SAdd("set", "a");
    db.SAdd("set", "a");

    EXPECT_EQ(db.SCard("set").value(), 1);
}

TEST_F(DBEngineTest, SIsMemberReturnsTrueForExistingMember) {
    db.SAdd("set", "a");

    EXPECT_TRUE(db.SIsMember("set", "a").value());
}

TEST_F(DBEngineTest, SIsMemberReturnsFalseForMissingMember) {
    db.SAdd("set", "a");

    EXPECT_FALSE(db.SIsMember("set", "b").value());
}

TEST_F(DBEngineTest, SIsMemberMissingKeyReturnsError) {
    auto result = db.SIsMember("missing", "a");

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error::ErrorCode::kKeyNotFound);
}

TEST_F(DBEngineTest, SRemRemovesMember) {
    db.SAdd("set", "a");

    db.SRem("set", "a");

    EXPECT_FALSE(db.SIsMember("set", "a").value());
}

TEST_F(DBEngineTest, SRemMissingMemberDoesNotFail) {
    db.SAdd("set", "a");

    auto result = db.SRem("set", "b");

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(db.SCard("set").value(), 1);
}

TEST_F(DBEngineTest, SMembersReturnsAllMembers) {
    db.SAdd("set", "a");
    db.SAdd("set", "b");

    auto result = db.SMembers("set");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), std::unordered_set<std::string>({"a", "b"}));
}

TEST_F(DBEngineTest, SCardWrongTypeReturnsError) {
    db.Set("key", "value");

    auto result = db.SCard("key");

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error::ErrorCode::kWrongType);
}

TEST_F(DBEngineTest, TypeForSet) {
    db.SAdd("set", "a");

    EXPECT_EQ(db.Type("set").value(), StorageType::kSet);
}

TEST_F(DBEngineTest, DelRemovesExistingKey) {
    db.Set("key", "value");

    auto result = db.Del("key");

    EXPECT_TRUE(result.has_value());
    EXPECT_FALSE(db.Exists("key").value());
}

TEST_F(DBEngineTest, FlushDbRemovesAllKeys) {
    db.Set("a", "1");
    db.Set("b", "2");

    db.FlushDb();

    EXPECT_FALSE(db.Exists("a").value());
    EXPECT_FALSE(db.Exists("b").value());
}

TEST_F(DBEngineTest, KeysStarReturnsAllKeys) {
    db.Set("a", "1");
    db.Set("b", "2");
    db.Set("c", "3");

    auto result = db.Keys("*");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::unordered_set<std::string>(result.value().begin(),
                                              result.value().end()),
              std::unordered_set<std::string>({"a", "b", "c"}));
}

TEST_F(DBEngineTest, KeysPrefixPatternReturnsMatchingKeys) {
    db.Set("user:1", "Ann");
    db.Set("user:2", "Bob");
    db.Set("admin:1", "Root");

    auto result = db.Keys("user:*");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::unordered_set<std::string>(result.value().begin(),
                                              result.value().end()),
              std::unordered_set<std::string>({"user:1", "user:2"}));
}

TEST_F(DBEngineTest, KeysQuestionPatternMatchesOneCharacter) {
    db.Set("key1", "a");
    db.Set("key2", "b");
    db.Set("key10", "c");

    auto result = db.Keys("key?");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::unordered_set<std::string>(result.value().begin(),
                                              result.value().end()),
              std::unordered_set<std::string>({"key1", "key2"}));
}

TEST_F(DBEngineTest, KeysCharClassPatternMatchesSet) {
    db.Set("file:a", "1");
    db.Set("file:b", "2");
    db.Set("file:c", "3");

    auto result = db.Keys("file:[ab]");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::unordered_set<std::string>(result.value().begin(),
                                              result.value().end()),
              std::unordered_set<std::string>({"file:a", "file:b"}));
}

TEST_F(DBEngineTest, KeysReturnsEmptyVectorWhenNothingMatches) {
    db.Set("first", "1");
    db.Set("second", "2");

    auto result = db.Keys("missing:*");

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value().empty());
}
