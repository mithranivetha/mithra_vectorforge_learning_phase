#pragma once

#include <memory>
#include <string>
#include <vector>

namespace vf {

class IDistance {
public:
    virtual ~IDistance() = default; 

    virtual float compute(const std::vector<float>& a,
                          const std::vector<float>& b) const = 0;

    virtual std::string name() const = 0;
};

class L2Distance final : public IDistance {
public:
    float compute(const std::vector<float>& a,
                  const std::vector<float>& b) const override;
    std::string name() const override { return "l2"; }
};

class CosineDistance final : public IDistance {
public:
    float compute(const std::vector<float>& a,
                  const std::vector<float>& b) const override;
    std::string name() const override { return "cosine"; }
};

class DotProductDistance final : public IDistance {
public:
    float compute(const std::vector<float>& a,
                  const std::vector<float>& b) const override;
    std::string name() const override { return "dot"; }
};

std::unique_ptr<IDistance> makeDistance(const std::string& name);

}  