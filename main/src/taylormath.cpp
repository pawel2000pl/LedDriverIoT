#include "taylormath.h"

namespace taylor {

    const std::size_t max_constant_tab_size = 32;

    std::size_t gamma_tab_size = 0;
    std::uint64_t gamma_tab[max_constant_tab_size];

    std::size_t asin_divisors_tab_size = 0;
    std::uint64_t asin_divisors_tab[max_constant_tab_size];

    std::size_t pochhammer_counters_size = 0;
    std::uint64_t pochhammer_counters[max_constant_tab_size];


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
        std::size_t double_factorial_tab_size = 0;
        std::uint64_t double_factorial_tab[max_constant_tab_size*2];

        double_factorial_tab_size = 0;
        double_factorial_tab[double_factorial_tab_size++] = 1;
        double_factorial_tab[double_factorial_tab_size++] = 1;
        while (double_factorial_tab_size < max_constant_tab_size*2) {
            std::uint64_t new_value = double_factorial_tab[double_factorial_tab_size-2] * double_factorial_tab_size;
            if (new_value / double_factorial_tab_size != double_factorial_tab[double_factorial_tab_size-2]) break;
            double_factorial_tab[double_factorial_tab_size++] = new_value;
        }

        pochhammer_counters_size = 0;
        pochhammer_counters[pochhammer_counters_size++] = 1;
        while (1) {
            unsigned n = (pochhammer_counters_size << 1) - 1;
            if (n >= double_factorial_tab_size) break;
            pochhammer_counters[pochhammer_counters_size++] = double_factorial_tab[n];
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
