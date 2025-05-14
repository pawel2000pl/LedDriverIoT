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


#ifndef FIXED_POINT
#define FIXED_POINT

#include <limits>
#include <cstddef>
#include <cstdint>
#include <typeindex>
#include <type_traits>

template<typename T>
struct make_fast_int {
    using type = T;
    static_assert(std::numeric_limits<T>::is_integer, "T must be an integer type.");
};

template<typename T>
struct make_least_int {
    using type = T;
    static_assert(std::numeric_limits<T>::is_integer, "T must be an integer type.");
};

#define __MAKE_INT_MAPPING(N, PREFIX)                                \
    template<>                                                       \
    struct make_##PREFIX##_int<std::int##N##_t> {                    \
        using type = std::int_##PREFIX##N##_t;                       \
    };                                                               \
    template<>                                                       \
    struct make_##PREFIX##_int<std::uint##N##_t> {                   \
        using type = std::uint_##PREFIX##N##_t;                      \
    };

#define __MAKE_INT_TRAITS(PREFIX) \
    __MAKE_INT_MAPPING(8, PREFIX) \
    __MAKE_INT_MAPPING(16, PREFIX) \
    __MAKE_INT_MAPPING(32, PREFIX) \
    __MAKE_INT_MAPPING(64, PREFIX)

__MAKE_INT_TRAITS(fast)
__MAKE_INT_TRAITS(least)

#undef __MAKE_INT_TRAITS
#undef __MAKE_INT_MAPPING


// T - type for data storage
// TC - type used for calculating products and quotients (best is double-sized T)
// frac_bits - count of fraction bits (bits after point)

template<typename T, typename TC=typename make_fast_int<T>::type, unsigned frac_bits=sizeof(T)*4-1>
class fixedpoint {

    public:

        #define FIXED_POINT_BOOL_TEMPLATE template<typename B, typename std::enable_if<std::is_same<B, bool>::value, B>::type* = nullptr>
        #define FIXED_POINT_INTEGER_TEMPLATE template<typename I, typename std::enable_if<std::is_integral<I>::value && !std::is_same<I, bool>::value, I>::type* = nullptr>
        #define FIXED_POINT_FLOAT_TEMPLATE template<typename FP, typename std::enable_if<std::is_floating_point<FP>::value, FP>::type* = nullptr>

        FIXED_POINT_INTEGER_TEMPLATE
        constexpr fixedpoint(const I value) noexcept : buf((T)value << frac_bits) {}

        FIXED_POINT_FLOAT_TEMPLATE
        constexpr fixedpoint(const FP value) noexcept : buf(value * ((T)1 << frac_bits)) {}

        constexpr fixedpoint() noexcept : buf(0) {}

        constexpr fixedpoint(const fixedpoint& another) noexcept = default;

        constexpr fixedpoint(fixedpoint&& another) noexcept = default;

        ~fixedpoint() noexcept = default;

        template<typename T2, typename TC2>
        constexpr fixedpoint(const fixedpoint<T2, TC2, frac_bits>& another) noexcept
            : buf(another.buf) {} 

        template<typename T2, typename TC2, unsigned frac_bits2>
        constexpr fixedpoint(const fixedpoint<T2, TC2, frac_bits2>& another) noexcept
            : buf((frac_bits > frac_bits2) ? ((T)another.buf << (frac_bits - frac_bits2)) : (another.buf >> (frac_bits2 - frac_bits))) {}


    private:

        template<typename U, U a, U b>
        struct static_min : std::conditional<(a < b), std::integral_constant<U, a>, std::integral_constant<U, b>>::type {};

        using bits_shift_large_num = std::integral_constant<std::size_t, static_min<std::size_t, (sizeof(TC) - sizeof(T)) * 4, frac_bits>::value>;
        using bits_shift_norm_element = std::integral_constant<std::size_t, ((frac_bits - bits_shift_large_num::value) >> 1)>;
        using bits_shift_imp_element = std::integral_constant<std::size_t, frac_bits - bits_shift_large_num::value - bits_shift_norm_element::value>;

        #if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        using is_little_endian = std::integral_constant<bool, __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__>;
        #else
        using is_little_endian = std::integral_constant<bool, false>;
        #endif

