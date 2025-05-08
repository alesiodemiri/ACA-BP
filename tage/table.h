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

    std::array<TableEntry, T_SIZE> table;

    public:

    TableEntry& get(const uint64_t hash) {
        const std::bitset<IDX_LEN> index(hash);
        const std::bitset<TAG_LEN> tag(hash >> IDX_LEN);

        assert(index.to_ullong() < table.size());

        if (table[index.to_ullong()].tag == tag) {
            return table[index.to_ullong()];
        } else {
            throw std::runtime_error("Tag mismatch");
        }
    }

    bool allocate(const uint64_t hash) {
        const std::bitset<IDX_LEN> index(hash);
        const std::bitset<TAG_LEN> tag(hash >> IDX_LEN);

        assert(index.to_ullong() < table.size());

        // check if there is a useless entry and allocate it
        if (table[index.to_ullong()].u.get() == 0) {
            table[index.to_ullong()].tag = tag;
            table[index.to_ullong()].prediction = FSM();
            return true;
        }
        return false;
    }

    void decrease_u(const uint64_t hash) {
        const std::bitset<IDX_LEN> index(hash);
        const std::bitset<TAG_LEN> tag(hash >> IDX_LEN);

        assert(index.to_ullong() < table.size());

        // decrease u
        table[index.to_ullong()].u.decrease();
    }

    void reset_u(const bool hard_reset) {
        for (int i = 0; i < T_SIZE; i++)
            if(hard_reset)
                table[i].u.bits.reset(1);    // set most significant bit to zero
            else
                table[i].u.bits.reset(0);    // set least significant bit to zero
    }

    void print() const {
        for (const auto& entry : table)
            std::cout << "TAG: "  << entry.tag << " Pred: " << entry.prediction.get_state() << " u: " << entry.u.bits << std::endl;
    }

};

#endif
