#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <cstdint>

constexpr uint8_t K = 16;                           // number of bits used to index the DEFAULT PREDICTOR
constexpr uint64_t DEFAULT_SIZE = (1 << K);         // number of entries in the DEFAULT PREDICTOR

constexpr uint8_t N_TABLES = 8;                     // number of tagged tables
constexpr uint16_t T_GHR_LEN[] = {5, 9, 15, 25, 44, 76, 130, 260};  // ghr bits to include when hashing for each tagged table

constexpr uint16_t GHR_LEN = T_GHR_LEN[N_TABLES - 1];


constexpr uint16_t TAG_LEN_S[] = {6, 6, 8, 11, 11, 11, 12, 12};  // number of bits used as tags in the given tagged table
constexpr uint8_t IDX_LEN = 11;                     // number of bits used to index a tagged table
constexpr uint64_t WAY_SIZE = (1 << IDX_LEN);

constexpr uint8_t ASSOCIATIVITY = 4;                 // number of entries sharing the same index in the table

// statically check that the number of bits used for tags and indexing is <= 64 in each table
constexpr bool validate_tag_lengths() {
    for (size_t i = 0; i < N_TABLES; ++i) {
        if (TAG_LEN_S[i] + IDX_LEN > 64) {
            return false;
        }
    }
    return true;
}

static_assert(K < 64, "K < 64");
static_assert(IDX_LEN < 64, "IDX_LEN < 64");
static_assert(validate_tag_lengths(), "Each TAG_LEN_S[i] + IDX_LEN must be <= 64");


// TODO : CHECK THIS NUMBER
constexpr uint64_t RESET_INTERVAL = (1 << 18);       // 200k, number of instructions to skip before performing graceful reset of u


#endif
