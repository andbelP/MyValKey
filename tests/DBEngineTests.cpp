#include <gtest/gtest.h>

#include <string>
#include <unordered_set>
#include <vector>

#include "Engine/DBEngine.hpp"
#include "Engine/StorageTypes.hpp"
#include "ErrorsHandling/Error.hpp"

using namespace keyval;

class DBEngineTest : public testing::Test {
   public:
    DBEngine db_;

    void SetUp() override { db_.FlushDb(); }
};

TEST_F(DBEngineTest, SetStoresString) {
    auto result = db_.Set("key", "value");

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(db_.Get("key").value(), "value");
}

TEST_F(DBEngineTest, SetOverwritesString) {
    db_.Set("key", "old");

    auto result = db_.Set("key", "new");

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(db_.Get("key").value(), "new");
}

TEST_F(DBEngineTest, SetStoresEmptyString) {
    db_.Set("key", "");

    EXPECT_EQ(db_.Get("key").value(), "");
}

TEST_F(DBEngineTest, KeysAreCaseSensitive) {
    db_.Set("key", "lower");
    db_.Set("Key", "upper");

    EXPECT_EQ(db_.Get("key").value(), "lower");
    EXPECT_EQ(db_.Get("Key").value(), "upper");
}

TEST_F(DBEngineTest, GetMissingKeyReturnsError) {
    auto result = db_.Get("missing");

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error::ErrorCode::kKeyNotFound);
}

TEST_F(DBEngineTest, GetListReturnsWrongType) {
    db_.LPush("key", "value");

    auto result = db_.Get("key");

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error::ErrorCode::kWrongType);
}

TEST_F(DBEngineTest, SetOnListReturnsWrongType) {
    db_.LPush("key", "value");

    auto result = db_.Set("key", "string");

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error::ErrorCode::kWrongType);
}

TEST_F(DBEngineTest, TypeForString) {
    db_.Set("key", "value");

    auto result = db_.Type("key");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), StorageType::kString);
}

TEST_F(DBEngineTest, LPushCreatesList) {
    auto result = db_.LPush("list", "a");

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(db_.LLen("list").value(), 1);
}

TEST_F(DBEngineTest, LPushAddsToFront) {
    db_.LPush("list", "b");
    db_.LPush("list", "a");

    EXPECT_EQ(db_.LIndex("list", 0).value(), "a");
}

TEST_F(DBEngineTest, RPushCreatesList) {
    auto result = db_.RPush("list", "a");

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(db_.LLen("list").value(), 1);
}

TEST_F(DBEngineTest, RPushAddsToBack) {
    db_.RPush("list", "a");
    db_.RPush("list", "b");

    EXPECT_EQ(db_.LIndex("list", 1).value(), "b");
}

TEST_F(DBEngineTest, MixedPushKeepsExpectedOrder) {
    db_.RPush("list", "b");
    db_.LPush("list", "a");
    db_.RPush("list", "c");

    auto result = db_.LRange("list", 0, 2);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), std::vector<std::string>({"a", "b", "c"}));
}

TEST_F(DBEngineTest, LLenReturnsListSize) {
    db_.RPush("list", "a");
    db_.RPush("list", "b");

    EXPECT_EQ(db_.LLen("list").value(), 2);
}

TEST_F(DBEngineTest, LLenMissingKeyReturnsError) {
    auto result = db_.LLen("missing");

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error::ErrorCode::kKeyNotFound);
}

TEST_F(DBEngineTest, LLenWrongTypeReturnsError) {
    db_.Set("key", "value");

    auto result = db_.LLen("key");

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error::ErrorCode::kWrongType);
}

TEST_F(DBEngineTest, LIndexReturnsElementByPositiveIndex) {
    db_.RPush("list", "a");
    db_.RPush("list", "b");

    EXPECT_EQ(db_.LIndex("list", 1).value(), "b");
}

