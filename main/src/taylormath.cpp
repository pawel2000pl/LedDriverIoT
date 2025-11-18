#include "taylormath.h"

namespace taylor {

    const std::size_t max_constant_tab_size = 32;

    std::size_t gamma_tab_size = 0;
    std::uint64_t gamma_tab[max_constant_tab_size];

    std::size_t asin_divisors_tab_size = 0;
    std::uint64_t asin_divisors_tab[max_constant_tab_size];

    std::size_t pochhammer_counters_size = 0;
    std::uint64_t pochhammer_counters[max_constant_tab_size];

    std::size_t loop_counter = 0;

    void generate_gamma_tab() {
        gamma_tab_size = 0;
        gamma_tab[gamma_tab_size++] = 1;
        while (gamma_tab_size < max_constant_tab_size) {
            std::uint64_t new_value = gamma_tab[gamma_tab_size-1] * gamma_tab_size;
            if (new_value / gamma_tab_size != gamma_tab[gamma_tab_size-1]) break;
            gamma_tab[gamma_tab_size++] = new_value;
        }
    }


    void generate_asin_divisors_tab() {
        for (unsigned i=0;i<gamma_tab_size;i++) {
            asin_divisors_tab[i] = gamma_tab[i] + gamma_tab[i] * (i << 1);
            if ((asin_divisors_tab[i] - gamma_tab[i]) / gamma_tab[i] != (i << 1)) {
                asin_divisors_tab_size = i;
                return;
            }
        }
        asin_divisors_tab_size = gamma_tab_size;
    }


    void generate_pochhammer_counters() {
        pochhammer_counters_size = 0;
        std::uint64_t factorial = 1;
        pochhammer_counters[pochhammer_counters_size++] = 1;
        while (pochhammer_counters_size < max_constant_tab_size) {
            unsigned n = (pochhammer_counters_size << 1) - 1;
            std::uint64_t new_factorial = factorial * (std::uint64_t)n;
            if (new_factorial / n != factorial) break;
            factorial = new_factorial;
            pochhammer_counters[pochhammer_counters_size++] = new_factorial;
        }
    }


    bool initialize_constants() {
        generate_gamma_tab();
        generate_asin_divisors_tab();
        generate_pochhammer_counters();
        return 1;
    }


    bool constants_initilized = initialize_constants();


}
