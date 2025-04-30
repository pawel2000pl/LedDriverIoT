#ifndef FIXED_MATH
#define FIXED_MATH

#include <functional>
#include <cmath>
#include <limits>

#ifndef __glibc_unlikely
#define __glibc_unlikely
#endif
#ifndef __glibc_likely
#define __glibc_likely
#endif

namespace taylor {

    extern unsigned long long gamma_tab[];
    extern unsigned long long pochhammer_counters[];

    template<typename T>
    T base_trig_pos(T x, bool sine) {
        T result = 0;
        T poly = sine ? x : (T)1;
        T x2 = x*x;
        unsigned i = 0;
        while (i < 21) {
            T part = poly / gamma_tab[(i<<1)|1];
            if (__glibc_unlikely(part <= 0)) return result;
            T new_result = (++i & 1) ? (result + part) : (result - part);
            if (__glibc_unlikely(result == new_result)) return result;
            result = new_result;
            poly *= x2;
        }
        return result;
    }


    template<typename T>
    T base_trig(T x, bool sine) {
        bool reverse_sign = false;
        if (x < 0) {
            x = -x;
            if (sine) reverse_sign = true;
        }
        constexpr static T pi2 = M_PI / 2;
        constexpr static T pi4 = M_PI / 4;
        unsigned n = std::floor(x / pi2);
        T rest = x - n * pi2;
        T result = (rest > pi4) ? base_trig_pos<T>(pi2 - rest, !sine) : base_trig_pos<T>(rest, sine);
        reverse_sign ^= ((n & 3) > 1);
        return reverse_sign ? -result : result;
    }


    template<typename T>
    T sin(T x) {
        return base_trig<T>(x, true);
    }


    template<typename T>
    T cos(T x) {
        return base_trig<T>(x, false);
    }


    template<typename T>
    T sqrt(T x) {
        T test = 1;
        if (test < x) while (test * test < x) test *= 2;
        T result = 0;
        while (test > 0) {
            T new_result = result + test;
            T new_result_sqr = new_result * new_result;
            if (new_result_sqr == x)
                return new_result;
            if (new_result_sqr < x)
                result = new_result;
            test /= 2;
        }
        return result;
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
        constexpr static T pi2 = M_PI / 2;
        x = 1-x;
        T result = 0;
        T coeff = sqrt(2*x);
        T poly = x;
        T x2 = x*x;
        for (unsigned i=0;i<18;i++) {
            T part = (T)pochhammer_counters[i] / (1ULL << (i+1)) * poly / (gamma_tab[i] + 2 * i * gamma_tab[i]);
            if (__glibc_unlikely(part <= 0)) return pi2 - coeff * result;
            T new_result = (result + part);
            if (__glibc_unlikely(result == new_result)) return pi2 - coeff * result;
            result = new_result;
            poly *= x2;
        }
        return pi2 - coeff * result;
    }


    template<typename T>
    T base_asin_pos(T x) {
        static T half = 0;
        if (__glibc_unlikely(half == 0)) {
            T test = 1;
            while (test > 0) {
                test /= 2;
                T test_half = half + test;
                T v1 = base_asin1<T>(test_half);
                T v2 = base_asin2<T>(test_half);
                if (v1 <= v2) {
                    half = test_half;
                    if (v1 == v2) break;
                }
            }
        }
        return (x < half) ? base_asin1<T>(x) : base_asin2<T>(x);
    }


    template<typename T>
    T asin(T x) {
        return (x < 0) ? -base_asin_pos(-x) : base_asin_pos(x);
    }


    template<typename T>
    T ln1(T x) {
        x -= 1;
        T poly = x;
        T result = 0;
        unsigned i = 1;
        while (1) {
            T part = poly / i;
            T new_result = (i++ & 1) ? (result + part) : (result - part);
            if (__glibc_unlikely(result == new_result)) return result;
            result = new_result;
            poly *= x;
        }
    }


    template<typename T>
    T ln_part2(T x) {
        x -= 1;
        T poly = x;
        T result = 0;
        unsigned i = 1;
        while (1) {
            T part = 1 / (poly * i);
            T new_result = (++i & 1) ? (result + part) : (result - part);
            if (__glibc_unlikely(result == new_result)) return result;
            result = new_result;
            poly *= x;
        }
    }


    template<typename T>
    T ln(T x) {
        return (x < 2) ? ln1(x) : (ln(x-1) - ln_part2(x));
    }


    template<typename T>
    T exp_small_pos(T x) {
        // std::cout << x << "  --\n";
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
