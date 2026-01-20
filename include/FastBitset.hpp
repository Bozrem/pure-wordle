#pragma once

#include <cstdint>
#include <cstring> // For memset
#include <functional> // For the hash extension


// The template makes it so each size that I use can be compiled/optimized separately, but written once
template <int N>
struct alignas(32) FastBitset {
    static constexpr int NUM_WORDS = (N + 63) / 64; // Doing 64 bit words

    uint64_t words[NUM_WORDS]; // Primary data structure

    FastBitset() {
        reset();
    }


    void reset() {
        std::memset(words, 0x0, sizeof(words));
    }

    void reset(int pos) {
        words[pos / 64] &= ~(1ULL << (pos % 64));
        // The 1ULL sets a single bit at the bottom
        // The pos % 64 identifies the bit within the word to be set
        // The ~ flips the 1 to a 0
        // The &= does a bitwise and, but with all 1s but pos, the only spot that can be effected is pos, which is automatically 0
    }

    void set() {
        std::memset(words, 0xFF, sizeof(words));
    }

    void set(int pos) {
        words[pos / 64] |= (1ULL << (pos % 64));
        // Pretty much the same thing as reset, but does a 1 and an or
    }

    bool test(int pos) const {
        return (words[pos / 64] & (1ULL << (pos % 64))) != 0;
        // 1ULL portion masks everything but that
        // Words get the word, if that one was 1 it remains 1 and thus is > 0
    }

    int count() const {
        int c = 0;
        for (int w = 0; w < NUM_WORDS; ++w)
            c += __builtin_popcountll(words[w]);
        return c;
    }

    bool any() const {
        for (int w = 0; w < NUM_WORDS; ++w)
            if (words[w] != 0) return true;
        return false;
    }

    bool operator==(const FastBitset<N>& other) const {
        for (int w = 0; w < NUM_WORDS; ++w)
            if (words[w] != other.words[w]) return false;
        return true;
    }

    bool operator!=(const FastBitset<N>& other) const {
        return !(*this == other); // Just reuse, compiler probably inlines
    }


    // Solver loops over bitsets often, but they're sparse and probably destory branch prediction
    // This is a custom iterator that uses instrinsics to iterate WAY faster
    struct BitIterator {
        const FastBitset* parent;
        int word_index;
        uint64_t current_word_bits; // Save it for each word

        BitIterator(const FastBitset* p, int index) : parent(p), word_index(index), current_word_bits(0) {
            if (word_index < NUM_WORDS) {
                current_word_bits = parent->words[word_index];
                if (current_word_bits == 0) advance_word();
            }
        }

        // Moves to the next word with some set bits
        void advance_word() {
            word_index++;
            while (word_index < NUM_WORDS) {
                current_word_bits = parent->words[word_index];
                if (current_word_bits != 0) return;
                word_index++; // Goes until it hits
            }
        }

        bool operator!=(const BitIterator& other) const {
            return word_index != other.word_index || current_word_bits != other.current_word_bits;
        }

        int operator*() const {
            return word_index * 64 + __builtin_ctzll(current_word_bits);
            // Bits per word * the position of the next bit
        }

        BitIterator& operator++() {
            current_word_bits &= (current_word_bits - 1);
            // The -1 will take away the lowest one without needing to know what it was

            if (current_word_bits == 0) advance_word();

            return *this;
        }
    };

    BitIterator begin() const {
        return BitIterator(this, 0);
    }

    BitIterator end() const {
        return BitIterator(this, NUM_WORDS);
    } // If past the last word index then done
};


// Since this gets used as map keys, it needs to be hashable
namespace std {
    template <int N>
    struct hash<FastBitset<N>> {
        size_t operator()(const FastBitset<N>& bitset) const {
            // This is FNV-1a. To me it's black magic but it works
            size_t hash = 14695981039346656037ULL; // These are just the wikipedia values
            for (int w = 0; w < bitset.NUM_WORDS; ++w) {
                hash ^= bitset.words[w];
                hash *= 1099511628211ULL;
            }
            return hash;
        }
    };
}
