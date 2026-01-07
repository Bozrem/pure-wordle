#include "MemoizationTable.hpp"

MemoizationTable::MemoizationTable(const Config& c) : config(c) {
    agnostic_map.reserve(config.agnostic_reserve);
    specific_map.reserve(config.specific_reserve);
}


std::optional<MemoizationTable::SearchResult> MemoizationTable::get(const StateBitset& state, int depth) {
    // Check Agnostic Table
    if (auto it = agnostic_map.find(state); it != agnostic_map.end()) {
        // We can only use this value if the solution tree fits within the remaining moves.
        if (depth + it->second.max_subtree_height <= 6) {
            //stats.cache_hits++;
            return SearchResult{it->second.expected_guesses,
                                it->second.best_guess_index,
                                it->second.max_subtree_height};
        }
    }

    // Check Specific Table (missing from agnostic or would be tainted)
    SpecificKey key{state, static_cast<uint8_t>(depth)};
    if (auto it = specific_map.find(key); it != specific_map.end()) {
        //stats.cache_hits++;
        return SearchResult{it->second.expected_guesses,
                            it->second.best_guess_index,
                            7 - depth};
    }

    //stats.cache_misses++;
    return std::nullopt;
}



void MemoizationTable::insert(const StateBitset& state, int depth, double val, int guess_index, int subtree_height) {
    // It didn't rely on the 1e9 cutoff value.
    bool is_clean_value = (depth + subtree_height <= 6);

    if (is_clean_value) {
        agnostic_map.try_emplace(state, AgnosticEntry{
            val,
            static_cast<int16_t>(guess_index),
            static_cast<uint8_t>(subtree_height)
        });
    } else {
        SpecificKey key{state, static_cast<uint8_t>(depth)};
        specific_map.try_emplace(key, SpecificEntry{
            val,
            static_cast<int16_t>(guess_index)
        });
    }
}
