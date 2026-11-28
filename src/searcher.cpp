#include "vf/searcher.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace vf {

std::vector<SearchResult> BruteForceSearcher::search(
    const VectorStore& store,
    const std::vector<float>& query,
    std::size_t k,
    const IDistance& metric,
    ISearchObserver* observer) const {

    if (query.size() != store.dimension()) {
        throw std::invalid_argument(
            "search: query dimension mismatch (expected " +
            std::to_string(store.dimension()) + ", got " +
            std::to_string(query.size()) + ")");
    }

    if (k == 0 || store.empty()) {
        return {};
    }

    std::vector<SearchResult> results;
    results.reserve(store.size());

    for (const auto& [id, values] : store.items()) {
        const float d = metric.compute(query, values);
        if (observer != nullptr) {
            observer->onVisit(id, d);
        }
        results.push_back({id, d});
    }

    auto closer = [](const SearchResult& a, const SearchResult& b) {
        if (a.distance != b.distance) {
            return a.distance < b.distance;
        }
        return a.id < b.id;
    };

    const std::size_t n = std::min(k, results.size());
    std::partial_sort(results.begin(), results.begin() + n, results.end(), closer);
    results.resize(n);
    return results;
}

}  