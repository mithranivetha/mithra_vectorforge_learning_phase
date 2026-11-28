#pragma once

#include <cstddef>
#include <vector>

#include "vf/distance.hpp"
#include "vf/observer.hpp"
#include "vf/types.hpp"
#include "vf/vector_store.hpp"

namespace vf {

class ISearcher {
public:
    virtual ~ISearcher() = default;

    virtual std::vector<SearchResult> search(
        const VectorStore& store,
        const std::vector<float>& query,
        std::size_t k,
        const IDistance& metric,
        ISearchObserver* observer = nullptr) const = 0;
};

class BruteForceSearcher final : public ISearcher {
public:
    std::vector<SearchResult> search(
        const VectorStore& store,
        const std::vector<float>& query,
        std::size_t k,
        const IDistance& metric,
        ISearchObserver* observer = nullptr) const override;
};

} 