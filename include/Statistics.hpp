// NOTE: This is AI Generated

#pragma once
#include <iostream>
#include <iomanip>

// Use plain integers for maximum speed (no atomics needed for TLS)
struct SolverStats {
    long cache_hits = 0;
    long cache_misses = 0;
    long nodes_visited = 0;
    long prune_function_calls = 0;
    long total_actions_checked = 0;
    long total_actions_kept = 0;
    long useless_pruned = 0;
    long duplicates_pruned = 0;
    long memo_inserts = 0;
    long memo_collisions = 0; // Duplicated work

    // Helper to merge another thread's stats into this one
    void operator+=(const SolverStats& other) {
        cache_hits += other.cache_hits;
        cache_misses += other.cache_misses;
        nodes_visited += other.nodes_visited;
        prune_function_calls += other.prune_function_calls;
        total_actions_checked += other.total_actions_checked;
        total_actions_kept += other.total_actions_kept;
        useless_pruned += other.useless_pruned;
        duplicates_pruned += other.duplicates_pruned;
        memo_inserts += other.memo_inserts;
        memo_collisions += other.memo_collisions;
    }

    void print() {
        long total_reqs = cache_hits + cache_misses;
        double hit_rate = total_reqs > 0 ? (100.0 * cache_hits / total_reqs) : 0.0;
        long pruned_count = useless_pruned + duplicates_pruned;
        long total_prune_ops = total_actions_checked > 0 ? total_actions_checked : 1;
        double prune_rate = 100.0 * pruned_count / total_prune_ops;
        double collision_rate = memo_inserts > 0 ? (100.0 * memo_collisions / memo_inserts) : 0.0;

        std::cout << "\n=== SOLVER STATISTICS ===\n";
        std::cout << "Nodes Visited:   " << nodes_visited << "\n";
        std::cout << "Cache Hit Rate:  " << std::fixed << std::setprecision(2) << hit_rate << "% (" 
                  << cache_hits << " hits / " << cache_misses << " misses)\n";
        std::cout << "-------------------------\n";
        std::cout << "Memoization:\n";
        std::cout << "  - Inserts:     " << memo_inserts << "\n";
        std::cout << "  - Collisions:  " << memo_collisions << "\n";
        std::cout << "  - Redundancy:  " << collision_rate << "% (Lower is better)\n";
        std::cout << "-------------------------\n";
        std::cout << "Pruning Calls:   " << prune_function_calls << "\n";
        std::cout << "Prune Rate:      " << prune_rate << "%\n";
        std::cout << "=========================\n";    }
};

// Declare the thread-local instance (each thread gets its own)
extern thread_local SolverStats t_stats;

// Declare the global accumulator (for the final sum)
extern SolverStats g_stats;
