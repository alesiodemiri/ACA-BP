#pragma once

#include <cstdint>

// Abstract base class for predictors
class BasePredictorInterface {
public:
    virtual ~BasePredictorInterface() = default;

    virtual bool predict(uint64_t seq_no, uint8_t piece) = 0;

    virtual void update(uint64_t seq_no, uint8_t piece, bool actual_outcome) = 0;

    virtual void print() = 0;
};