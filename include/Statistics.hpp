// NOTE: This is AI Generated

#pragma once

#include "RelaxedAtomic.hpp"

#include <iostream>
#include <iomanip>

struct SolverStats {
    // Cache
    RelaxedAtomic<long> cache_hits{0};
    RelaxedAtomic<long> cache_misses{0};

    // Nodes
    RelaxedAtomic<long> nodes_visited{0};

    // Pruning Efficiency
    RelaxedAtomic<long> prune_function_calls{0};
    RelaxedAtomic<long> total_actions_checked{0}; // Input to prune
    RelaxedAtomic<long> total_actions_kept{0};    // Output of prune
    RelaxedAtomic<long> useless_pruned{0};        // Removed for 0 info
    RelaxedAtomic<long> duplicates_pruned{0};     // Removed for signature match

    void print() {
        long total_reqs = cache_hits + cache_misses;
        double hit_rate = total_reqs > 0 ? (100.0 * cache_hits / total_reqs) : 0.0;
 
        long pruned_count = useless_pruned + duplicates_pruned;
        long total_prune_ops = total_actions_checked > 0 ? total_actions_checked.load() : 1;
        double prune_rate = 100.0 * pruned_count / total_prune_ops;

        std::cout << "\n=== SOLVER STATISTICS ===\n";
        std::cout << "Nodes Visited:   " << nodes_visited << "\n";
        std::cout << "Cache Hit Rate:  " << std::fixed << std::setprecision(2) << hit_rate << "% (" 
                  << cache_hits << " hits / " << cache_misses << " misses)\n";
        std::cout << "-------------------------\n";
        std::cout << "Pruning Calls:   " << prune_function_calls << "\n";
        std::cout << "Actions Checked: " << total_actions_checked << "\n";
        std::cout << "Actions Kept:    " << total_actions_kept << "\n";
        std::cout << "Prune Rate:      " << prune_rate << "% (Higher is better)\n";
        std::cout << "  - Useless:     " << useless_pruned << "\n";
        std::cout << "  - Duplicates:  " << duplicates_pruned << "\n";
        std::cout << "=========================\n";
    }
};

// Global singleton instance for simplicity in this project
//inline SolverStats stats;