TEST_F(DBEngineTest, LIndexOutOfRangeReturnsError) {
    db_.RPush("list", "a");

    auto result = db_.LIndex("list", 10);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error::ErrorCode::kInvalidCommand);
}

TEST_F(DBEngineTest, LRangeReturnsWholeList) {
    db_.RPush("list", "a");
    db_.RPush("list", "b");
    db_.RPush("list", "c");

    auto result = db_.LRange("list", 0, 2);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), std::vector<std::string>({"a", "b", "c"}));
}

TEST_F(DBEngineTest, LRangeReturnsMiddlePart) {
    db_.RPush("list", "a");
    db_.RPush("list", "b");
    db_.RPush("list", "c");

    auto result = db_.LRange("list", 1, 2);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), std::vector<std::string>({"b", "c"}));
}

TEST_F(DBEngineTest, LRangeInvalidRangeReturnsError) {
    db_.RPush("list", "a");
    db_.RPush("list", "b");

    auto result = db_.LRange("list", 1, 0);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error::ErrorCode::kInvalidCommand);
}

TEST_F(DBEngineTest, LPopRemovesFrontElement) {
    db_.RPush("list", "a");
    db_.RPush("list", "b");

    db_.LPop("list");

    EXPECT_EQ(db_.LIndex("list", 0).value(), "b");
}

TEST_F(DBEngineTest, RPopRemovesBackElement) {
    db_.RPush("list", "a");
    db_.RPush("list", "b");

    db_.RPop("list");

    EXPECT_EQ(db_.LIndex("list", 0).value(), "a");
}

TEST_F(DBEngineTest, LPopDeletesKeyAfterLastElement) {
    db_.RPush("list", "a");

    db_.LPop("list");

    EXPECT_FALSE(db_.Exists("list").value());
}

TEST_F(DBEngineTest, RPopMissingKeyReturnsError) {
    auto result = db_.RPop("missing");

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error::ErrorCode::kKeyNotFound);
}

TEST_F(DBEngineTest, TypeForList) {
    db_.LPush("list", "a");

    EXPECT_EQ(db_.Type("list").value(), StorageType::kList);
}

TEST_F(DBEngineTest, SAddCreatesSet) {
    auto result = db_.SAdd("set", "a");

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(db_.SCard("set").value(), 1);
}

TEST_F(DBEngineTest, SAddIgnoresDuplicateMembers) {
    db_.SAdd("set", "a");
    db_.SAdd("set", "a");

    EXPECT_EQ(db_.SCard("set").value(), 1);
}

TEST_F(DBEngineTest, SIsMemberReturnsTrueForExistingMember) {
    db_.SAdd("set", "a");

    EXPECT_TRUE(db_.SIsMember("set", "a").value());
}

TEST_F(DBEngineTest, SIsMemberReturnsFalseForMissingMember) {
    db_.SAdd("set", "a");

    EXPECT_FALSE(db_.SIsMember("set", "b").value());
}

TEST_F(DBEngineTest, SIsMemberMissingKeyReturnsError) {
    auto result = db_.SIsMember("missing", "a");

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error::ErrorCode::kKeyNotFound);
}

TEST_F(DBEngineTest, SRemRemovesMember) {
    db_.SAdd("set", "a");

    db_.SRem("set", "a");

    EXPECT_FALSE(db_.SIsMember("set", "a").value());
}

TEST_F(DBEngineTest, SRemMissingMemberDoesNotFail) {
    db_.SAdd("set", "a");

    auto result = db_.SRem("set", "b");

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(db_.SCard("set").value(), 1);
}

TEST_F(DBEngineTest, SMembersReturnsAllMembers) {
    db_.SAdd("set", "a");
    db_.SAdd("set", "b");

    auto result = db_.SMembers("set");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), std::unordered_set<std::string>({"a", "b"}));
}

TEST_F(DBEngineTest, SCardWrongTypeReturnsError) {
    db_.Set("key", "value");

    auto result = db_.SCard("key");

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error::ErrorCode::kWrongType);
}

