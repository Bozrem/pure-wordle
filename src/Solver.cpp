#include "Solver.hpp"
#include "Definitions.hpp"
#include "MemoizationTable.hpp"
// #include "Statistics.hpp"
#include "Wordle.hpp"

#include <omp.h>

Solver::Solver(const Config& c, const Wordle& g, MemoizationTable& m) : config(c), game(g), cache(m) {}

// -- Public --

SearchResult Solver::evaluate_guess(const StateBitset& state, int guess_ind, const GuessBitset& useful_guesses, int depth) {
    std::array<int, NUM_PATTERNS> pattern_count = {0};
    for (int a = 0; a < NUM_ANSWERS; ++a) {
        if (!state.test(a)) continue; // Answer not possible in this state
        pattern_count[game.get_pattern_lookup(guess_ind, a)]++;
    } // TODO: Replace with a generalized pattern counter

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
    // stats.nodes_visited++;

    if (depth > 6) return { config.fail_cost, -1, 0 }; 
    int active_count = state.count();
    if (active_count == 1) return { 1.0, -1, 1 }; // -1 because no guess needed
    if (active_count == 0) return { 0.0, -1, 0 };
 
    // Cache Check
    if (auto entry = cache.get(state, depth)) return *entry;

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

GuessBitset Solver::prune_actions(const StateBitset& state, const GuessBitset& curr_guesses) {
    //stats.prune_function_calls++;
    int active_count = state.count();

    // TODO: Experiment with pruning threshold again

    static thread_local std::vector<std::pair<std::string, int>> candidates;
    candidates.clear();
    candidates.reserve(NUM_GUESSES); // Reserve max once

    //stats.total_actions_checked += curr_guesses.count();

    // Build a list of active inds to avoid 2315 bitset pass each time in the inner
    std::vector<int> active_indices;
    active_indices.reserve(active_count);
    for(int i=0; i<NUM_ANSWERS; ++i) {
        if(state.test(i)) active_indices.push_back(i);
    }

    for (int g = 0; g < NUM_GUESSES; ++g) {
        if (!curr_guesses.test(g)) continue;

        // Optimized Signature Generation
        // Instead of string += char, purely reserve space
        std::string signature; 
        signature.resize(active_count); 

        bool all_same = true;
        Pattern first_p = game.get_pattern_lookup(g, active_indices[0]);

        for (int i = 0; i < active_count; ++i) {
            Pattern p = game.get_pattern_lookup(g, active_indices[i]);
            if (p != first_p) all_same = false;
            signature[i] = static_cast<char>(p);
        }

        if (all_same) {
            //stats.useless_pruned++;
            continue;
        }

        // Store candidate
        candidates.push_back({std::move(signature), g});
    }

    // Sort to identify dupes
    std::sort(candidates.begin(), candidates.end(), 
        [](const auto& a, const auto& b) { return a.first < b.first; });

    GuessBitset useful_guesses;
    if (!candidates.empty()) {
        // Always keep first
        useful_guesses.set(candidates[0].second);

        for (size_t i = 1; i < candidates.size(); ++i) {
            if (candidates[i].first != candidates[i-1].first) {
                useful_guesses.set(candidates[i].second); // Only add if not same as last
            } else {
                //stats.duplicates_pruned++;
            }
        }
    }

    //stats.total_actions_kept += useful_guesses.count();
    return useful_guesses;
}
