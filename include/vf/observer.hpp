#pragma once

#include "vf/types.hpp"

namespace vf {

class ISearchObserver {
public:
    virtual ~ISearchObserver() = default;

    virtual void onVisit(VectorId id, float distance) = 0;
};

}  