#include <gtest/gtest.h>

#include <stdexcept>
#include <vector>

#include "vf/vector_store.hpp"

using vf::VectorStore;

TEST(VectorStore, ZeroDimensionIsRejected) {
    EXPECT_THROW(VectorStore(0), std::invalid_argument);
}

TEST(VectorStore, StartsEmpty) {
    VectorStore s(3);
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0u);
    EXPECT_EQ(s.dimension(), 3u);
}

TEST(VectorStore, InsertAndFind) {
    VectorStore s(3);
    s.upsert(1, {1.0f, 2.0f, 3.0f});
    ASSERT_TRUE(s.contains(1));
    ASSERT_NE(s.find(1), nullptr);
    EXPECT_EQ(*s.find(1), (std::vector<float>{1.0f, 2.0f, 3.0f}));
    EXPECT_EQ(s.size(), 1u);
}

TEST(VectorStore, FindUnknownIdReturnsNull) {
    VectorStore s(2);
    EXPECT_EQ(s.find(99), nullptr);
    EXPECT_FALSE(s.contains(99));
}

TEST(VectorStore, EmptyVectorIsRejected) {
    VectorStore s(3);
    EXPECT_THROW(s.upsert(1, {}), std::invalid_argument);
    EXPECT_TRUE(s.empty()); 
}

TEST(VectorStore, WrongDimensionIsRejected) {
    VectorStore s(3);
    EXPECT_THROW(s.upsert(1, {1.0f, 2.0f}), std::invalid_argument);
    EXPECT_THROW(s.upsert(1, {1.0f, 2.0f, 3.0f, 4.0f}), std::invalid_argument);
    EXPECT_TRUE(s.empty());
}

TEST(VectorStore, ExistingIdIsUpdatedNotDuplicated) {
    VectorStore s(2);
    s.upsert(7, {1.0f, 1.0f});
    s.upsert(7, {9.0f, 9.0f});
    EXPECT_EQ(s.size(), 1u);
    EXPECT_EQ(*s.find(7), (std::vector<float>{9.0f, 9.0f}));
}

TEST(VectorStore, FailedUpdateKeepsOldValue) {
    VectorStore s(2);
    s.upsert(7, {1.0f, 1.0f});
    EXPECT_THROW(s.upsert(7, {1.0f}), std::invalid_argument);
    EXPECT_EQ(*s.find(7), (std::vector<float>{1.0f, 1.0f}));
}

TEST(VectorStore, RemoveExistingReturnsTrue) {
    VectorStore s(2);
    s.upsert(1, {1.0f, 2.0f});
    EXPECT_TRUE(s.remove(1));
    EXPECT_FALSE(s.contains(1));
    EXPECT_TRUE(s.empty());
}

TEST(VectorStore, RemoveMissingReturnsFalseAndDoesNotThrow) {
    VectorStore s(2);
    EXPECT_NO_THROW({ EXPECT_FALSE(s.remove(123)); });
}

TEST(VectorStore, ClearRemovesEverything) {
    VectorStore s(2);
    s.upsert(1, {1.0f, 2.0f});
    s.upsert(2, {3.0f, 4.0f});
    s.clear();
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.dimension(), 2u); 
}