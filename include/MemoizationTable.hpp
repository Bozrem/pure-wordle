#pragma once
#include "Definitions.hpp"
// #include "Statistics.hpp"
#include <parallel_hashmap/phmap.h>
#include <optional>

/*
 * Outwardly, this behaves as a single table. Interally, it has two
 * The Agnostic table:
 *   - Can only work when the final value was untainted by going past 6 guesses
 *   - Stores the values and how far down the subtree it had to go to get them
 *   - In getting, if the get callee is far enough up that the subtree wouldn't reach 7
 *       then it's able to use that value, because it wouldn't ever hit fail
 * The Specific table:
 *   - Stores tainted final values (moved by the fail value)
 *   - Just stores the final value, but the key is state AND depth
 *   - This is the failover when it can't be added or found in the Agnostic table
 *   - Likely mostly stores the small states, since they're the most likely to use a fail value
 *
 * It has to be done like this for two reasons
 * 1. Cached values must know their depth
 *      If we tried without it, then the 1e9 value assigned to a bitset at depth 6 is the same as that one at depth 2
 * 2. That was way too slow, and resulted in a 50x cache miss rate
 *      This method gets it much closer to no-depth with around 5x the miss rate
 */

class MemoizationTable {
public:
    MemoizationTable(const Config& c);

    std::optional<SearchResult> get(const StateBitset& state, int depth);
    void insert(const StateBitset& state, int depth, const SearchResult& res);
private:
    // -- Agnostic Map Structs --

    struct AgnosticEntry {
        double expected_guesses;
        int16_t best_guess_index;
        uint8_t max_subtree_height;
    };

    using AgnosticKey = StateBitset;

    struct AgnosticHash {
        std::size_t operator()(const StateBitset& state) const noexcept {
            return std::hash<StateBitset>{}(state);
        }
    };


    // -- Specific Map Structs --

    struct SpecificEntry {
        double expected_guesses;
        int16_t best_guess_index;
    };

    struct SpecificKey {
        StateBitset state;
        uint8_t depth;

        bool operator==(const SpecificKey& other) const {
            return depth == other.depth && state == other.state;
        }
    };

    struct SpecificHash {
        size_t operator()(const SpecificKey& k) const noexcept {
            size_t h = AgnosticHash{}(k.state);
            h ^= static_cast<size_t>(k.depth) * 0x9e3779b97f4a7c15ull;
            return h;
        }
    };

    // -- Maps Setup --

    using AgnosticMap = phmap::parallel_flat_hash_map<
        AgnosticKey,
        AgnosticEntry,
        AgnosticHash,
        std::equal_to<StateBitset>,
        std::allocator<std::pair<const StateBitset, AgnosticEntry>>,
        9 // Means 2^9 strips
    >;

    using SpecificMap = phmap::parallel_flat_hash_map<
        SpecificKey,
        SpecificEntry,
        SpecificHash,
        std::equal_to<SpecificKey>,
        std::allocator<std::pair<const SpecificKey, SpecificEntry>>,
        9
    >;

    // -- Map Objects --

    const Config& config;
    AgnosticMap agnostic_map;
    SpecificMap specific_map;
};
