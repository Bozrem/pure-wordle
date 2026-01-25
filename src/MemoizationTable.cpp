#include "MemoizationTable.hpp"
#include "Statistics.hpp"

MemoizationTable::MemoizationTable(const Config& c) : config(c) {
    agnostic_map.reserve(config.agnostic_reserve);
    specific_map.reserve(config.specific_reserve);
}


std::optional<SearchResult> MemoizationTable::get(const StateBitset& state, int depth) {
    std::optional<SearchResult> result = std::nullopt;

    // Check Agnostic Table
    agnostic_map.if_contains(state, [&](const auto& kv) {
        const AgnosticEntry& entry = kv.second;

        if (depth + entry.max_subtree_height <= 6) {
            result = SearchResult{
                entry.expected_guesses,
                entry.best_guess_index,
                entry.max_subtree_height
            };
        }
    });

    if (result) return result;

    // Check Specific Table
    SpecificKey key{state, static_cast<uint8_t>(depth)};

    specific_map.if_contains(key, [&](const auto& kv) {
        const SpecificEntry& entry = kv.second;

        result = SearchResult{
            entry.expected_guesses,
            entry.best_guess_index,
            7 - depth 
        };
    });

    return result;
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
