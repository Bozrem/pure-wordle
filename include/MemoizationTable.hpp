#pragma once

#include "Types.hpp"
#include "BitsetHash.hpp"

#include <unordered_map>
#include <shared_mutex>
#include <array>
#include <optional>

struct Entry {
    double expected_guesses;
    int best_guess_index;
};

class MemoizationTable {
private:
    // Sort of like lock striping, but avoiding the rehashing issues
    struct Bucket {
        std::unordered_map<StateBitset, Entry, BitsetHash> map;
        mutable std::shared_mutex lock;
    };

    static constexpr int NUM_BUCKETS = 256;
    std::array<Bucket, NUM_BUCKETS> buckets;

    int get_bucket_index(const StateBitset& state) const;
public:
    MemoizationTable();
    std::optional<Entry> get(const StateBitset& state) const;
    void insert(const StateBitset& state, double val, int guess_index);

    // Statistics
    size_t total_size() const;
};
