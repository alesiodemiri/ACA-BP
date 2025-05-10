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
        for (unsigned int i = 0; i < DEFAULT_SIZE; i += 64) {
            std::cout << i << "\t\t" << table[i].get_state() << std::endl;
        }
        std::cout << std::endl;
    }

    int get_bit_size() override {
        return DEFAULT_SIZE * FSM::get_bit_size();
    }
};

#endif