TEST_F(DBEngineTest, TypeForSet) {
    db_.SAdd("set", "a");

    EXPECT_EQ(db_.Type("set").value(), StorageType::kSet);
}

TEST_F(DBEngineTest, DelRemovesExistingKey) {
    db_.Set("key", "value");

    auto result = db_.Del("key");

    EXPECT_TRUE(result.has_value());
    EXPECT_FALSE(db_.Exists("key").value());
}

TEST_F(DBEngineTest, FlushDbRemovesAllKeys) {
    db_.Set("a", "1");
    db_.Set("b", "2");

    db_.FlushDb();

    EXPECT_FALSE(db_.Exists("a").value());
    EXPECT_FALSE(db_.Exists("b").value());
}

TEST_F(DBEngineTest, KeysStarReturnsAllKeys) {
    db_.Set("a", "1");
    db_.Set("b", "2");
    db_.Set("c", "3");

    auto result = db_.Keys("*");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::unordered_set<std::string>(result.value().begin(),
                                              result.value().end()),
              std::unordered_set<std::string>({"a", "b", "c"}));
}

TEST_F(DBEngineTest, KeysPrefixPatternReturnsMatchingKeys) {
    db_.Set("user:1", "Ann");
    db_.Set("user:2", "Bob");
    db_.Set("admin:1", "Root");

    auto result = db_.Keys("user:*");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::unordered_set<std::string>(result.value().begin(),
                                              result.value().end()),
              std::unordered_set<std::string>({"user:1", "user:2"}));
}

TEST_F(DBEngineTest, KeysQuestionPatternMatchesOneCharacter) {
    db_.Set("key1", "a");
    db_.Set("key2", "b");
    db_.Set("key10", "c");

    auto result = db_.Keys("key?");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::unordered_set<std::string>(result.value().begin(),
                                              result.value().end()),
              std::unordered_set<std::string>({"key1", "key2"}));
}

TEST_F(DBEngineTest, KeysCharClassPatternMatchesSet) {
    db_.Set("file:a", "1");
    db_.Set("file:b", "2");
    db_.Set("file:c", "3");

    auto result = db_.Keys("file:[ab]");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::unordered_set<std::string>(result.value().begin(),
                                              result.value().end()),
              std::unordered_set<std::string>({"file:a", "file:b"}));
}

TEST_F(DBEngineTest, KeysReturnsEmptyVectorWhenNothingMatches) {
    db_.Set("first", "1");
    db_.Set("second", "2");

    auto result = db_.Keys("missing:*");

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value().empty());
}

TEST_F(DBEngineTest, GeoAddCreatesGeoIndex) {
    keyval::DBEngine engine;

    auto result =
        engine.GeoAdd("cities", "Moscow", keyval::GeoPoint{55.7558, 37.6173});

    ASSERT_TRUE(result.has_value());

    auto type = engine.Type("cities");
    ASSERT_TRUE(type.has_value());
    EXPECT_EQ(type.value(), StorageType::kGeo);
}

TEST_F(DBEngineTest, GeoPosReturnsStoredPoint) {
    keyval::DBEngine engine;

    ASSERT_TRUE(
        engine.GeoAdd("cities", "Moscow", keyval::GeoPoint{55.7558, 37.6173})
            .has_value());

    auto result = engine.GeoPos("cities", "Moscow");

    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result.value().latitude, 55.7558);
    EXPECT_DOUBLE_EQ(result.value().longitude, 37.6173);
}

TEST_F(DBEngineTest, GeoAddUpdatesExistingMember) {
    keyval::DBEngine engine;

    ASSERT_TRUE(
        engine.GeoAdd("cities", "Moscow", keyval::GeoPoint{55.7558, 37.6173})
            .has_value());
    ASSERT_TRUE(engine.GeoAdd("cities", "Moscow", keyval::GeoPoint{55.0, 37.0})
                    .has_value());

    auto result = engine.GeoPos("cities", "Moscow");

    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result.value().latitude, 55.0);
    EXPECT_DOUBLE_EQ(result.value().longitude, 37.0);
}

