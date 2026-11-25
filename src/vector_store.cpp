#include "vf/vector_store.hpp"

#include <stdexcept>
#include <string>
#include <utility>

namespace vf {

VectorStore::VectorStore(std::size_t dimension) : dimension_(dimension) {
    if (dimension_ == 0) {
        throw std::invalid_argument("VectorStore: dimension must be > 0");
    }
}

void VectorStore::upsert(VectorId id, std::vector<float> values) {
    if (values.empty()) {
        throw std::invalid_argument("VectorStore: vector must not be empty");
    }
    if (values.size() != dimension_) {
        throw std::invalid_argument(
            "VectorStore: dimension mismatch (expected " +
            std::to_string(dimension_) + ", got " +
            std::to_string(values.size()) + ")");
    }
    data_[id] = std::move(values);
}

bool VectorStore::remove(VectorId id) {
    return data_.erase(id) > 0;
}

const std::vector<float>* VectorStore::find(VectorId id) const {
    auto it = data_.find(id);
    return it == data_.end() ? nullptr : &it->second;
}

bool VectorStore::contains(VectorId id) const {
    return data_.count(id) > 0;
}

std::size_t VectorStore::size() const noexcept { return data_.size(); }
bool VectorStore::empty() const noexcept { return data_.empty(); }
std::size_t VectorStore::dimension() const noexcept { return dimension_; }

void VectorStore::clear() noexcept { data_.clear(); }

const VectorStore::Storage& VectorStore::items() const noexcept { return data_; }

}  