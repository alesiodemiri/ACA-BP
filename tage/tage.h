#ifndef TAGE_H
#define TAGE_H

#include <ostream>
#include <cstdlib>
#include <ctime>

#include "parameters.h"
#include "table.h"
#include "BasePredictor/BasePredictor.h"
#include "BasePredictor/BasePredictorInterface.h"

static constexpr uint8_t DEFAULT_PROVIDER = -1;

// this struct does not count towards the storage budget, it only records the prediction information provided by
// the TAGE predictor
struct Prediction {

    uint8_t provider_idx = DEFAULT_PROVIDER;
    bool provider_pred = false;
    bool provider_is_weak = false;

    uint8_t alt_idx = DEFAULT_PROVIDER;
    bool alt_pred = false;

    friend std::ostream & operator<<(std::ostream &os, const Prediction &obj) {
        if (obj.provider_idx == DEFAULT_PROVIDER) {
            return os;
        }

        return os
               << "provider_idx: " << static_cast<int>(obj.provider_idx)
               << " provider_pred: " << obj.provider_pred
               << " alt_idx: " << static_cast<int>(obj.alt_idx)
               << " alt_pred: " << obj.alt_pred << std::endl;
    }
};

class TAGE {

    // global history register
    std::bitset<GHR_LEN> GHR;

    // tagged tables
    std::vector<Table> tables;

    // base predictor
    std::unique_ptr<BasePredictorInterface> base_predictor = std::make_unique<BasePredictor>();

    // branches counter
    uint64_t num_branches = 0;
    bool hard_reset = true;

    // this function performs a hash
    template <size_t N>
    uint64_t hash(const uint64_t input, std::bitset<N> bits, const size_t k) {
        if (k == 0 || k > N) {
            std::cerr << "k: " << k << " N: " << N << std::endl;
            throw std::invalid_argument("k must be in the range 1..N");
        }

        // Step 1: Extract the last k bits
        std::bitset<N> repeated_bits;
        for (size_t i = 0; i < k; ++i) {
            repeated_bits[i] = bits[N - k + i];
        }

        // Step 2: Repeat those k bits to fill the bitset
        std::bitset<N> filled;
        for (size_t i = 0; i < N; ++i) {
            filled[i] = repeated_bits[i % k];
        }

        // Step 3: Hashing — process bitset in 64-bit chunks
        uint64_t hash = input;
        for (size_t i = 0; i < N; i += 64) {
            uint64_t chunk = 0;
            for (size_t j = 0; j < 64 && (i + j) < N; ++j) {
                if (filled[i + j]) {
                    chunk |= (uint64_t(1) << j);
                }
            }

            // Mix with golden ratio constant
            hash ^= chunk + 0x9e3779b97f4a7c15 + (hash << 6) + (hash >> 2);
        }

        /*
        std::cout << "PC  : " << std::bitset<64>(input) << std::endl;
        std::cout << "GHR : " << GHR << std::endl;
        std::cout << "hash: " << std::bitset<64>(hash) << std::endl << std::endl;
        */

        return hash;
    }

    Prediction get_prediction(const uint64_t pc) {

        Prediction prediction;
        prediction.provider_pred = prediction.alt_pred = base_predictor->predict(pc, 0);

        int table_idx = N_TABLES - 1;

        // look for the first a valid entry in the tagged tables
        for(; table_idx >= 0; table_idx--) {
            uint64_t h = hash(pc, GHR, T_GHR_LEN[table_idx]);
            try {
                const TableEntry provider_entry = tables.at(table_idx).get(h);

                prediction.provider_idx = table_idx;
                prediction.provider_pred = provider_entry.prediction.get_prediction();
                prediction.provider_is_weak = provider_entry.prediction.is_weak() && provider_entry.u.get() == 0;

                break;
            } catch (...) {}
        }


        table_idx --;
        for(; table_idx >= 0; table_idx--) {
            uint64_t h = hash(pc, GHR, T_GHR_LEN[table_idx]);
            try {
                const TableEntry alternative_entry = tables.at(table_idx).get(h);

                prediction.alt_idx = table_idx;
                prediction.alt_pred = alternative_entry.prediction.get_prediction();

            } catch (...) {}
        }


        return prediction;

    }



public:

    explicit TAGE() {
        tables.reserve(N_TABLES);

        for(int i = 0; i < N_TABLES; i++) {
            tables.emplace_back(i);
        }
    }

