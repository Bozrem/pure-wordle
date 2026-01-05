#pragma once
#include "Types.hpp"
#include "BitsetHash.hpp"
#include "Statistics.hpp"
#include <parallel_hashmap/phmap.h>
#include <optional>

struct Entry {
    double expected_guesses;
    int best_guess_index;
};

class MemoizationTable {
private:
    using MapType = phmap::parallel_flat_hash_map<
        StateBitset, 
        Entry, 
        BitsetHash,
        std::equal_to<StateBitset>,
        std::allocator<std::pair<const StateBitset, Entry>>,
        9 // 2^9 = 512 submaps
    >;

    MapType map;

public:
    MemoizationTable() {
        map.reserve(1000000); 
    }

    std::optional<Entry> get(const StateBitset& state) {
        std::optional<Entry> result = std::nullopt;
 
        // if_contains locks the specific shard for the duration of the lambda
        bool found = map.if_contains(state, [&](const auto& pair) {
            result = pair.second;
        });

        if (found) {
            stats.cache_hits++;
            return result;
        }

        stats.cache_misses++;
        return std::nullopt;
    }

    void insert(const StateBitset& state, double val, int guess_index) {
        map.try_emplace(state, Entry{val, guess_index});
    }

    size_t total_size() const {
        return map.size();
    }
};
