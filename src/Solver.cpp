#include "Solver.hpp"
#include "Definitions.hpp"
#include "MemoizationTable.hpp"
#include "Statistics.hpp"
#include "Wordle.hpp"

#include <omp.h>

thread_local SolverStats t_stats;

Solver::Solver(const Config& c, const Wordle& g, MemoizationTable& m) : config(c), game(g), cache(m) {}

// -- Public --

SearchResult Solver::evaluate_guess(const StateBitset& state, int guess_ind, const GuessBitset& useful_guesses, int depth) {
    std::array<int, NUM_PATTERNS> pattern_count = {0};
    for_each_active_bit(state, [&](int answer_idx) {
        pattern_count[game.get_pattern_lookup(guess_ind, answer_idx)]++;
    });

    double total_cost = 0.0;
    int max_height = 0;

    for (int p = 0; p < NUM_PATTERNS; ++p) {
        if (pattern_count[p] == 0) continue;

        StateBitset new_state = game.prune_state(state, guess_ind, p);

        // Recursive
        SearchResult new_state_res = solve_state(new_state, useful_guesses, depth + 1);

        total_cost += new_state_res.expected_cost * pattern_count[p];
        max_height = std::max(max_height, new_state_res.max_height);
    }

    // Result for THIS guess, so the guess_ind is just this
    return { 1 + (total_cost / state.count()), guess_ind, max_height + 1 };
} // TODO: If I can make solve_state clean enough, it's probably cleanest to have it all in solve_state

// -- Private Primary --

SearchResult Solver::solve_state(const StateBitset& state, const GuessBitset& remaining_guesses, int depth) {
    t_stats.nodes_visited++;

    if (depth > 6) return { config.fail_cost, -1, 0 }; 
    int active_count = state.count();
    if (active_count == 1) return { 1.0, -1, 1 }; // -1 because no guess needed
    if (active_count == 0) return { 0.0, -1, 0 };
 
    // Cache Check
    if (auto entry = cache.get(state, depth)) {
        t_stats.cache_hits++;
        return *entry;
    }
    t_stats.cache_misses++;

    GuessBitset useful_guesses = prune_actions(state, remaining_guesses);

    // Track the best result found in this loop
    SearchResult best_res { 1000.0, -1, 1000 }; 

    std::vector<int> guess_inds;
    guess_inds.reserve(useful_guesses.count());
    for (int g = 0; g < NUM_GUESSES; ++g)
        if (useful_guesses.test(g)) guess_inds.push_back(g);

    for (int g : guess_inds) {
        // Recursive
        SearchResult res = evaluate_guess(state, g, useful_guesses, depth + 1);

        if (res.expected_cost < best_res.expected_cost)
            best_res = res;
    }

    // Cache save
    cache.insert(state, depth, best_res);

    return best_res;
}

// Simple FNV-1a style hash combiner
inline size_t combine_hash(size_t hash, uint8_t value) {
    return (hash ^ value) * 1099511628211ULL;
}

GuessBitset Solver::prune_actions(const StateBitset& state, const GuessBitset& curr_guesses) {
    t_stats.prune_function_calls++;

    static thread_local std::vector<int> active_indices;
    active_indices.clear();
    active_indices.reserve(state.count()); 

    for_each_active_bit(state, [&](int i) {
        active_indices.push_back(i);
    });


    struct Candidate {
        size_t signature_hash;
        int guess_index;
    };

    static thread_local std::vector<Candidate> candidates;
    candidates.clear();
    candidates.reserve(NUM_GUESSES);


    for (int g = 0; g < NUM_GUESSES; ++g) {
        if (!curr_guesses.test(g)) continue;
        t_stats.total_actions_checked++;

        size_t hash = 14695981039346656037ULL; // FNV offset basis
        bool all_same = true;

        // First pattern for uselessness check
        Pattern first_p = game.get_pattern_lookup(g, active_indices[0]);
        hash = combine_hash(hash, first_p);

        // Compute Signature
        for (size_t i = 1; i < active_indices.size(); ++i) {
            Pattern p = game.get_pattern_lookup(g, active_indices[i]);
 
            if (p != first_p) all_same = false;
            hash = combine_hash(hash, p);
        }

        if (all_same) {
            t_stats.useless_pruned++;
            continue; // Useless guess
        }
        // TODO: It's useless when all eliminate nothing. Is this equivelent?

        candidates.push_back({hash, g});
    }

    // Sort the hashes
    std::sort(candidates.begin(), candidates.end(), 
        [](const Candidate& a, const Candidate& b) {
            return a.signature_hash < b.signature_hash;
        });

    GuessBitset useful_guesses;
    if (candidates.empty()) return useful_guesses; // TODO: When could this happen?

    // Pass with handling
    useful_guesses.set(candidates[0].guess_index);

    for (size_t i = 1; i < candidates.size(); ++i) {
        // If hashes are different, it's definitely a different signature
        if (candidates[i].signature_hash != candidates[i-1].signature_hash) {
            useful_guesses.set(candidates[i].guess_index);
            continue;
        }

        // Collision Check
        bool is_duplicate = true;
        int g1 = candidates[i].guess_index;
        int g2 = candidates[i-1].guess_index;

        for (int answer_idx : active_indices) {
            if (game.get_pattern_lookup(g1, answer_idx) != game.get_pattern_lookup(g2, answer_idx)) {
                is_duplicate = false;
                break;
            }
        } // TODO: Explore tradeoff of keeping all in memory from the start and avoiding this

        if (is_duplicate)
            t_stats.duplicates_pruned++;
        else
            useful_guesses.set(candidates[i].guess_index);
    }

    t_stats.total_actions_kept += useful_guesses.count();

    return useful_guesses;
}
