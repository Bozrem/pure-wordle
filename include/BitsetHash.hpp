#pragma once

#include "Types.hpp"

struct BitsetHash {
    std::size_t operator()(const StateBitset& state) const noexcept {
        return std::hash<StateBitset>{}(state); // C++ Standard Library has this specialization!
    }
};