        template<typename F, typename I>
        struct Union2 {
            constexpr Union2(F x) : data{.x{x}} {}
            constexpr Union2(I i) : data{.i{i}} {}
            union {F x; I i;} data;
        };

        template<typename F, unsigned expected_size, unsigned fraction, unsigned exponents, typename int_buf = typename std::conditional<fraction <= 32, std::uint32_t, std::uint64_t>::type>
        static constexpr typename std::enable_if<std::numeric_limits<F>::is_iec559 && sizeof(F) == expected_size && is_little_endian::value, T>::type
        buf_from_ieee754_generic(Union2<F, typename std::make_unsigned<int_buf>::type> input) {
            using u_int_buf = typename std::make_unsigned<int_buf>::type;
            using s_int_buf = typename std::make_signed<int_buf>::type;
            using exp_mask = std::integral_constant<u_int_buf, (((u_int_buf)1 << exponents) - 1) << fraction>;
            using exp_offset = std::integral_constant<s_int_buf, ((s_int_buf)1 << (exponents-1)) - 1 + fraction - frac_bits>;
            using frac_one = std::integral_constant<u_int_buf, ((u_int_buf)1 << fraction)>;
            using frac_mask = std::integral_constant<u_int_buf, ((u_int_buf)1 << fraction) - 1>;
            using sign_mask = std::integral_constant<u_int_buf, (u_int_buf)1 << (fraction+exponents)>;
            s_int_buf exp_bits = (input.data.i & exp_mask::value) >> fraction;
            u_int_buf y = (input.data.i & frac_mask::value) | (exp_bits ? frac_one::value : 0);
            s_int_buf exp = exp_offset::value - exp_bits;
            T z = (exp < 0) ? y << (-exp) : y >> (exp);
            return (input.data.i & sign_mask::value) ? -z : z;
        }

        template<typename F, unsigned expected_size, unsigned fraction, unsigned exponents, typename int_buf = typename std::conditional<fraction <= 32, std::uint32_t, std::uint64_t>::type>
        static constexpr typename std::enable_if<!(std::numeric_limits<F>::is_iec559 && sizeof(F) == expected_size && is_little_endian::value), T>::type
        buf_from_ieee754_generic(F x) {
            return x * ((T)1 << frac_bits);
        }

        static constexpr T buf_from_ieee754(float x) {
            return buf_from_ieee754_generic<float, 4, 23, 8, std::uint32_t>(x);
        }

        static constexpr T buf_from_ieee754(double x) {
            return buf_from_ieee754_generic<double, 8, 52, 11, std::uint64_t>(x);
        }
    
    public:

        static constexpr fixedpoint from_ieee754(float x) {
            return fixedpoint(buf_from_ieee754(x), true);
        }

        static constexpr fixedpoint from_ieee754(double x) {
            return fixedpoint(buf_from_ieee754(x), true);
        }

        static fixedpoint from_float(float x) {
            return fixedpoint(buf_from_ieee754(x), true);
        }

        static fixedpoint from_double(double x) {
            return fixedpoint(buf_from_ieee754(x), true);
        }

        constexpr static fixedpoint buf_cast(const T buf) noexcept {
            return fixedpoint(buf, true);
        }

        constexpr static fixedpoint fraction(const T& counter, const T& denimonator) noexcept {
            return  fixedpoint((counter << frac_bits) / denimonator, true);
        }

        constexpr friend fixedpoint operator+(const fixedpoint first, const fixedpoint second) noexcept {
            return fixedpoint(first.buf + second.buf, true);
        }

        FIXED_POINT_INTEGER_TEMPLATE
        constexpr friend fixedpoint operator+(const fixedpoint first, const I second) noexcept {
            return fixedpoint(first.buf + ((T)second << frac_bits), true);
        }

