#pragma once

#include <cstddef>
#include <unordered_map>
#include <vector>

#include "vf/types.hpp"

namespace vf {

class VectorStore {
public:
    using Storage = std::unordered_map<VectorId, std::vector<float>>;

    explicit VectorStore(std::size_t dimension);

    void upsert(VectorId id, std::vector<float> values);

    bool remove(VectorId id);

    const std::vector<float>* find(VectorId id) const;

    bool contains(VectorId id) const;
    std::size_t size() const noexcept;
    bool empty() const noexcept;
    std::size_t dimension() const noexcept;

    void clear() noexcept;

    const Storage& items() const noexcept;

private:
    std::size_t dimension_;
    Storage data_;
};

} 