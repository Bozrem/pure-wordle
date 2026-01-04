#include "Solver.hpp"
#include "MemoizationTable.hpp"
#include "Wordle.hpp"
#include "Types.hpp"

Solver::Solver(const Wordle& g, MemoizationTable& cache) : game(g), cache(cache) {}

double Solver::evaluate_opener(int guess_index) {
    StateBitset root_state;
    root_state.set(); // Inits all to 1, all answers are still possible

    std::array<int, 243> pattern_count;

    for (int i = 0; i < NUM_ANSWERS; ++i)
        pattern_count[game.get_pattern_lookup(guess_index, i)]++;

    // Run those patterns to get cost
    double total_cost = 0.0;

    #pragma omp parallel for schedule(dynamic, 1) reduction(+:total_cost)
    for (int p = 0; p < 243; ++p) {
        if (pattern_count[p] == 0) continue; // Don't bother with ones that don't happen

        const StateBitset next_state = game.prune_state(root_state, guess_index, p);
        double next_state_cost = solve_state(next_state);
        total_cost += (next_state_cost * pattern_count[p]);
    }

    return 1 + (total_cost / NUM_ANSWERS);
}

double Solver::solve_state(const StateBitset& state) {
    if (auto entry = cache.get(state))
        return entry->expected_guesses; // DP Memoization
 
    int active_count = state.count();
    if (active_count == 1) return 1.0; // 1 guess left to make
    // Shouldn't we also have a 0 check for if we happen to guess correctly? Or is that not possible with the pattern-based model?

    double best_cost = 1000;
    int best_guess_index = -1;

    for (int g = 0; g < NUM_GUESSES; ++g) { // TODO: Action pruning
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

            const StateBitset next_state = game.prune_state(state, g, p); // Not possible for next_state == current_state unless all in the same bucket right?
            double next_state_cost = solve_state(next_state);
            total_cost += (next_state_cost * pattern_count[p]);

            // Optimization idea, check cost throughout this and just stop if it exceeds
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
