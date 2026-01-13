#include "MemoizationTable.hpp"
#include "Statistics.hpp"

MemoizationTable::MemoizationTable(const Config& c) : config(c) {
    agnostic_map.reserve(config.agnostic_reserve);
    specific_map.reserve(config.specific_reserve);
}


std::optional<SearchResult> MemoizationTable::get(const StateBitset& state, int depth) {
    // Check Agnostic Table
    if (auto it = agnostic_map.find(state); it != agnostic_map.end()) {
        if (depth + it->second.max_subtree_height <= 6) {
            return SearchResult{
                it->second.expected_guesses,
                it->second.best_guess_index,
                it->second.max_subtree_height
            };
        }
    }

    // Check Specific Table
    SpecificKey key{state, static_cast<uint8_t>(depth)};
    if (auto it = specific_map.find(key); it != specific_map.end()) {
        return SearchResult{
            it->second.expected_guesses,
            it->second.best_guess_index,
            7 - depth // If it wasn't a clean value, it's height must be at least to the bottom + 1
        };
    }

    return std::nullopt;
}

void MemoizationTable::insert(const StateBitset& state, int depth, const SearchResult& result) {
    bool is_clean_value = (depth + result.max_height <= 6);
    bool inserted = false;

    if (is_clean_value) {
        inserted = agnostic_map.try_emplace(state, AgnosticEntry{
            result.expected_cost,
            static_cast<int16_t>(result.best_guess_index),
            static_cast<uint8_t>(result.max_height)
        }).second;
    } else {
        SpecificKey key{state, static_cast<uint8_t>(depth)};
        inserted = specific_map.try_emplace(key, SpecificEntry{
            result.expected_cost,
            static_cast<int16_t>(result.best_guess_index)
        }).second;
    }

    t_stats.memo_inserts++;
    if (!inserted)
        t_stats.memo_collisions++;
}
