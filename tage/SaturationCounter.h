#ifndef SATURATIONCOUNTER_H
#define SATURATIONCOUNTER_H

#include <cstddef>
#include <cstdint>
#include <limits>

template <std::size_t n_bits>
class SaturationCounter {


public:
    std::bitset<n_bits> bits;

    // Constructor: initialize value to 0
    explicit SaturationCounter(uint64_t initial_value) {
        bits = std::bitset<n_bits>(initial_value);
    }

    // Increment the counter, but do not exceed MaxValue
    void increase() {
        if (bits.all()) return;

        bits = std::bitset<n_bits>(bits.to_ullong() + 1ULL);
    }

    // Decrement the counter, but do not go below 0
    void decrease() {
        if (bits.any())
            bits = std::bitset<n_bits>(bits.to_ullong() - 1ULL);
    }

    // Return the current value
    [[nodiscard]] uint64_t get() const {
        return bits.to_ullong();
    }
};

#endif
