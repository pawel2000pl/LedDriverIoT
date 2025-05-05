/****************************************************************************************

                                       MIT License

               Copyright (c) 2025 Pawel Bielecki [pbielecki2000@gmail.com]

       Permission is hereby granted, free of charge, to any person obtaining a copy
      of this software and associated documentation files (the "Software"), to deal
       in the Software without restriction, including without limitation the rights
        to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
          copies of the Software, and to permit persons to whom the Software is
                 furnished to do so, subject to the following conditions:

      The above copyright notice and this permission notice shall be included in all
                     copies or substantial portions of the Software.

        THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
         IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
       FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
          AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
      LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
      OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
                                        SOFTWARE.

****************************************************************************************/


#ifndef FIXED_MATH
#define FIXED_MATH

#include <cmath>
#include <limits>
#include <utility>
#include <cstdint>
#include <type_traits>

#ifndef __glibc_unlikely
#define __glibc_unlikely
#endif
#ifndef __glibc_likely
#define __glibc_likely
#endif

namespace taylor {

    extern std::size_t gamma_tab_size;
    extern std::uint64_t gamma_tab[];
    extern std::size_t asin_divisors_tab_size;
    extern std::uint64_t asin_divisors_tab[];
    extern std::size_t pochhammer_counters_size;
    extern std::uint64_t pochhammer_counters[];

    extern bool constants_initilized;

    template<typename T>
    struct has_shift_left {
    private:
        template<typename U>
        static auto test(int) -> decltype(std::declval<U>() << std::declval<int>(), std::true_type());
    
        template<typename>
        static std::false_type test(...);
    
    public:
        static constexpr bool value = decltype(test<T>(0))::value;
    };

    template<typename T>
    struct has_shift_right {
    private:
        template<typename U>
        static auto test(int) -> decltype(std::declval<U>() >> std::declval<int>(), std::true_type());
    
        template<typename>
        static std::false_type test(...);
    
    public:
        static constexpr bool value = decltype(test<T>(0))::value;
    };

    template<typename T>
    typename std::enable_if<has_shift_left<T>::value, T>::type mul_by_pow2(T x, unsigned n=1) {
        return x << n;
    }

    template<typename T>
    typename std::enable_if<!has_shift_left<T>::value, T>::type mul_by_pow2(T x, unsigned n=1) {
        return x * ((std::size_t)1 << n);
    }

    template<typename T>
    typename std::enable_if<has_shift_right<T>::value, T>::type div_by_pow2(T x, unsigned n=1) {
        return x >> n;
    }

    template<typename T>
    typename std::enable_if<!has_shift_right<T>::value, T>::type div_by_pow2(T x, unsigned n=1) {
        return x / ((std::size_t)1 << n);
    }


    template<typename T>
    T base_trig_pos(T x, bool sine) {
        T result = 0;
        T poly = sine ? x : (T)1;
        T x2 = x*x;
        unsigned add_gamma = sine ? 1 : 0;
        unsigned i = 0;
        while (1) {
            unsigned j = (i<<1)|add_gamma;
            if (j >= gamma_tab_size) break;
            T part = poly / gamma_tab[j];
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
        static const unsigned max_iter = std::round(std::sqrt(
            (std::log2((double)std::numeric_limits<T>::max()) - std::log2((double)std::numeric_limits<T>::min())) / 2
        ));   
        constexpr static const T third = (T)1 / 3;   
        T x = (s < third) ? mul_by_pow2<T>(s, 1) : (div_by_pow2<T>(s - 1, 1) + 1);
        for (unsigned i=0;i<max_iter;i++) {
            T nx = div_by_pow2<T>(x + s / x, 1);
            if ((s > 1 ? nx > s : nx < s)) break;
            x = nx;
        };
        return x;
    }


    template<typename T>
    T base_asin1(T x) {
        T result = 0;
        T poly = x;
        T x2 = x*x;
        for (unsigned i=0;i<asin_divisors_tab_size;i++) {
            T part = div_by_pow2<T>(pochhammer_counters[i] * poly / asin_divisors_tab[i], i);            
            T new_result = result + part;
            if (__glibc_unlikely(result == new_result)) break;
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
        T coeff = sqrt<T>(mul_by_pow2<T>(x, 1));
        T poly = 1;
        for (unsigned i=0;i<asin_divisors_tab_size;i++) {
            T part = div_by_pow2<T>(pochhammer_counters[i] * poly / asin_divisors_tab[i], i << 1);
            T new_result = result + part;
            if (__glibc_unlikely(result == new_result)) break;
            result = new_result;
            poly *= x;
        }
        return pi2 - coeff * result;
    }


    template<typename T>
    T calculate_asin_half() {
        T test = div_by_pow2<T>(1, 1);
        T half = test;
        while (test > 0) {
            test = div_by_pow2<T>(test, 1);
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
        static const unsigned max_iter = std::round(
            (std::log2((double)std::numeric_limits<T>::max()) - std::log2((double)std::numeric_limits<T>::min()))
        );   
        for (unsigned i=1;i<max_iter;i++) {
            T part = poly / i;            
            T new_result = (i & 1) ? (result + part) : (result - part);
            if (__glibc_unlikely(result == new_result)) break;
            result = new_result;
            poly *= x;
        }
        return result;
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
        return ((x > 0.2) && x < (T)1.5f) ? ln_small<T>(x) : mul_by_pow2<T>(ln(sqrt<T>(x)), 1); //(ln<T>(x-1) - ln_part2<T>(x));
    }


    template<typename T>
    T log(T x) {
        return ln<T>(x);
    }


    template<typename T>
    T exp_small_pos(T x) {
        T poly = 1;
        T result = 0;
        for (unsigned i=0;i<gamma_tab_size;i++) {
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
