#include <gtest/gtest.h>

#include "Glob/GlobMatcher.hpp"

using namespace keyval;

TEST(GlobMatcherTest, EmptyPatternMatchesEmptyText) {
    EXPECT_TRUE(GlobMatcher::Match("", ""));
}

TEST(GlobMatcherTest, EmptyPatternDoesNotMatchNonEmptyText) {
    EXPECT_FALSE(GlobMatcher::Match("", "abc"));
}

TEST(GlobMatcherTest, ExactTextMatches) {
    EXPECT_TRUE(GlobMatcher::Match("abc", "abc"));
}

TEST(GlobMatcherTest, DifferentTextDoesNotMatch) {
    EXPECT_FALSE(GlobMatcher::Match("abc", "abd"));
}

TEST(GlobMatcherTest, QuestionMatchesOneCharacter) {
    EXPECT_TRUE(GlobMatcher::Match("a?c", "abc"));
}

TEST(GlobMatcherTest, QuestionDoesNotMatchZeroCharacters) {
    EXPECT_FALSE(GlobMatcher::Match("a?c", "ac"));
}

TEST(GlobMatcherTest, QuestionDoesNotMatchTwoCharacters) {
    EXPECT_FALSE(GlobMatcher::Match("a?c", "abbc"));
}

TEST(GlobMatcherTest, StarMatchesEmptyText) {
    EXPECT_TRUE(GlobMatcher::Match("*", ""));
}

TEST(GlobMatcherTest, StarMatchesWholeText) {
    EXPECT_TRUE(GlobMatcher::Match("*", "abc"));
}

TEST(GlobMatcherTest, StarMatchesMiddlePart) {
    EXPECT_TRUE(GlobMatcher::Match("a*c", "abbbc"));
}

TEST(GlobMatcherTest, StarCanMatchZeroCharacters) {
    EXPECT_TRUE(GlobMatcher::Match("ab*cd", "abcd"));
}

TEST(GlobMatcherTest, MultipleStarsWorkLikeOneStar) {
    EXPECT_TRUE(GlobMatcher::Match("a**c", "abc"));
}

TEST(GlobMatcherTest, StarDoesNotHideWrongSuffix) {
    EXPECT_FALSE(GlobMatcher::Match("a*c", "abbbd"));
}

TEST(GlobMatcherTest, CharClassMatchesListedCharacter) {
    EXPECT_TRUE(GlobMatcher::Match("h[ae]llo", "hello"));
}

TEST(GlobMatcherTest, CharClassMatchesSecondListedCharacter) {
    EXPECT_TRUE(GlobMatcher::Match("h[ae]llo", "hallo"));
}

TEST(GlobMatcherTest, CharClassRejectsMissingCharacter) {
    EXPECT_FALSE(GlobMatcher::Match("h[ae]llo", "hillo"));
}

TEST(GlobMatcherTest, CharClassRangeMatchesCharacter) {
    EXPECT_TRUE(GlobMatcher::Match("key[0-9]", "key5"));
}

TEST(GlobMatcherTest, CharClassRangeRejectsCharacterOutsideRange) {
    EXPECT_FALSE(GlobMatcher::Match("key[0-9]", "keyx"));
}

TEST(GlobMatcherTest, NegatedCharClassMatchesDifferentCharacter) {
    EXPECT_TRUE(GlobMatcher::Match("h[^e]llo", "hallo"));
}

TEST(GlobMatcherTest, NegatedCharClassRejectsListedCharacter) {
    EXPECT_FALSE(GlobMatcher::Match("h[^e]llo", "hello"));
}

TEST(GlobMatcherTest, EscapedStarMatchesRealStar) {
    EXPECT_TRUE(GlobMatcher::Match("file\\*", "file*"));
}

TEST(GlobMatcherTest, EscapedStarDoesNotActLikeWildcard) {
    EXPECT_FALSE(GlobMatcher::Match("file\\*", "file123"));
}

TEST(GlobMatcherTest, PrefixStarMatchesSuffix) {
    EXPECT_TRUE(GlobMatcher::Match("*end", "the-end"));
}

TEST(GlobMatcherTest, PrefixStarRejectsWrongSuffix) {
    EXPECT_FALSE(GlobMatcher::Match("*end", "the-finish"));
}

TEST(GlobMatcherTest, SuffixStarMatchesPrefix) {
    EXPECT_TRUE(GlobMatcher::Match("start*", "start-here"));
}

TEST(GlobMatcherTest, SuffixStarRejectsWrongPrefix) {
    EXPECT_FALSE(GlobMatcher::Match("start*", "begin-start"));
}

TEST(GlobMatcherTest, StarBetweenWordsMatchesLongMiddle) {
    EXPECT_TRUE(GlobMatcher::Match("user:*:name", "user:12345:name"));
}

TEST(GlobMatcherTest, StarBetweenWordsRejectsWrongEnd) {
    EXPECT_FALSE(GlobMatcher::Match("user:*:name", "user:12345:age"));
}

TEST(GlobMatcherTest, TwoStarsMatchTwoParts) {
    EXPECT_TRUE(GlobMatcher::Match("a*b*c", "axbyc"));
}

TEST(GlobMatcherTest, TwoStarsCanMatchEmptyParts) {
    EXPECT_TRUE(GlobMatcher::Match("a*b*c", "abc"));
}

TEST(GlobMatcherTest, QuestionAtStartMatchesOneCharacter) {
    EXPECT_TRUE(GlobMatcher::Match("?bc", "abc"));
}

TEST(GlobMatcherTest, QuestionAtEndMatchesOneCharacter) {
    EXPECT_TRUE(GlobMatcher::Match("ab?", "abc"));
}

TEST(GlobMatcherTest, SeveralQuestionsMatchSameLengthText) {
    EXPECT_TRUE(GlobMatcher::Match("???", "abc"));
}

TEST(GlobMatcherTest, SeveralQuestionsRejectShortText) {
    EXPECT_FALSE(GlobMatcher::Match("???", "ab"));
}

TEST(GlobMatcherTest, LetterRangeMatchesLowercaseLetter) {
    EXPECT_TRUE(GlobMatcher::Match("[a-z]", "m"));
}

TEST(GlobMatcherTest, LetterRangeRejectsUppercaseLetter) {
    EXPECT_FALSE(GlobMatcher::Match("[a-z]", "M"));
}

TEST(GlobMatcherTest, NegatedRangeMatchesCharacterOutsideRange) {
    EXPECT_TRUE(GlobMatcher::Match("[^0-9]", "a"));
}

TEST(GlobMatcherTest, NegatedRangeRejectsCharacterInsideRange) {
    EXPECT_FALSE(GlobMatcher::Match("[^0-9]", "7"));
}

TEST(GlobMatcherTest, EscapedQuestionMatchesRealQuestion) {
    EXPECT_TRUE(GlobMatcher::Match("file\\?", "file?"));
}

TEST(GlobMatcherTest, EscapedQuestionDoesNotActLikeWildcard) {
    EXPECT_FALSE(GlobMatcher::Match("file\\?", "file1"));
}

TEST(GlobMatcherTest, UnclosedCharClassDoesNotMatch) {
    EXPECT_FALSE(GlobMatcher::Match("key[abc", "keya"));
}

TEST(GlobMatcherTest, EmptyCharClassDoesNotMatch) {
    EXPECT_FALSE(GlobMatcher::Match("key[]", "keya"));
}
