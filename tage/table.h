#ifndef TABLE_H
#define TABLE_H

#include <array>
#include <bitset>
#include <cassert>
#include <iostream>

#include "2bitFSM.h"
#include "parameters.h"
#include "SaturationCounter.h"
#include "bits_reg.h"

struct TableEntry {
    bits_reg tag;
    FSM prediction;
    SaturationCounter<2> u = SaturationCounter<2>(0);

    /**
     * Initializes the entry according to the table that contains it, setting the correct tag len.
     * @param table_idx index of the table that contains this entry
     */
    explicit TableEntry(const uint8_t table_idx) : tag(TAG_LEN_S[table_idx]) {}
};

inline std::ostream& operator<<(std::ostream& os, const TableEntry& entry) {
    os << entry.tag << " "
       << entry.u.bits << " "
       << entry.prediction.get_state();
    return os;
}


class Table {
    const uint8_t table_idx;     // tagged table index [0 : N_TABLES - 1]
    const uint8_t TAG_LEN;
    const uint8_t IDX_LEN;

    const uint64_t WAY_SIZE;


    std::array<std::vector<TableEntry>, ASSOCIATIVITY> table;

    static uint8_t check_table_number(const uint8_t table_number) {
        assert(table_number < N_TABLES);

        return table_number;
    }

public:

    // counter for all the predictions coming from this table
    long total_predictions = 0;

    explicit Table(const uint8_t table_idx_) :
            table_idx(check_table_number(table_idx_)),
            TAG_LEN(TAG_LEN_S[table_idx]),
            IDX_LEN(IDX_LEN_S[table_idx]),
            WAY_SIZE(1 << IDX_LEN)
    {
        for (auto& way : table) {
            way.reserve(WAY_SIZE);
            while (way.size() < WAY_SIZE)
                way.emplace_back(table_idx);
        }
    }


    /**
     * @param hash hash of the branch performed with the PC and the correct portion of GHR.
     * @return a reference to the table entry, in case the tags match. Throws an exception otherwise.
     */
    TableEntry& get(const uint64_t hash) {
        const bits_reg index(IDX_LEN, hash);
        const bits_reg tag(TAG_LEN_S[table_idx], hash >> IDX_LEN);

        assert(index.to_ullong() < table[0].size());

        for (auto& way : table)
            if (way.at(index.to_ullong()).tag == tag) {
                return way.at(index.to_ullong());
            }

        // no entry matches the tags
        throw std::runtime_error("Tag mismatch");
    }

    /**
     * Allocates a new entry.
     * @param hash hash of the branch performed with the PC and the correct portion of GHR.
     * @returns true if the allocation succeeded, false otherwise
     */
    bool allocate(const uint64_t hash, const bool actual_outcome) {
        const bits_reg index(IDX_LEN, hash);
        const bits_reg tag(TAG_LEN_S[table_idx], hash >> IDX_LEN);

        assert(index.to_ullong() < table[0].size());

        // check if there is a useless entry and allocate it
        for (auto& way : table) {
            if (way.at(index.to_ullong()).u.get() == 0) {
                way.at(index.to_ullong()).tag.copy_from(tag);
                way.at(index.to_ullong()).prediction = FSM(actual_outcome);
                return true;
            }
        }
        return false;
    }

    void decrease_u(const uint64_t hash) {
        const bits_reg index(IDX_LEN, hash);
        //const bits_reg tag(hash >> IDX_LEN);

        assert(index.to_ullong() < table[0].size());

        // decrease u for all entries that share the same index
        for (auto& way : table)
            way.at(index.to_ullong()).u.decrease();
    }

    /**
     * performs graceful reset of the u counter for ALL the table entries.
     * @param hard_reset specifies the bit to reset is the most significant (true) or least significant (false).
     */
    void reset_u(const bool hard_reset) {
        for (auto& way : table)
            for (int i = 0; i < way.size(); i++)
                if(hard_reset)
                    way.at(i).u.bits.reset(1);    // set most significant bit to zero
                else
                    way.at(i).u.bits.reset(0);    // set least significant bit to zero
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
                if (way.at(i).tag.to_ullong() != 0)
                    filled_entries += 1;

        return filled_entries;
    }

    [[nodiscard]] int get_tot_entries() const {
        return WAY_SIZE * ASSOCIATIVITY;
    }

    friend std::ostream& operator<<(std::ostream& os, const Table& t);
};

inline std::ostream& operator<<(std::ostream& os, const Table& t) {
    os << "=== Table [" << static_cast<int>(t.table_idx) << "] ===\n";

    os << "Idx ";
    for (int way = 0; way < ASSOCIATIVITY; ++way) {
        os << "| Way " << way << "        ";
    }
    os << "\n";

    os << std::string(5 + ASSOCIATIVITY * 17, '-') << "\n";

    for (int idx = 0; idx < t.WAY_SIZE; ++idx) {
        bits_reg index_bits(t.IDX_LEN, idx);
        os << index_bits << " ";
        for (int way = 0; way < ASSOCIATIVITY; ++way) {
            os << "| " << t.table[way][idx] << " ";
        }
        os << "\n";
    }

    return os;
}

#endif