TEST_F(DBEngineTest, GeoPosMissingKeyReturnsError) {
    keyval::DBEngine engine;

    auto result = engine.GeoPos("missing", "Moscow");

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, keyval::error::ErrorCode::kKeyNotFound);
}

TEST_F(DBEngineTest, GeoPosMissingMemberReturnsError) {
    keyval::DBEngine engine;

    ASSERT_TRUE(
        engine.GeoAdd("cities", "Moscow", keyval::GeoPoint{55.7558, 37.6173})
            .has_value());

    auto result = engine.GeoPos("cities", "SPB");

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, keyval::error::ErrorCode::kKeyNotFound);
}

TEST_F(DBEngineTest, GeoAddWrongTypeReturnsError) {
    keyval::DBEngine engine;

    ASSERT_TRUE(engine.Set("cities", "not geo").has_value());

    auto result =
        engine.GeoAdd("cities", "Moscow", keyval::GeoPoint{55.7558, 37.6173});

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, keyval::error::ErrorCode::kWrongType);
}

TEST_F(DBEngineTest, GeoDistSamePointIsZero) {
    keyval::DBEngine engine;

    ASSERT_TRUE(
        engine.GeoAdd("cities", "Moscow", keyval::GeoPoint{55.7558, 37.6173})
            .has_value());

    auto result = engine.GeoDist("cities", "Moscow", "Moscow",
                                 keyval::GeoUnit::kKilometer);

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result.value(), 0.0, 1e-9);
}

TEST_F(DBEngineTest, GeoDistMoscowToSpbInKilometers) {
    keyval::DBEngine engine;

    ASSERT_TRUE(
        engine.GeoAdd("cities", "Moscow", keyval::GeoPoint{55.7558, 37.6173})
            .has_value());
    ASSERT_TRUE(
        engine.GeoAdd("cities", "SPB", keyval::GeoPoint{59.9343, 30.3351})
            .has_value());

    auto result =
        engine.GeoDist("cities", "Moscow", "SPB", keyval::GeoUnit::kKilometer);

    ASSERT_TRUE(result.has_value());

    EXPECT_NEAR(result.value(), 633.0, 10.0);
}

TEST_F(DBEngineTest, GeoDistSupportsMeters) {
    keyval::DBEngine engine;

    ASSERT_TRUE(
        engine.GeoAdd("cities", "Moscow", keyval::GeoPoint{55.7558, 37.6173})
            .has_value());
    ASSERT_TRUE(
        engine.GeoAdd("cities", "SPB", keyval::GeoPoint{59.9343, 30.3351})
            .has_value());

    auto result =
        engine.GeoDist("cities", "Moscow", "SPB", keyval::GeoUnit::kMeter);

    ASSERT_TRUE(result.has_value());

    EXPECT_NEAR(result.value(), 633000.0, 10000.0);
}

TEST_F(DBEngineTest, GeoSearchFindsPointsInsideRadius) {
    keyval::DBEngine engine;

    ASSERT_TRUE(
        engine.GeoAdd("cities", "Moscow", keyval::GeoPoint{55.7558, 37.6173})
            .has_value());
    ASSERT_TRUE(
        engine.GeoAdd("cities", "SPB", keyval::GeoPoint{59.9343, 30.3351})
            .has_value());

    auto result =
        engine.GeoSearch("cities", keyval::GeoPoint{55.7558, 37.6173}, 700.0,
                         keyval::GeoUnit::kKilometer, 10, true);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().size(), 2);
}