        FIXED_POINT_INTEGER_TEMPLATE
        constexpr friend fixedpoint operator+(const I first, const fixedpoint second) noexcept {
            return fixedpoint(((T)first << frac_bits) + second.buf, true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        constexpr friend fixedpoint operator+(const fixedpoint first, const FP second) noexcept {
            return fixedpoint(first.buf + second * ((T)1 << frac_bits), true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        constexpr friend fixedpoint operator+(const FP first, const fixedpoint second) noexcept {
            return fixedpoint(first * ((T)1 << frac_bits) + second.buf, true);
        }

        constexpr friend fixedpoint operator-(const fixedpoint first, const fixedpoint second) noexcept {
            return fixedpoint(first.buf - second.buf, true);
        }

        FIXED_POINT_INTEGER_TEMPLATE
        constexpr friend fixedpoint operator-(const fixedpoint first, const I second) noexcept  {
            return fixedpoint(first.buf - ((T)second << frac_bits), true);
        }

        FIXED_POINT_INTEGER_TEMPLATE
        constexpr friend fixedpoint operator-(const I first, const fixedpoint second) noexcept {
            return fixedpoint(((T)first << frac_bits) - second.buf, true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        constexpr friend fixedpoint operator-(const fixedpoint first, const FP second) noexcept {
            return fixedpoint(first.buf - second * ((T)1 << frac_bits), true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        constexpr friend fixedpoint operator-(const FP first, const fixedpoint second) noexcept {
            return fixedpoint(first * ((T)1 << frac_bits) - second.buf, true);
        }

        constexpr friend fixedpoint operator*(const fixedpoint first, const fixedpoint second) noexcept {
            return fixedpoint((((TC)first.buf >> bits_shift_imp_element::value) * ((TC)second.buf >> bits_shift_norm_element::value)) >> bits_shift_large_num::value, true);
        }

        FIXED_POINT_INTEGER_TEMPLATE
        constexpr friend fixedpoint operator*(const fixedpoint first, const I second) noexcept {
            return fixedpoint(first.buf * (T)second, true);
        }

        FIXED_POINT_INTEGER_TEMPLATE
        constexpr friend fixedpoint operator*(const I first, const fixedpoint second) noexcept {
            return fixedpoint((T)first * second.buf, true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        constexpr friend fixedpoint operator*(const fixedpoint first, const FP second) noexcept {
            return fixedpoint(first.buf * second, true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        constexpr friend fixedpoint operator*(const FP first, const fixedpoint second) noexcept {
            return fixedpoint(first * second.buf, true);
        }

        constexpr friend fixedpoint operator/(const fixedpoint first, const fixedpoint second) noexcept {
            T divisor = second.buf >> bits_shift_norm_element::value;
            return (bits_shift_norm_element::value && (divisor == (divisor >> 1)))
                ? (((first.buf < 0) != (second.buf < 0)) ? std::numeric_limits<fixedpoint>::lowest() : std::numeric_limits<fixedpoint>::max())
                : fixedpoint((((TC)first.buf << bits_shift_large_num::value) / (TC)divisor) << bits_shift_imp_element::value, true);
        }

        FIXED_POINT_INTEGER_TEMPLATE
        constexpr friend fixedpoint operator/(const fixedpoint first, const I second) noexcept {
            return fixedpoint(first.buf / (T)second, true);
        }

        FIXED_POINT_INTEGER_TEMPLATE
        constexpr friend fixedpoint operator/(const I first, const fixedpoint second) noexcept {            
            T divisor = second.buf >> bits_shift_norm_element::value;
            return (bits_shift_norm_element::value && (divisor == (divisor >> 1)))
                ? (((first < 0) != (second.buf < 0)) ? std::numeric_limits<fixedpoint>::lowest() : std::numeric_limits<fixedpoint>::max())
                : fixedpoint((((TC)first << (bits_shift_large_num::value + frac_bits)) / (TC)divisor) << bits_shift_imp_element::value, true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        constexpr friend fixedpoint operator/(const fixedpoint first, const FP second) noexcept {            
            TC divisor = (TC)(second * ((TC)1 << (frac_bits - bits_shift_norm_element::value)));
            return (bits_shift_norm_element::value && (divisor == (divisor >> 1)))
                ? (((first.buf < 0) != (second.buf < 0)) ? std::numeric_limits<fixedpoint>::lowest() : std::numeric_limits<fixedpoint>::max())
                : fixedpoint((((TC)first.buf << bits_shift_large_num::value) / (TC)divisor) << bits_shift_imp_element::value, true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        constexpr friend fixedpoint operator/(const FP first, const fixedpoint second) noexcept {            
            T divisor = second.buf >> bits_shift_norm_element::value;
            return (bits_shift_norm_element::value && (divisor == (divisor >> 1)))
                ? (((first.buf < 0) != (second.buf < 0)) ? std::numeric_limits<fixedpoint>::lowest() : std::numeric_limits<fixedpoint>::max())
                : fixedpoint(((TC)(first * ((TC)1 << (bits_shift_large_num::value + frac_bits))) / (TC)divisor) << bits_shift_imp_element::value, true);
        }

        constexpr friend fixedpoint operator%(const fixedpoint first, const fixedpoint second) noexcept {
            return fixedpoint(first.buf % second.buf, true);
        }

        FIXED_POINT_INTEGER_TEMPLATE
        friend fixedpoint operator%(const fixedpoint first, const I second) noexcept {
            return fixedpoint(first.buf % ((T)second << frac_bits), true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        constexpr friend fixedpoint operator%(const fixedpoint first, const FP second) noexcept {
            return fixedpoint(first.buf % (second * ((T)1 << frac_bits)), true);
        }

        FIXED_POINT_INTEGER_TEMPLATE
        constexpr friend fixedpoint operator<<(const fixedpoint first, const I second) noexcept {
            return fixedpoint(first.buf << second, true);
        }

        FIXED_POINT_INTEGER_TEMPLATE
        constexpr friend fixedpoint operator>>(const fixedpoint first, const I second) noexcept {
            return fixedpoint(first.buf >> second, true);
        }

        #define FixedPointOperatorMaker(oper, type1, param1, type2, param2) \
        FIXED_POINT_TEMPLATE_DECLARATION \
        constexpr friend bool operator oper (const type1 first, const type2 second) noexcept { \
            return (param1) oper (param2); \
        }

        #define FixedPointOperatorsMaker(type1, param1, type2, param2) \
        FixedPointOperatorMaker(==, type1, param1, type2, param2) \
        FixedPointOperatorMaker(!=, type1, param1, type2, param2) \
        FixedPointOperatorMaker(<, type1, param1, type2, param2) \
        FixedPointOperatorMaker(>, type1, param1, type2, param2) \
        FixedPointOperatorMaker(<=, type1, param1, type2, param2) \
        FixedPointOperatorMaker(>=, type1, param1, type2, param2)

        #define FIXED_POINT_TEMPLATE_DECLARATION
        FixedPointOperatorsMaker(fixedpoint, first.buf, fixedpoint, second.buf)
        #undef FIXED_POINT_TEMPLATE_DECLARATION
        #define FIXED_POINT_TEMPLATE_DECLARATION FIXED_POINT_INTEGER_TEMPLATE
        FixedPointOperatorsMaker(fixedpoint, first.buf, I, (T)second << frac_bits)
        FixedPointOperatorsMaker(I, (T)first << frac_bits, fixedpoint, second.buf)
        #undef FIXED_POINT_TEMPLATE_DECLARATION
        #define FIXED_POINT_TEMPLATE_DECLARATION FIXED_POINT_FLOAT_TEMPLATE
        FixedPointOperatorsMaker(fixedpoint, first.buf, FP, second * ((T)1 << frac_bits))
        FixedPointOperatorsMaker(FP, first * ((T)1 << frac_bits), fixedpoint, second.buf)
        #undef FIXED_POINT_TEMPLATE_DECLARATION

        #undef FixedPointOperatorsMaker
        #undef FixedPointOperatorMaker

        fixedpoint& operator=(const fixedpoint&) = default;

        fixedpoint& operator=(fixedpoint&& other) = default;

        void operator+=(const fixedpoint another) noexcept {
            buf += another.buf;
        }

        FIXED_POINT_INTEGER_TEMPLATE
        void operator+=(const I another) noexcept {
            buf += (T)another << frac_bits;
        }

        void operator-=(const fixedpoint another) noexcept {
            buf -= another.buf;
        }

        FIXED_POINT_INTEGER_TEMPLATE
        void operator-=(const I another) noexcept {
            buf -= (T)another << frac_bits;
        }

        void operator*=(const fixedpoint another) noexcept {
            buf = (((TC)buf >> bits_shift_imp_element::value) * ((TC)another.buf >> bits_shift_norm_element::value)) >> bits_shift_large_num::value;
        }

        FIXED_POINT_INTEGER_TEMPLATE
        void operator*=(const I another) {
            buf *= (T)another;
        }

        FIXED_POINT_FLOAT_TEMPLATE
        void operator*=(const FP another) {
            buf *= another;
        }

        void operator/=(const fixedpoint another) noexcept {                 
            T divisor = another.buf >> bits_shift_norm_element::value;
            buf = (bits_shift_norm_element::value && (divisor == (divisor >> 1)))
                ? (((buf < 0) != (another.buf < 0)) ? std::numeric_limits<T>::lowest() : std::numeric_limits<T>::max())
                : (((TC)buf << bits_shift_large_num::value) / (TC)divisor) << bits_shift_imp_element::value;                      
        }

        FIXED_POINT_INTEGER_TEMPLATE
        void operator/=(const I another) noexcept {
            buf /= (T)another;
        }

        FIXED_POINT_FLOAT_TEMPLATE
        void operator/=(const FP another) noexcept {
            TC divisor = (TC)(another * ((TC)1 << (frac_bits - bits_shift_norm_element::value)));;
            buf = (bits_shift_norm_element::value && (divisor == (divisor >> 1)))
                ? (((buf < 0) != (another.buf < 0)) ? std::numeric_limits<T>::lowest() : std::numeric_limits<T>::max())
                : (((TC)buf << bits_shift_large_num::value) / (TC)divisor) << bits_shift_imp_element::value;  
        }

        void operator%=(const fixedpoint another) noexcept {
            buf %= another.buf;
        }

        FIXED_POINT_INTEGER_TEMPLATE
        void operator%=(const I another) noexcept {
            buf %= (T)another;
        }

        FIXED_POINT_FLOAT_TEMPLATE
        void operator%=(const FP another) noexcept {
            buf %= another;
        }

        FIXED_POINT_INTEGER_TEMPLATE
        void operator<<=(const I another) noexcept {
            buf <<= another;
        }

        FIXED_POINT_INTEGER_TEMPLATE
        void operator>>=(const I another) noexcept {
            buf >>= another;
        }

        constexpr fixedpoint operator+() const noexcept {
            return fixedpoint(buf, true);
        }

        constexpr fixedpoint operator-() const noexcept {
            return fixedpoint(-buf, true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        constexpr operator FP() const noexcept {
            constexpr const FP k = ((FP)1) / ((FP)((T)1 << frac_bits));
            return (FP)buf * k;
        }

        FIXED_POINT_INTEGER_TEMPLATE
        constexpr operator I() const noexcept {
            return buf >> frac_bits;
        }
        
        FIXED_POINT_BOOL_TEMPLATE
        constexpr operator B() noexcept {
            return (B)buf;
        }

        constexpr T getBuf() const noexcept {
            return buf;
        }

        constexpr bool isNeg() const noexcept {
            return buf < 0;
        }

        constexpr bool isPos() const noexcept {
            return buf > 0;
        }

        constexpr bool isZero() const noexcept {
            return buf == 0;
        }

        constexpr bool isNotZero() const noexcept {
            return buf != 0;
        }

        constexpr T getfrac() const noexcept {
            return buf & (((T)1 << frac_bits) - 1);
        }

        unsigned toCharBuf(char* buffer, unsigned char base=10, unsigned max_frac_digits=(unsigned)(-1)) const {
            char* wbuf = buffer;
            T tmpBuf = buf;
            if (tmpBuf < 0) {
                *(wbuf++) = '-';
                tmpBuf = -tmpBuf;
                if (tmpBuf < 0) {
                    *(wbuf++) = '-';
                    *(wbuf++) = 'i';
                    *(wbuf++) = 'n';
                    *(wbuf++) = 'f';
                    *wbuf = 0;
                    return wbuf - buffer;
                }
            }
            T intPart = tmpBuf >> frac_bits;
            T fracPart = tmpBuf - (intPart << frac_bits);
            char* swapStart = wbuf - 1;
            do {
                *(wbuf++) = num2chr(intPart % base);
                intPart /= base;
            } while (intPart > 0);
            char* swapEnd = wbuf;
            while (++swapStart < --swapEnd) {
                *swapStart ^= *swapEnd;
                *swapEnd ^= *swapStart;
                *swapStart ^= *swapEnd;
            }
            if (fracPart) {
                *(wbuf++) = '.';
                while (fracPart && max_frac_digits--) {
                    fracPart *= 10;
                    T digit = fracPart >> frac_bits;
                    fracPart -= digit << frac_bits;
                    *(wbuf++) = num2chr(digit);
                }
            }
            *wbuf = 0;
            return wbuf - buffer;
        }

        static fixedpoint fromCharBuf(const char* buffer, unsigned char base=10, unsigned* size=NULL) {
            fixedpoint result = 0;
            fixedpoint multiplier = 1;
            bool frac_mode = false;
            unsigned position = 0;
            unsigned max_size = size ? *size : (unsigned)(-1);
            bool minus = buffer[position] == '-';
            if (buffer[position] == '-' || buffer[position] == '+') position++;
            position--;
            while (++position < max_size && buffer[position] && multiplier) {
                if (buffer[position] == '.') {
                    if (frac_mode) break;
                    frac_mode = true;
                    continue;
                }
                unsigned num = chr2num(buffer[position]);
                if (num >= base) break;
                if (frac_mode) {
                    multiplier /= base;
                    result += multiplier * num;
                } else {
                    result *= base;
                    result += num;
                }
            }
            if (size) *size = position;
            return minus ? -result : result;
        }

        // each bit (the lowest base) + sign + point + zero-char + padding
        using stream_buf_size = std::integral_constant<unsigned, sizeof(T)*8+4>;

        template<class string_type>
        string_type toString(unsigned char base=10, unsigned max_frac_digits=(unsigned)(-1)) const {
            char buf[stream_buf_size::value];
            toCharBuf(buf, base, max_frac_digits);
            return string_type((const char*)buf);
        }

        template<class string_type>
        static fixedpoint fromString(const string_type& str, unsigned char base=10, unsigned* size=NULL) {
            return fixedpoint::fromCharBuf(str.c_str(), base, size);
        }

        template<class Stream>
        friend Stream& operator << (Stream& stream, const fixedpoint fp) {
            char buf[stream_buf_size::value];
            fp.toCharBuf(buf);
            stream << buf;
            return stream;
        }

        template<class Stream>
        friend Stream& operator >> (Stream& stream, fixedpoint& fp) {
            char buf[stream_buf_size::value];
            unsigned position = 0;
            char peeked = stream.peek();
            if (peeked == '-' || peeked == '+') buf[position++] = stream.get();
            while (1) {
                peeked = stream.peek();
                if (peeked >= '0' && peeked <= '9')
                    buf[position++] = stream.get();
                else break;
            }
            if (stream.peek() == '.') {
                buf[position++] = stream.get();
                while (1) {
                    peeked = stream.peek();
                    if (peeked >= '0' && peeked <= '9')
                        buf[position++] = stream.get();
                    else break;
                }
            }
            buf[position++] = 0;
            fp = fixedpoint::fromCharBuf(buf);
            return stream;
        }        

        friend struct std::numeric_limits<fixedpoint<T, TC, frac_bits>>;

        template<typename T2, typename TC2, unsigned frac_bits2>
        friend class fixedpoint;

        static_assert(std::is_arithmetic<T>::value, "Type for the buf must be arithmetic.");
        static_assert(std::is_arithmetic<TC>::value, "Type helper for multiplication must be arithmetic.");
        static_assert(std::numeric_limits<T>::radix == 2, "Type for the buf must be binary (radix == 2).");
        static_assert(std::numeric_limits<TC>::radix == 2, "Type helper for multiplication must be binary (radix == 2).");
        static_assert(std::numeric_limits<T>::is_integer, "Type for the buf must be an integer type.");
        static_assert(std::numeric_limits<TC>::is_integer, "Type for multiplication must be an integer type.");
        static_assert(std::is_signed<T>::value == std::is_signed<TC>::value, "T and TC must be both signed or unsigned.");
        static_assert(sizeof(T) * 8 - std::is_signed<T>::value > frac_bits, "There must be less fraction bits than number of bits in the buf type.");
        static_assert(sizeof(T) <= sizeof(TC), "Type for multiplication must be equal or greater than buf type.");        

        #undef FIXED_POINT_BOOL_TEMPLATE
        #undef FIXED_POINT_INTEGER_TEMPLATE
        #undef FIXED_POINT_FLOAT_TEMPLATE

    private:
        T buf;

        constexpr fixedpoint(const T new_buf, bool) noexcept : buf(new_buf) {}

        static constexpr unsigned char num2chr(unsigned char num) noexcept {
            return (num <= 9) ? ('0' + num) : (('a' - 10) + num);
        }

        static constexpr unsigned char chr2num(unsigned char chr) noexcept {
            return (chr <= '9') ? (chr - '0') : (chr >= 'a') ? (chr - ('a' - 10)) : (chr - ('A' - 10));
        }

};


namespace std {

    template<typename T, typename TC, unsigned frac_bits>
    constexpr T floor(const fixedpoint<T, TC, frac_bits> x) noexcept {
        return x.getBuf() >> frac_bits;
    }

    template<typename T, typename TC, unsigned frac_bits>
    constexpr T ceil(const fixedpoint<T, TC, frac_bits> x) noexcept {
        return (x.getBuf() >> frac_bits) + !!x.getfrac();
    }

    template<typename T, typename TC, unsigned frac_bits>
    constexpr T round(const fixedpoint<T, TC, frac_bits> x) noexcept {
        return (x.getBuf() >> frac_bits) + (x.getfrac() >> (frac_bits-1));
    }

    template<typename T, typename TC, unsigned frac_bits>
    constexpr fixedpoint<T, TC, frac_bits> abs(const fixedpoint<T, TC, frac_bits> x) noexcept {
        return x.isNeg() ? -x : x;
    }

    template<typename T, typename TC, unsigned frac_bits>
    constexpr fixedpoint<T, TC, frac_bits> sign(const fixedpoint<T, TC, frac_bits> x) noexcept {
        return x.getBuf() ? (x.isNeg() ? -1 : 1) : 0;
    }

    template<typename T, typename TC, unsigned frac_bits>
    struct hash<fixedpoint<T, TC, frac_bits>> {

        std::size_t operator()(const fixedpoint<T, TC, frac_bits>& fp) const noexcept {
            std::size_t buf = hash_t(fp.getBuf()) ^ ((frac_bits - 1) * (sizeof(T)-1));
            return buf + (buf << frac_bits) + (buf >> frac_bits);
        }

        std::hash<T> hash_t;
    };

    template<typename T, typename TC, unsigned frac_bits>
    struct is_arithmetic<fixedpoint<T, TC, frac_bits>> : public std::true_type {};

    template<typename T, typename TC, unsigned frac_bits>
    struct is_scalar<fixedpoint<T, TC, frac_bits>> : public std::true_type {};

    template<typename T, typename TC, unsigned frac_bits>
    struct is_object<fixedpoint<T, TC, frac_bits>> : public std::true_type {};

    template<typename T, typename TC, unsigned frac_bits>
    struct is_signed<fixedpoint<T, TC, frac_bits>> : public std::is_signed<T> {};

    template<typename T, typename TC, unsigned frac_bits>
    struct is_unsigned<fixedpoint<T, TC, frac_bits>> : public std::is_unsigned<T> {};

    template<typename T, typename TC, unsigned frac_bits>
    struct make_signed<fixedpoint<T, TC, frac_bits>> {
        using type = fixedpoint<typename make_signed<T>::type, typename make_signed<TC>::type, frac_bits>;
    };

    template<typename T, typename TC, unsigned frac_bits>
    struct make_unsigned<fixedpoint<T, TC, frac_bits>> {
        using type = fixedpoint<typename make_unsigned<T>::type, typename make_unsigned<TC>::type, frac_bits>;
    };

    template<typename T, typename TC, unsigned frac_bits>
    struct numeric_limits<fixedpoint<T, TC, frac_bits>> {

        using numeric_limits_t = numeric_limits<T>;
        using fp = fixedpoint<T, TC, frac_bits>;

        static constexpr const auto is_specialized = true;
        static constexpr const auto is_signed = (bool)std::is_signed<T>::value;
        static constexpr const auto is_integer = false;
        static constexpr const auto is_exact = numeric_limits_t::is_exact;
        static constexpr const auto has_infinity = numeric_limits_t::has_infinity;
        static constexpr const auto has_quiet_NaN = numeric_limits_t::has_quiet_NaN;
        static constexpr const auto has_denorm = std::denorm_absent;
        static constexpr const auto has_denorm_loss = false;
        static constexpr const auto round_style = numeric_limits_t::round_style;
        static constexpr const auto is_iec559 = false;
        static constexpr const auto digits = numeric_limits_t::digits;
        static constexpr const auto digits10 = numeric_limits_t::digits10;
        static constexpr const auto radix = 2;
        static constexpr const auto min_exponent = 0;
        static constexpr const auto min_exponent10 = 0;
        static constexpr const auto max_exponent = 0;
        static constexpr const auto max_exponent10 = 0;
        static constexpr const auto traps = numeric_limits_t::traps;
        static constexpr const auto tinyness_before = numeric_limits_t::tinyness_before;

        static constexpr fp min() noexcept {
            return fp(1, true);
        }

        static constexpr fp lowest() noexcept {
            return fp(numeric_limits_t::lowest(), true);
        }

        static constexpr fp max() noexcept {
            return fp(numeric_limits_t::max(), true);
        }

        static constexpr fp epsilon() noexcept {
            return fp(1, true);
        }

        static constexpr fp infinity() noexcept {
            return fp(numeric_limits_t::infinity(), true);
        }

        static constexpr fp quiet_NaN() noexcept {
            return fp(numeric_limits_t::quiet_NaN(), true);
        }

        static constexpr fp signaling_NaN() noexcept {
            return fp(numeric_limits_t::signaling_NaN(), true);
        }

        static constexpr fp denorm_min() noexcept {
            return fp(1, true);
        }

        using float_round_style = std::float_round_style;
        using float_denorm_style = std::float_denorm_style;

    };

}

// standard types
using fixed8 = fixedpoint<std::int8_t>;
using fixed16 = fixedpoint<std::int16_t>;
using fixed32 = fixedpoint<std::int32_t>;
using fixed64 = fixedpoint<std::int64_t>;

using ufixed8 = fixedpoint<std::uint8_t>;
using ufixed16 = fixedpoint<std::uint16_t>;
using ufixed32 = fixedpoint<std::uint32_t>;
using ufixed64 = fixedpoint<std::uint64_t>;

// accurate types - multiplication with larger int - if it is possible
using fixed8_a = fixedpoint<std::int8_t, make_fast_int<std::int16_t>::type>;
using fixed16_a = fixedpoint<std::int16_t, make_fast_int<std::int32_t>::type>;
using fixed32_a = fixedpoint<std::int32_t, make_fast_int<std::int64_t>::type>;
using fixed64_a = fixed64;

using ufixed8_a = fixedpoint<std::uint8_t, make_fast_int<std::uint16_t>::type>;
using ufixed16_a = fixedpoint<std::uint16_t, make_fast_int<std::uint32_t>::type>;
using ufixed32_a = fixedpoint<std::uint32_t, make_fast_int<std::uint64_t>::type>;
using ufixed64_a = ufixed64;

// simple types - multiplication with the same size of int
using fixed8_s = fixedpoint<std::int8_t, std::int8_t>;
using fixed16_s = fixedpoint<std::int16_t, std::int16_t>;
using fixed32_s = fixedpoint<std::int32_t, std::int32_t>;
using fixed64_s = fixedpoint<std::int64_t, std::int64_t>;

using ufixed8_s = fixedpoint<std::uint8_t, std::uint8_t>;
using ufixed16_s = fixedpoint<std::uint16_t, std::uint16_t>;
using ufixed32_s = fixedpoint<std::uint32_t, std::uint32_t>;
using ufixed64_s = fixedpoint<std::uint64_t, std::uint64_t>;


#endif
