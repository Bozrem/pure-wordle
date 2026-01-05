#include "Solver.hpp"
#include "MemoizationTable.hpp"
#include "Statistics.hpp"
#include "Wordle.hpp"
#include "Types.hpp"

Solver::Solver(const Wordle& g, MemoizationTable& cache) : game(g), cache(cache) {}

double Solver::evaluate_opener(int guess_index) {
    StateBitset root_state;
    root_state.set(); // Inits all to 1, all answers are still possible
    GuessBitset root_guesses;
    root_guesses.set();

    std::array<int, NUM_PATTERNS> pattern_count {0};

    for (int i = 0; i < NUM_ANSWERS; ++i)
        pattern_count[game.get_pattern_lookup(guess_index, i)]++;

    // Run those patterns to get cost
    double total_cost = 0.0;

    #pragma omp parallel for schedule(dynamic, 1) reduction(+:total_cost)
    for (int p = 0; p < NUM_PATTERNS; ++p) {
        if (pattern_count[p] == 0) continue; // Don't bother with ones that don't happen

        const StateBitset next_state = game.prune_state(root_state, guess_index, p);
        double next_state_cost = solve_state(next_state, root_guesses);
        total_cost += (next_state_cost * pattern_count[p]);
    }

    return 1 + (total_cost / NUM_ANSWERS);
}

double Solver::solve_state(const StateBitset& state, const GuessBitset& remaining_guesses) {
    stats.nodes_visited++;

    int active_count = state.count();
    if (active_count == 1) return 1.0; // 1 guess left to make
    if (active_count == 0) return 0.0; // I don't THINK this should happen? TODO: Check

    if (auto entry = cache.get(state)) return entry->expected_guesses;

    GuessBitset useful_guesses = prune_actions(state, remaining_guesses);

    double best_cost = 1000;
    int best_guess_index = -1;

    for (int g = 0; g < NUM_GUESSES; ++g) {
        if (!useful_guesses.test(g)) continue; // Ignore prunded guesses
        std::array<int, NUM_PATTERNS> pattern_count = {0};

        for (int a = 0; a < NUM_ANSWERS; ++a) {
            if (!state.test(a)) continue; // Answer not possible in this state
            pattern_count[game.get_pattern_lookup(g, a)]++;
        }

        double total_cost = 0.0;

        for (int p = 0; p < NUM_PATTERNS; ++p) {
            if (pattern_count[p] == 0) continue;

            const StateBitset next_state = game.prune_state(state, g, p);
            double next_state_cost = solve_state(next_state, useful_guesses);
            total_cost += (next_state_cost * pattern_count[p]);

            // Optimization idea, check cost throughout this and fail fast if exceeds
            // TODO: Keep best as non-divided, since active count is the same. Avoids FP errors more too
        }

        double expected_value = 1.0 + (total_cost / active_count);

        if (expected_value < best_cost) {
            best_cost = expected_value;
            best_guess_index = g;
        }
    }

    cache.insert(state, best_cost, best_guess_index);

    return best_cost;
}

GuessBitset Solver::prune_actions(const StateBitset& state, const GuessBitset& curr_guesses) {
    stats.prune_function_calls++;
    int active_count = state.count();

    static thread_local std::vector<std::pair<std::string, int>> candidates;
    candidates.clear();
    candidates.reserve(NUM_GUESSES); // Reserve max once

    stats.total_actions_checked += curr_guesses.count();

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
            stats.useless_pruned++;
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
                stats.duplicates_pruned++;
            }
        }
    }

    stats.total_actions_kept += useful_guesses.count();
    return useful_guesses;
}
