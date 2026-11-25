#pragma once

#include <cstdint>

namespace vf {

using VectorId = std::int64_t;

struct SearchResult {
    VectorId id;
    float distance;
};

} 