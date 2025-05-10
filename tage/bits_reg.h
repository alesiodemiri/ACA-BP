#ifndef BITS_REG_H
#define BITS_REG_H

#include <bitset>
#include <iostream>
#include <string>

/**
 * @class bits_reg
 * @brief Simulates a variable length bitset using a fixed size bitset and a mask
 *
 * This class uses a fixed size std::bitset internally (64 bits by default)
 * but provides an interface that only exposes a specified number of bits.
 * Access to bits beyond the specified length is restricted.
 */
class bits_reg {

    static constexpr size_t MAX_BITS = 64;  // Maximum number of bits (fixed size)

    std::bitset<MAX_BITS> data;             // Internal storage
    const std::bitset<MAX_BITS> mask;       // Mask for the valid bits (now const)
    const size_t length;                    // Current effective length

    /**
     * @brief Creates a mask with n least significant bits set to 1
     * @param relevant_bits_n The number of bits to set
     * @return A bitset with the n least significant bits set to 1
     */
    static std::bitset<MAX_BITS> createMask(const size_t relevant_bits_n) {
        if (relevant_bits_n >= MAX_BITS) {
            return std::bitset<MAX_BITS>().set(); // All 1's
        }

        if (relevant_bits_n == 0) {
            return std::bitset<MAX_BITS>(); // All 0's
        }

        // Create a mask with the n least significant bits set to 1
        return std::bitset<MAX_BITS>((1ULL << relevant_bits_n) - 1);
    }

    /**
     * @brief Apply the mask to the data
     * This ensures that only the bits within the specified length are used
     */
    void applyMask() {
        data &= mask;
    }

public:

    /**
     * @brief Constructor with specified length
     * @param len The number of bits to use
     */
    explicit bits_reg(const size_t len) : mask(createMask(len)),  length(len > MAX_BITS ? MAX_BITS : len) {
        data.reset(); // Initialize all bits to 0
    }

    /**
     * @brief Constructor with specified length and initial value
     * @param len The number of bits to use
     * @param value The initial value for the bitset
     */
    explicit bits_reg(const size_t len, const uint64_t value) : mask(createMask(len)),  length(len > MAX_BITS ? MAX_BITS : len) {
        data = std::bitset<MAX_BITS>(value);
        applyMask(); // Ensure value conforms to the mask
    }

    /**
     * Copies content from the other object to this
     * @param other register to copy data from
     * @return reference to the modified object (this)
     */
    bits_reg& operator=(const bits_reg& other) {
        if (this != &other) {  // Check for self-assignment
            data = other.data;   // Copy the data bitset
            applyMask();
        }
        return *this;  // Allow chaining
    }


    /**
     * @brief Resets all bits to 0
     * @return Reference to this object
     */
    bits_reg& reset() {
        data.reset();
        return *this;
    }

    /**
     * @brief Gets the current effective length of the bitset
     * @return The number of bits being used
     */
    [[nodiscard]] size_t size() const {
        return length;
    }

    /**
     * @brief Converts the bitset to an unsigned long long
     * @return The bitset as an unsigned integer
     * @note Only bits within the variable length are included
     */
    [[nodiscard]] unsigned long long to_ullong() const {
        return (data & mask).to_ullong();
    }

    /**
     * @brief Equality operator
     * @param other The other VariableLengthBitset to compare with
     * @return true if both bitsets have the same length and bit values
     */
    bool operator==(const bits_reg& other) const {
        if (length != other.length) {
            return false;
        }
        return (data & mask) == (other.data & other.mask);
    }

    /**
     * @brief Inequality operator
     * @param other The other VariableLengthBitset to compare with
     * @return true if the bitsets differ in length or bit values
     */
    bool operator!=(const bits_reg& other) const {
        return !(*this == other);
    }

    /**
     * @brief Output stream operator
     * @param os The output stream
     * @param bs The VariableLengthBitset to output
     * @return The output stream
     */
    friend std::ostream& operator<<(std::ostream& os, const bits_reg& bs) {
        // We want to print in the standard order (MSB first)
        for (int i = bs.length - 1; i >= 0; --i) {
            os << (bs.data[i] ? '1' : '0');
        }
        return os;
    }
};

#endif //BITS_REG_H