// NOTE: This is AI generated

#include <atomic>

template <typename T>
struct RelaxedAtomic {
    std::atomic<T> val;

    RelaxedAtomic(T v = 0) : val(v) {}

    // Pre-increment returns what it now is
    T operator++() {
        return val.fetch_add(1, std::memory_order_relaxed) + 1;
    }

    // Post-increment returns what it was
    T operator++(int) {
        return val.fetch_add(1, std::memory_order_relaxed);
    }

    T operator+=(T arg) {
        return val.fetch_add(arg, std::memory_order_relaxed) + arg;
    }

    // Implicit conversion to T (allows: long x = stats.hits)
    operator T() const {
        return val.load(std::memory_order_relaxed);
    }
 
    // Explicit load if needed
    T load() const {
        return val.load(std::memory_order_relaxed);
    }
};
