#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <cstdint>

constexpr uint8_t K = 12;                           // number of bits used to index the DEFAULT PREDICTOR
constexpr uint64_t DEFAULT_SIZE = (1 << K);         // number of entries in the DEFAULT PREDICTOR

constexpr uint8_t N_TABLES = 8;                     // number of tagged tables
constexpr uint16_t T_GHR_LEN[] = {1, 2, 5, 9, 15, 25, 44, 76, 130};  // ghr bits to include when hashing for each tagged table

constexpr uint16_t GHR_LEN = T_GHR_LEN[N_TABLES - 1];

constexpr uint8_t IDX_LEN = 12;                      // number of bits used to index the given tagged table
constexpr uint8_t TAG_LEN = 11;                      // number of bits used as tags in the given tagged table
constexpr uint64_t T_SIZE = (1 << IDX_LEN);         // number of entries in each tagged table

// TODO : make tables with different lengths
// TODO : tags with variable length

static_assert(K < 64, "K < 64");
static_assert(IDX_LEN < 64, "IDX_LEN < 64");
static_assert(IDX_LEN + TAG_LEN <= 64, "IDX_LEN + TAG_LEN <= 64");


// TODO : CHECK THIS NUMBER
constexpr uint64_t RESET_INTERVAL = (1 << 18);       // 200k, number of instructions to skip before performing graceful reset of u


#endif
