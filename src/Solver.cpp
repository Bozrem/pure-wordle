#include "Solver.hpp"
#include "MemoizationTable.hpp"
#include "Wordle.hpp"
#include "Types.hpp"

Solver::Solver(const Wordle& g, MemoizationTable& cache) : game(g), cache(cache) {}

double Solver::evaluate_opener(int guess_index) {
    StateBitset root_state;
    root_state.set(); // Inits all to 1, all answers are still possible
    GuessBitset root_guesses;
    root_guesses.set();

    std::array<int, 243> pattern_count {0};

    for (int i = 0; i < NUM_ANSWERS; ++i)
        pattern_count[game.get_pattern_lookup(guess_index, i)]++;

    // Run those patterns to get cost
    double total_cost = 0.0;

    #pragma omp parallel for schedule(dynamic, 1) reduction(+:total_cost)
    for (int p = 0; p < 243; ++p) {
        if (pattern_count[p] == 0) continue; // Don't bother with ones that don't happen

        const StateBitset next_state = game.prune_state(root_state, guess_index, p);
        double next_state_cost = solve_state(next_state, root_guesses);
        total_cost += (next_state_cost * pattern_count[p]);
    }

    return 1 + (total_cost / NUM_ANSWERS);
}

double Solver::solve_state(const StateBitset& state, const GuessBitset& remaining_guesses) {
    if (auto entry = cache.get(state))
        return entry->expected_guesses; // DP Memoization
 
    int active_count = state.count();
    if (active_count == 1) return 1.0; // 1 guess left to make
    if (active_count == 0) return 0.0; // I don't THINK this should happen? TODO: Check

    GuessBitset useful_guesses = prune_actions(state, remaining_guesses);

    double best_cost = 1000;
    int best_guess_index = -1;

    for (int g = 0; g < NUM_GUESSES; ++g) {
        if (!useful_guesses.test(g)) continue; // Ignore prunded guesses
        std::array<int, 243> pattern_count = {0};

        for (int a = 0; a < NUM_ANSWERS; ++a) {
            if (!state.test(a)) continue; // Answer not possible in this state
            pattern_count[game.get_pattern_lookup(g, a)]++;
        }

        bool useless = false;
        for (int p = 0; p < 243; ++p) {
            if (pattern_count[p] == active_count) {
                useless = true;
                break;
            }
        }
        if (useless) continue;

        double total_cost = 0.0;

        for (int p = 0; p < 243; ++p) { // TODO: Make 243 a constexpr
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
    int active_count = state.count();
 
    // Only prune if the state is small enough to avoid signature overhead
    if (active_count > 64) { // TODO: add to config
        return curr_guesses;
    }

    GuessBitset useful_guesses; // Init to all 0

    // Build a list of active inds to avoid 2315 bitset pass each time in the inner
    std::vector<int> active_indices;
    active_indices.reserve(active_count);
    for(int i=0; i<NUM_ANSWERS; ++i) {
        if(state.test(i)) active_indices.push_back(i);
    }

    // Map signatures to the first guess that produced them
    std::unordered_map<std::string, int> seen_signatures;

    // Optimization: Reserve to prevent rehashes
    seen_signatures.reserve(active_count * 5); 

    for (int g = 0; g < NUM_GUESSES; ++g) {
        if (!curr_guesses.test(g)) continue;

        std::string signature = "";
        signature.reserve(active_count);

        bool all_same = true;
        Pattern first_p = game.get_pattern_lookup(g, active_indices[0]);

        // Build signature
        for (int a_idx : active_indices) {
            Pattern p = game.get_pattern_lookup(g, a_idx);
 
            if (p != first_p) all_same = false;

            // Append raw byte to signature
            signature += static_cast<char>(p);
        }

        // If this was the same pattern for all remaining answers, 
        //  it cannot distinguish and wouldn't eliminate any, so it's pruned
        if (all_same) continue;

        // If we have seen this exact signature before, this guess is mathematically identical
        // to a previous one, and only 1 is useful. Prune the duplicates
        if (seen_signatures.find(signature) == seen_signatures.end()) {
            seen_signatures[signature] = g;
            useful_guesses.set(g);
        }
    }

    return useful_guesses;
}
