#ifndef TABLE_H
#define TABLE_H

#include <array>
#include <bitset>
#include <cassert>
#include <iostream>

#include "2bitFSM.h"
#include "parameters.h"
#include "SaturationCounter.h"

struct TableEntry {
    std::bitset<TAG_LEN> tag;
    FSM prediction;
    SaturationCounter<2> u = SaturationCounter<2>(0);
};

class Table {

    std::array<std::array<TableEntry, WAY_SIZE>, ASSOCIATIVITY> table;

public:

    // counter for all the predictions coming from this table
    long total_predictions = 0;


    /**
     * @param hash hash of the branch performed with the PC and the correct portion of GHR.
     * @return a reference to the table entry, in case the tags match. Throws an exception otherwise.
     */
    TableEntry& get(const uint64_t hash) {
        const std::bitset<IDX_LEN> index(hash);
        const std::bitset<TAG_LEN> tag(hash >> IDX_LEN);

        assert(index.to_ullong() < table[0].size());

        for (auto& way : table)
            if (way[index.to_ullong()].tag == tag) {
                return way[index.to_ullong()];
            }

        // no entry matches the tags
        throw std::runtime_error("Tag mismatch");
    }

    /**
     * Allocates a new entry. Assumes that the spot is free in the table.
     * @param hash hash of the branch performed with the PC and the correct portion of GHR.
     */
    bool allocate(const uint64_t hash) {
        const std::bitset<IDX_LEN> index(hash);
        const std::bitset<TAG_LEN> tag(hash >> IDX_LEN);

        assert(index.to_ullong() < table[0].size());

        // check if there is a useless entry and allocate it
        for (auto& way : table)
            if (way[index.to_ullong()].u.get() == 0) {
                way[index.to_ullong()].tag = tag;
                way[index.to_ullong()].prediction = FSM();
                return true;
            }
        return false;
    }

    void decrease_u(const uint64_t hash) {
        const std::bitset<IDX_LEN> index(hash);
        const std::bitset<TAG_LEN> tag(hash >> IDX_LEN);

        assert(index.to_ullong() < table.size());

        // decrease u for all entries that share the same index
        for (auto& way : table)
            way[index.to_ullong()].u.decrease();
    }

    /**
     * performs graceful reset of the u counter for ALL the table entries.
     * @param hard_reset specifies the bit to reset is the most significant (true) or least significant (false).
     */
    void reset_u(const bool hard_reset) {
        for (auto& way : table)
            for (int i = 0; i < way.size(); i++)
                if(hard_reset)
                    way[i].u.bits.reset(1);    // set most significant bit to zero
                else
                    way[i].u.bits.reset(0);    // set least significant bit to zero
    }

    /**
     * @return the size of the table in BITS, as if it was implemented in HW.
     */
    [[nodiscard]] int get_bit_size() const {
        return ASSOCIATIVITY * WAY_SIZE * (TAG_LEN + FSM::get_bit_size() + 2);
    }

    /**
     * @return the number of entries that have a tag different from 0.
     */
    [[nodiscard]] int get_occupation() const {
        int filled_entries = 0;
        for (auto& way : table)
            for (int i = 0; i < WAY_SIZE; i++)
                if (way[i].tag.to_ullong() != 0)
                    filled_entries += 1;

        return filled_entries;
    }
};

#endif
