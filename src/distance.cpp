#include "vf/distance.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>

namespace vf {

namespace {

void validateInputs(const std::vector<float>& a, const std::vector<float>& b) {
    if (a.empty() || b.empty()) {
        throw std::invalid_argument("distance: vectors must not be empty");
    }
    if (a.size() != b.size()) {
        throw std::invalid_argument(
            "distance: dimension mismatch (" + std::to_string(a.size()) +
            " vs " + std::to_string(b.size()) + ")");
    }
}

}  

float L2Distance::compute(const std::vector<float>& a,
                          const std::vector<float>& b) const {
    validateInputs(a, b);
    double sum = 0.0;
    for (std::size_t i = 0; i < a.size(); ++i) {
        const double d = static_cast<double>(a[i]) - static_cast<double>(b[i]);
        sum += d * d;
    }
    return static_cast<float>(std::sqrt(sum));
}

float CosineDistance::compute(const std::vector<float>& a,
                              const std::vector<float>& b) const {
    validateInputs(a, b);
    double dot = 0.0, normA = 0.0, normB = 0.0;
    for (std::size_t i = 0; i < a.size(); ++i) {
        const double x = a[i];
        const double y = b[i];
        dot += x * y;
        normA += x * x;
        normB += y * y;
    }
    if (normA == 0.0 || normB == 0.0) {
        throw std::invalid_argument(
            "cosine distance: undefined for zero-length (all-zero) vector");
    }
    const double cosSim = dot / (std::sqrt(normA) * std::sqrt(normB));
    return static_cast<float>(1.0 - cosSim);
}

float DotProductDistance::compute(const std::vector<float>& a,
                                  const std::vector<float>& b) const {
    validateInputs(a, b);
    double dot = 0.0;
    for (std::size_t i = 0; i < a.size(); ++i) {
        dot += static_cast<double>(a[i]) * static_cast<double>(b[i]);
    }
    return static_cast<float>(-dot);
}

std::unique_ptr<IDistance> makeDistance(const std::string& name) {
    std::string key = name;
    std::transform(key.begin(), key.end(), key.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (key == "l2")     return std::make_unique<L2Distance>();
    if (key == "cosine") return std::make_unique<CosineDistance>();
    if (key == "dot")    return std::make_unique<DotProductDistance>();

    throw std::invalid_argument("unknown distance metric: '" + name +
                                "' (expected l2, cosine or dot)");
}

}  