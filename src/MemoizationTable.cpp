#include "MemoizationTable.hpp"

int MemoizationTable::get_bucket_index(const StateBitset& state) const { // TODO: does this need explicit inline?
        return std::hash<StateBitset>(state) % NUM_BUCKETS;
}


std::optional<Entry> MemoizationTable::get(const StateBitset& state) const {
    const Bucket bucket = buckets[get_bucket_index(state)];
    std::shared_lock lock(bucket.lock);
 
    auto item = bucket.map.find(state);
    if (item != bucket.map.end())
        return item->second; // TODO: See if the fancy pair notation works here

    return std::nullopt;
}

void MemoizationTable::insert(const StateBitset& state, double val, int guess_index) {
    Bucket bucket = buckets[get_bucket_index(state)];

    std::unique_lock lock(bucket.lock);

    bucket.map[state] = {val, guess_index};
}

size_t MemoizationTable::total_size() const {
    size_t total = 0;
    for (const Bucket& bucket : buckets) {
        std::shared_lock lock(bucket.lock); // Probably not needed, but doesn't hurt
        total += bucket.map.size();
    }
    return total;
}
