#include <gtest/gtest.h>

#include <cmath>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "vf/distance.hpp"
#include "vf/observer.hpp"
#include "vf/searcher.hpp"
#include "vf/vector_store.hpp"

using vf::BruteForceSearcher;
using vf::CosineDistance;
using vf::DotProductDistance;
using vf::IDistance;
using vf::ISearcher;
using vf::ISearchObserver;
using vf::L2Distance;
using vf::SearchResult;
using vf::VectorId;
using vf::VectorStore;

namespace {

//n vectors on a line (inserted in reverse on purpose)
VectorStore makeLineStore(int n) {
    VectorStore s(2);
    for (int i = n - 1; i >= 0; --i) {
        s.upsert(i, {static_cast<float>(i), 0.0f});
    }
    return s;
}

std::vector<VectorId> idsOf(const std::vector<SearchResult>& results) {
    std::vector<VectorId> out;
    for (const auto& r : results) {
        out.push_back(r.id);
    }
    return out;
}

//test-only metric (looks at the first component)
class FirstComponentDistance final : public IDistance {
public:
    float compute(const std::vector<float>& a,
                  const std::vector<float>& b) const override {
        return std::fabs(a[0] - b[0]);
    }
    std::string name() const override { return "first-component"; }
};

//test-only observer (remembers every visit)
class RecordingObserver final : public ISearchObserver {
public:
    void onVisit(VectorId id, float distance) override {
        visits.emplace_back(id, distance);
    }
    std::vector<std::pair<VectorId, float>> visits;
};

} 

TEST(BruteForceSearch, Top3ReturnsClosestInOrder) {
    VectorStore store = makeLineStore(25);
    BruteForceSearcher searcher;
    L2Distance l2;

    auto r = searcher.search(store, {0.0f, 0.0f}, 3, l2);

    ASSERT_EQ(r.size(), 3u);
    EXPECT_EQ(idsOf(r), (std::vector<VectorId>{0, 1, 2}));
    EXPECT_NEAR(r[0].distance, 0.0f, 1e-5f);
    EXPECT_NEAR(r[1].distance, 1.0f, 1e-5f);
    EXPECT_NEAR(r[2].distance, 2.0f, 1e-5f);
}

TEST(BruteForceSearch, Top5ReturnsClosestInOrder) {
    VectorStore store = makeLineStore(25);
    BruteForceSearcher searcher;
    L2Distance l2;

    auto r = searcher.search(store, {0.0f, 0.0f}, 5, l2);

    EXPECT_EQ(idsOf(r), (std::vector<VectorId>{0, 1, 2, 3, 4}));
}

TEST(BruteForceSearch, OrderFollowsDistanceNotId) {
    VectorStore store = makeLineStore(25);
    BruteForceSearcher searcher;
    L2Distance l2;

    auto r = searcher.search(store, {10.2f, 0.0f}, 5, l2);

    EXPECT_EQ(idsOf(r), (std::vector<VectorId>{10, 11, 9, 12, 8}));
    ASSERT_EQ(r.size(), 5u);
    EXPECT_NEAR(r[0].distance, 0.2f, 1e-4f);
    EXPECT_NEAR(r[1].distance, 0.8f, 1e-4f);
    EXPECT_NEAR(r[2].distance, 1.2f, 1e-4f);
    EXPECT_NEAR(r[3].distance, 1.8f, 1e-4f);
    EXPECT_NEAR(r[4].distance, 2.2f, 1e-4f);
}

TEST(BruteForceSearch, DistancesAreNonDecreasing) {
    VectorStore store = makeLineStore(25);
    BruteForceSearcher searcher;
    L2Distance l2;

    auto r = searcher.search(store, {10.2f, 0.0f}, 10, l2);

    ASSERT_EQ(r.size(), 10u);
    for (std::size_t i = 1; i < r.size(); ++i) {
        EXPECT_LE(r[i - 1].distance, r[i].distance);
    }
}

TEST(BruteForceSearch, TiesAreBrokenBySmallerIdRegardlessOfInsertOrder) {
    VectorStore store(2);
    store.upsert(9, {1.0f, 1.0f});
    store.upsert(5, {1.0f, 1.0f});
    store.upsert(3, {1.0f, 1.0f});
    BruteForceSearcher searcher;
    L2Distance l2;

    EXPECT_EQ(idsOf(searcher.search(store, {0.0f, 0.0f}, 3, l2)),
              (std::vector<VectorId>{3, 5, 9}));
    EXPECT_EQ(idsOf(searcher.search(store, {0.0f, 0.0f}, 2, l2)),
              (std::vector<VectorId>{3, 5}));
}