    bool predict(const uint64_t seq_no, const uint8_t /* piece */ ) {
        const Prediction p = get_prediction(seq_no);

        if (p.provider_is_weak) {
            // actual prediction will come from alternative predictor
            if (p.alt_idx != DEFAULT_PROVIDER) tables.at(p.alt_idx).total_predictions += 1;
            return p.alt_pred;
        }

        // actual prediction comes from provider
        if (p.provider_idx != DEFAULT_PROVIDER) tables.at(p.provider_idx).total_predictions += 1;
        return p.provider_pred;
    }

    void update(const uint64_t seq_no, const uint8_t piece, const bool actual_outcome) {

        Prediction prediction = get_prediction(seq_no);

        // update u
        if (prediction.provider_pred != prediction.alt_pred && prediction.provider_idx != DEFAULT_PROVIDER) {
            try {
                uint64_t h = hash(seq_no, GHR, T_GHR_LEN[prediction.provider_idx]);
                TableEntry& provider_entry = tables.at(prediction.provider_idx).get(h);
                actual_outcome == prediction.provider_pred ? provider_entry.u.increase() : provider_entry.u.decrease();
            } catch (...) {
                std::cout << "Provider prediction entry UNEXPECTEDLY not found (1)" << std::endl;
            }
        }

        // update prediction counter of provider component
        try {
            if (prediction.provider_idx != DEFAULT_PROVIDER) {
                uint64_t h = hash(seq_no, GHR, T_GHR_LEN[prediction.provider_idx]);
                TableEntry& provider_entry = tables.at(prediction.provider_idx).get(h);
                provider_entry.prediction.update_prediction(actual_outcome);
            } else {
                base_predictor->update(seq_no, piece, actual_outcome);
            }
        } catch (std::exception &e) {
            std::cout << e.what() << std::endl;
            std::cout << "Provider prediction entry UNEXPECTEDLY not found (2)" << std::endl;
        }

        // the overall prediction is incorrect
        if (prediction.provider_pred != actual_outcome) {
            uint8_t idx = prediction.provider_idx + 1;

            // try to allocate a new entry
            for(; idx < N_TABLES; idx++) {
                uint64_t h = hash(seq_no, GHR, T_GHR_LEN[idx]);
                const bool allocation_success = tables.at(idx).allocate(h, actual_outcome);
                if (allocation_success) break;
            }

            // if the allocation did not succeed in any of the "bigger" tables, decrease the u in those tables
            if (idx >= N_TABLES) {
                for(int i = prediction.provider_idx + 1; i < N_TABLES; i++) {
                    uint64_t h = hash(seq_no, GHR, T_GHR_LEN[i]);
                    tables.at(i).decrease_u(h);
                }
            }
        }

        num_branches = (num_branches + 1) % RESET_INTERVAL;
        // reset of u
        if (num_branches == 0) {
            for(int i = 0; i < N_TABLES; i++)
                tables.at(i).reset_u(hard_reset);

            hard_reset = !hard_reset;
        }


        // update the global history register
        GHR = GHR << 1;
        if(actual_outcome) GHR.set(0);

    }

    void print_tables() {
        std::cout << "===== Base Predictor =====" << std::endl;
        base_predictor->print();

        for(int table_idx = 0; table_idx < N_TABLES; table_idx++) {
            std::cout << "===== T" << table_idx << " Table =====" << std::endl;
            //tables[table_idx].print();
        }

    }

    [[nodiscard]] int get_bit_size() const {
        int total_size = 0;

        // add each table size
        for(int table_idx = 0; table_idx < N_TABLES; table_idx++)
            total_size += tables.at(table_idx).get_bit_size();

        // add base predictor size
        total_size += base_predictor->get_bit_size();

        // add GHR register and BRANCH COUNTER
        total_size += GHR_LEN;
        total_size += 64;

        return total_size;
    }

    void print_statistics() const {
        for (int i = 0; i < N_TABLES; i++) {
            std::cout << "Table[" << i <<
                "]\n\t occupation: " << tables.at(i).get_occupation() << " / " << tables.at(i).get_tot_entries()
            << "\n\t total predictions: " << tables.at(i).total_predictions << std::endl;
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const TAGE& tage);
};

inline std::ostream& operator<<(std::ostream& os, const TAGE& tage) {
    tage.base_predictor->print();

    for (const auto& table : tage.tables) {
        os << table << std::endl;
    }

    return os;
}

#endif //TAGE_H
static TAGE tage;
