#ifndef BASEPREDICTOR_H
#define BASEPREDICTOR_H

#include <array>
#include <cstdint>

#include "BasePredictorInterface.h"
#include "../2bitFSM.h"
#include "../parameters.h"

class BasePredictor : public BasePredictorInterface {

    std::array<FSM, DEFAULT_SIZE> table;

public:
    bool predict(const uint64_t seq_no, uint8_t /* piece */) override {
        return table[seq_no % DEFAULT_SIZE].get_prediction();
    }

    void update(const uint64_t seq_no, uint8_t /* piece */, const bool actual_outcome) override {
        table[seq_no % DEFAULT_SIZE].update_prediction(actual_outcome);
    }

    void print() override {
        std::cout << "=== Base Table ===" << std::endl;
        std::cout << std::string(5 + ASSOCIATIVITY * 17, '-') << "\n";

        for (int idx = 0; idx < DEFAULT_SIZE; ++idx) {
            std::bitset<K> index_bits(idx);
            std::cout << index_bits << " | " << table[idx].get_state() << "\n";
        }
        std::cout << std::endl;
    }

    int get_bit_size() override {
        return DEFAULT_SIZE * FSM::get_bit_size();
    }
};

#endif
