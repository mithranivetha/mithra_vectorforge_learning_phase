#include <gtest/gtest.h>

#include <stdexcept>
#include <vector>

#include "vf/distance.hpp"

using vf::CosineDistance;
using vf::DotProductDistance;
using vf::IDistance;
using vf::L2Distance;
using vf::makeDistance;

//L2
TEST(L2Distance, MatchesHandCalculatedValue) {
    L2Distance d;
    EXPECT_NEAR(d.compute({1, 2, 3}, {4, 6, 3}), 5.0f, 1e-5f);
}

TEST(L2Distance, IdenticalVectorsHaveZeroDistance) {
    L2Distance d;
    EXPECT_NEAR(d.compute({1, 2}, {1, 2}), 0.0f, 1e-6f);
}

TEST(L2Distance, IsSymmetric) {
    L2Distance d;
    EXPECT_FLOAT_EQ(d.compute({1, 2, 3}, {4, 6, 3}), d.compute({4, 6, 3}, {1, 2, 3}));
}

//Cosine
TEST(CosineDistance, PerpendicularVectorsGiveOne) {
    CosineDistance d;
    EXPECT_NEAR(d.compute({1, 0}, {0, 1}), 1.0f, 1e-5f);
}

TEST(CosineDistance, SameDirectionGivesZero) {
    CosineDistance d;
    EXPECT_NEAR(d.compute({1, 2}, {2, 4}), 0.0f, 1e-5f);
}

TEST(CosineDistance, OppositeDirectionGivesTwo) {
    CosineDistance d;
    EXPECT_NEAR(d.compute({1, 0}, {-1, 0}), 2.0f, 1e-5f);
}

TEST(CosineDistance, ZeroVectorThrows) {
    CosineDistance d;
    EXPECT_THROW(d.compute({0, 0, 0}, {1, 2, 3}), std::invalid_argument);
    EXPECT_THROW(d.compute({1, 2, 3}, {0, 0, 0}), std::invalid_argument);
}

//Dot product
TEST(DotProductDistance, IsNegatedDotProduct) {
    DotProductDistance d;
    EXPECT_NEAR(d.compute({1, 2, 3}, {4, 5, 6}), -32.0f, 1e-4f);
}

TEST(DotProductDistance, LargerDotProductMeansCloser) {
    DotProductDistance d;
    const std::vector<float> q = {1, 1};
    const float near_ = d.compute(q, {10, 10});  // dot = 20 -> -20
    const float far_ = d.compute(q, {1, 1});     // dot = 2  -> -2
    EXPECT_LT(near_, far_);
}

//Invalid input (all metrics)
TEST(Distances, MismatchedDimensionsThrow) {
    for (const char* name : {"l2", "cosine", "dot"}) {
        auto d = makeDistance(name);
        EXPECT_THROW(d->compute({1, 2, 3}, {1, 2}), std::invalid_argument) << name;
    }
}

TEST(Distances, EmptyVectorsThrow) {
    for (const char* name : {"l2", "cosine", "dot"}) {
        auto d = makeDistance(name);
        EXPECT_THROW(d->compute({}, {}), std::invalid_argument) << name;
        EXPECT_THROW(d->compute({}, {1}), std::invalid_argument) << name;
    }
}

//Factory
TEST(MakeDistance, CreatesCorrectTypeByName) {
    EXPECT_EQ(makeDistance("l2")->name(), "l2");
    EXPECT_EQ(makeDistance("cosine")->name(), "cosine");
    EXPECT_EQ(makeDistance("dot")->name(), "dot");
}

TEST(MakeDistance, IsCaseInsensitive) {
    EXPECT_EQ(makeDistance("L2")->name(), "l2");
    EXPECT_EQ(makeDistance("CoSiNe")->name(), "cosine");
}

TEST(MakeDistance, UnknownNameThrows) {
    EXPECT_THROW(makeDistance("manhattan"), std::invalid_argument);
    EXPECT_THROW(makeDistance(""), std::invalid_argument);
}

TEST(MakeDistance, WorksThroughBasePointer) {
    std::unique_ptr<IDistance> d = makeDistance("l2");
    EXPECT_NEAR(d->compute({0, 0}, {3, 4}), 5.0f, 1e-5f); 
}