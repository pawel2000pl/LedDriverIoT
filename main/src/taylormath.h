#ifndef FIXED_MATH
#define FIXED_MATH

#include <cmath>
#include <limits>
#include <cstdint>
#include <functional>

#ifndef __glibc_unlikely
#define __glibc_unlikely
#endif
#ifndef __glibc_likely
#define __glibc_likely
#endif

namespace taylor {

    extern std::uint64_t gamma_tab[];
    extern std::uint64_t pochhammer_counters[];

    template<typename T>
    T base_trig_pos(T x, bool sine) {
        T result = 0;
        T poly = sine ? x : (T)1;
        T x2 = x*x;
        unsigned add_gamma = sine ? 1 : 0;
        unsigned i = 0;
        while (i < 10) {
            T part = poly / gamma_tab[(i<<1)|add_gamma];
            if (__glibc_unlikely(part <= 0)) return result;
            T new_result = (++i & 1) ? (result + part) : (result - part);
            if (__glibc_unlikely(result == new_result)) return result;
            result = new_result;
            poly *= x2;
        }
        return result;
    }


    template<typename T>
    T base_trig(T x) {
        if (x < 0) x = -x;
        constexpr static T pi2 = M_PI / 2;
        constexpr static T pi4 = M_PI / 4;
        unsigned n = std::floor(x / pi2);
        T rest = x - n * pi2;
        n &= 3;
        if (n & 1) rest = pi2 - rest;
        T result = (rest > pi4) ? base_trig_pos<T>(pi2 - rest, true) : base_trig_pos<T>(rest, false);
        return (n == 1 || n == 2) ? -result : result;
    }


    template<typename T>
    T sin(T x) {
        constexpr static T pi2 = M_PI / 2;
        return base_trig<T>(x - pi2);
    }


    template<typename T>
    T cos(T x) {
        return base_trig<T>(x);
    }


    template<typename T>
    T sqrt(T s) {
        if (s == 0) return T(0);
        constexpr static const unsigned max_iter = std::round(std::sqrt(sizeof(T)*4));     
        constexpr static const T half = 0.5;        
        T x = (s < 1) ? 2*s : (((s-1) * half)+1);
        for (unsigned i=0;i<max_iter;i++) {
            x = (x + s / x)*half;
        };
        return x;
    }


    template<typename T>
    T base_asin1(T x) {
        T result = 0;
        T poly = x;
        T x2 = x*x;
        for (unsigned i=0;i<18;i++) {
            T part = (T)pochhammer_counters[i] / (1ULL << i) * poly / (gamma_tab[i] + 2 * i * gamma_tab[i]);
            if (__glibc_unlikely(part <= 0)) return result;
            T new_result = (result + part);
            if (__glibc_unlikely(result == new_result)) return result;
            result = new_result;
            poly *= x2;
        }
        return result;
    }


    template<typename T>
    T base_asin2(T x) {
        constexpr const static T pi2 = M_PI / 2;
        x = 1-x;
        T result = 0;
        T coeff = sqrt<T>(2*x);
        T poly = 1;
        for (unsigned i=0;i<18;i++) {
            T part = (T)pochhammer_counters[i] / (1ULL << (i<<1)) * poly / (gamma_tab[i] + 2 * i * gamma_tab[i]);
            if (__glibc_unlikely(part <= 0)) return pi2 - coeff * result;
            T new_result = (result + part);
            if (__glibc_unlikely(result == new_result)) return pi2 - coeff * result;
            result = new_result;
            poly *= x;
        }
        return pi2 - coeff * result;
    }


    template<typename T>
    T calculate_asin_half() {
        T test = 1;
        test /= 2;
        T half = test;
        while (test > 0) {
            test /= 2;
            T test_half1 = half + test;
            T test_half2 = half - test;
            T diff1 = base_asin2<T>(test_half1) - base_asin1<T>(test_half1);
            T diff2 = base_asin2<T>(test_half2) - base_asin1<T>(test_half2);
            half = (diff1 < diff2) ? test_half1 : test_half2;
        }
        return half;
    }


    template<typename T>
    T base_asin_pos(T x) {
        const static T half = calculate_asin_half<T>();
        return (x < half) ? base_asin1<T>(x) : base_asin2<T>(x);
    }


    template<typename T>
    T asin(T x) {
        return (x < 0) ? -base_asin_pos(-x) : base_asin_pos(x);
    }


    template<typename T>
    T ln_small(T x) {
        x -= 1;
        T poly = x;
        T result = 0;
        unsigned i = 1;
        while (1) {
            T part = poly / i;            
            if (__glibc_unlikely(part > (T)(1) || part < (T)(-1) || part == 0)) return result;
            T new_result = (i++ & 1) ? (result + part) : (result - part);
            if (__glibc_unlikely(result == new_result)) return result;
            result = new_result;
            poly *= x;
        }
    }


    // template<typename T>
    // T ln_part2(T x) {
    //     x -= 1;
    //     T poly = x;
    //     T result = 0;
    //     unsigned i = 1;
    //     while (1) {
    //         T part = 1 / (poly * i);
    //         T new_result = (++i & 1) ? (result + part) : (result - part);
    //         if (__glibc_unlikely(result == new_result)) return result;
    //         result = new_result;
    //         poly *= x;
    //     }
    // }


    template<typename T>
    T ln(T x) {
        return (x < 2) ? ln_small<T>(x) : 2*ln(sqrt<T>(x)); //(ln<T>(x-1) - ln_part2<T>(x));
    }


    template<typename T>
    T log(T x) {
        return ln<T>(x);
    }


    template<typename T>
    T exp_small_pos(T x) {
        T poly = 1;
        T result = 0;
        for (unsigned i=0;i<21;i++) {
            T part = poly / gamma_tab[i];
            if (__glibc_unlikely(part <= 0)) return result;
            T new_result = result + part;
            if (__glibc_unlikely(result == new_result)) return result;
            result = new_result;
            poly *= x;
        }
        return result;
    }


    template<typename T>
    T pown(T x, unsigned n) {
        T ans = 1;
        while (n > 0) {
            int last_bit = (n & 1);
            if (last_bit) {
                ans = ans * x;
            }
            x = x * x;
            n = n >> 1;
        }
        return ans;
    }


    template<typename T>
    T exp(T x) {
        if (x == 0) return 1;
        if (x < 0) return 1 / exp(-x);
        if (x <= 1) return exp_small_pos<T>(x);
        unsigned long long int n = std::floor(x);
        T rest = x - n;
        static T exp1 = 0;
        if (__glibc_unlikely(exp1 == 0)) exp1 = exp_small_pos<T>(1);
        return exp_small_pos<T>(rest) * pown<T>(exp1, n);
    }


}

#endif