TEST(BruteForceSearch, WorksWithCosine) {
    VectorStore store(2);
    store.upsert(1, {1.0f, 0.0f});
    store.upsert(2, {0.0f, 1.0f});
    store.upsert(3, {1.0f, 1.0f});
    store.upsert(4, {-1.0f, 0.0f});
    BruteForceSearcher searcher;
    CosineDistance cosine;

    auto r = searcher.search(store, {2.0f, 0.0f}, 4, cosine);

    EXPECT_EQ(idsOf(r), (std::vector<VectorId>{1, 3, 2, 4}));
    ASSERT_EQ(r.size(), 4u);
    EXPECT_NEAR(r[1].distance, 0.2929f, 1e-3f);
}

TEST(BruteForceSearch, WorksWithDotProduct) {
    VectorStore store(2);
    store.upsert(1, {1.0f, 0.0f});
    store.upsert(2, {2.0f, 0.0f});
    store.upsert(3, {3.0f, 0.0f});
    BruteForceSearcher searcher;
    DotProductDistance dot;

    auto r = searcher.search(store, {1.0f, 0.0f}, 3, dot);

    EXPECT_EQ(idsOf(r), (std::vector<VectorId>{3, 2, 1}));
}

TEST(BruteForceSearch, WorksWithACustomMetricItHasNeverSeen) {
    VectorStore store(2);
    store.upsert(1, {5.0f, 100.0f});
    store.upsert(2, {1.0f, -100.0f});
    store.upsert(3, {3.0f, 0.0f});
    BruteForceSearcher searcher;
    FirstComponentDistance custom;

    auto r = searcher.search(store, {0.0f, 0.0f}, 3, custom);

    EXPECT_EQ(idsOf(r), (std::vector<VectorId>{2, 3, 1}));
}

//edge cases
TEST(BruteForceSearch, EmptyStoreReturnsNothing) {
    VectorStore store(2);
    BruteForceSearcher searcher;
    L2Distance l2;

    EXPECT_TRUE(searcher.search(store, {0.0f, 0.0f}, 5, l2).empty());
}

TEST(BruteForceSearch, KZeroReturnsNothing) {
    VectorStore store = makeLineStore(5);
    BruteForceSearcher searcher;
    L2Distance l2;

    EXPECT_TRUE(searcher.search(store, {0.0f, 0.0f}, 0, l2).empty());
}

TEST(BruteForceSearch, KLargerThanStoreReturnsEverything) {
    VectorStore store = makeLineStore(5);
    BruteForceSearcher searcher;
    L2Distance l2;

    auto r = searcher.search(store, {0.0f, 0.0f}, 100, l2);

    EXPECT_EQ(idsOf(r), (std::vector<VectorId>{0, 1, 2, 3, 4}));
}

TEST(BruteForceSearch, KEqualToStoreSizeReturnsEverything) {
    VectorStore store = makeLineStore(5);
    BruteForceSearcher searcher;
    L2Distance l2;

    EXPECT_EQ(searcher.search(store, {0.0f, 0.0f}, 5, l2).size(), 5u);
}

TEST(BruteForceSearch, WrongQueryDimensionThrows) {
    VectorStore store = makeLineStore(5);
    BruteForceSearcher searcher;
    L2Distance l2;

    EXPECT_THROW(searcher.search(store, {1.0f, 2.0f, 3.0f}, 3, l2),
                 std::invalid_argument);
    EXPECT_THROW(searcher.search(store, std::vector<float>{}, 3, l2),
                 std::invalid_argument);
    // bad input is reported even when k == 0
    EXPECT_THROW(searcher.search(store, {1.0f}, 0, l2), std::invalid_argument);
}

TEST(BruteForceSearch, WrongQueryDimensionThrowsEvenOnEmptyStore) {
    VectorStore store(2);
    BruteForceSearcher searcher;
    L2Distance l2;

    EXPECT_THROW(searcher.search(store, {1.0f}, 3, l2), std::invalid_argument);
}


TEST(BruteForceSearch, WorksThroughTheInterface) {
    VectorStore store = makeLineStore(25);
    std::unique_ptr<ISearcher> searcher = std::make_unique<BruteForceSearcher>();
    L2Distance l2;

    auto r = searcher->search(store, {0.0f, 0.0f}, 3, l2);

    EXPECT_EQ(idsOf(r), (std::vector<VectorId>{0, 1, 2}));
}

TEST(BruteForceSearch, ObserverSeesEveryVectorOnce) {
    VectorStore store = makeLineStore(25);
    BruteForceSearcher searcher;
    L2Distance l2;
    RecordingObserver observer;

    searcher.search(store, {0.0f, 0.0f}, 3, l2, &observer);

    EXPECT_EQ(observer.visits.size(), 25u);
    std::set<VectorId> seen;
    for (const auto& [id, dist] : observer.visits) {
        seen.insert(id);
        EXPECT_NEAR(dist, static_cast<float>(id), 1e-5f);  // query is the origin
    }
    EXPECT_EQ(seen.size(), 25u);
}