TEST_F(DBEngineTest, GeoSearchDoesNotReturnPointsOutsideRadius) {
    keyval::DBEngine engine;

    ASSERT_TRUE(
        engine.GeoAdd("cities", "Moscow", keyval::GeoPoint{55.7558, 37.6173})
            .has_value());
    ASSERT_TRUE(
        engine.GeoAdd("cities", "SPB", keyval::GeoPoint{59.9343, 30.3351})
            .has_value());

    auto result =
        engine.GeoSearch("cities", keyval::GeoPoint{55.7558, 37.6173}, 100.0,
                         keyval::GeoUnit::kKilometer, 10, true);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().size(), 1);
    EXPECT_DOUBLE_EQ(result.value()[0].point.latitude, 55.7558);
    EXPECT_DOUBLE_EQ(result.value()[0].point.longitude, 37.6173);
}

TEST_F(DBEngineTest, GeoSearchAscendingSortsByDistance) {
    keyval::DBEngine engine;

    ASSERT_TRUE(
        engine.GeoAdd("cities", "Moscow", keyval::GeoPoint{55.7558, 37.6173})
            .has_value());
    ASSERT_TRUE(
        engine.GeoAdd("cities", "SPB", keyval::GeoPoint{59.9343, 30.3351})
            .has_value());

    auto result = engine.GeoSearch("cities", keyval::GeoPoint{55.7558, 37.6173},
                                   700.0, keyval::GeoUnit::kKilometer, 2, true);

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result.value().size(), 2);

    EXPECT_DOUBLE_EQ(result.value()[0].point.latitude, 55.7558);
    EXPECT_DOUBLE_EQ(result.value()[0].point.longitude, 37.6173);
}

TEST_F(DBEngineTest, GeoSearchCountLimitsResult) {
    keyval::DBEngine engine;

    ASSERT_TRUE(
        engine.GeoAdd("cities", "Moscow", keyval::GeoPoint{55.7558, 37.6173})
            .has_value());
    ASSERT_TRUE(
        engine.GeoAdd("cities", "SPB", keyval::GeoPoint{59.9343, 30.3351})
            .has_value());

    auto result = engine.GeoSearch("cities", keyval::GeoPoint{55.7558, 37.6173},
                                   700.0, keyval::GeoUnit::kKilometer, 1, true);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().size(), 1);
}

TEST_F(DBEngineTest, GeoSearchStoreCreatesDestinationGeoIndex) {
    keyval::DBEngine engine;

    ASSERT_TRUE(
        engine.GeoAdd("cities", "Moscow", keyval::GeoPoint{55.7558, 37.6173})
            .has_value());
    ASSERT_TRUE(
        engine.GeoAdd("cities", "SPB", keyval::GeoPoint{59.9343, 30.3351})
            .has_value());

    auto result = engine.GeoSearchStore(
        "near", "cities", keyval::GeoPoint{55.7558, 37.6173}, 100.0,
        keyval::GeoUnit::kKilometer, 10, true);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().size(), 1);

    auto type = engine.Type("near");
    ASSERT_TRUE(type.has_value());
    EXPECT_EQ(type.value(), StorageType::kGeo);

    auto pos = engine.GeoPos("near", "Moscow");
    ASSERT_TRUE(pos.has_value());
    EXPECT_DOUBLE_EQ(pos.value().latitude, 55.7558);
    EXPECT_DOUBLE_EQ(pos.value().longitude, 37.6173);
}

TEST_F(DBEngineTest, GeoSearchStoreFailsWhenDestinationExists) {
    keyval::DBEngine engine;

    ASSERT_TRUE(
        engine.GeoAdd("cities", "Moscow", keyval::GeoPoint{55.7558, 37.6173})
            .has_value());
    ASSERT_TRUE(engine.Set("near", "already exists").has_value());

    auto result = engine.GeoSearchStore(
        "near", "cities", keyval::GeoPoint{55.7558, 37.6173}, 100.0,
        keyval::GeoUnit::kKilometer, 10, true);

    ASSERT_FALSE(result.has_value());
}
