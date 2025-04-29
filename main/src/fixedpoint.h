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

#include <cstddef>
#include <limits>
#include <typeindex>
#include <type_traits>

// T - type for data storage
// TB - type used for multiplication and division
// fracBits - count of fraction bits (bits after point)
// simpleMultiplication - multiplication with the T type instad of the TB

template<typename T, typename TB, unsigned fracBits, bool simpleMultiplication=(sizeof(T)==sizeof(TB))>
class fixedpoint {

    public:

        #define FIXED_POINT_INTEGRAL_TEMPLATE template<typename I, typename std::enable_if<std::is_integral<I>::value, I>::type* = nullptr>
        #define FIXED_POINT_FLOAT_TEMPLATE template<typename FP, typename std::enable_if<std::is_floating_point<FP>::value, FP>::type* = nullptr>
        #define FIXED_POINT_ARITHMETIC_TEMPLATE template<typename A, typename std::enable_if<std::is_arithmetic<A>::value, A>::type* = nullptr>

        FIXED_POINT_INTEGRAL_TEMPLATE
        constexpr fixedpoint(const I value) noexcept : buf((T)value << fracBits) {}

        FIXED_POINT_FLOAT_TEMPLATE
        constexpr fixedpoint(const FP value) noexcept : buf(value * ((T)1 << fracBits)) {}

        constexpr fixedpoint() noexcept : buf(0) {}

        constexpr fixedpoint(const fixedpoint& another) noexcept = default;

        constexpr fixedpoint(fixedpoint&& another) noexcept = default;

        ~fixedpoint() noexcept = default;

        template<typename T2, typename TB2, bool simpleMultiplication2>
        constexpr fixedpoint(const fixedpoint<T2, TB2, fracBits, simpleMultiplication2>& another) noexcept
         : buf(another.buf) {}

        template<typename T2, typename TB2, unsigned fracBits2, bool simpleMultiplication2>
        constexpr fixedpoint(const fixedpoint<T2, TB2, fracBits2, simpleMultiplication2>& another) noexcept
         : buf(another.buf << (int)((int)fracBits - (int)fracBits2)) {}

        constexpr static fixedpoint buf_cast(const T buf) noexcept {
            return fixedpoint(buf, true);
        }

        constexpr friend fixedpoint operator+(const fixedpoint first, const fixedpoint second) noexcept {
            return fixedpoint(first.buf + second.buf, true);
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        constexpr friend fixedpoint operator+(const fixedpoint first, const I second) noexcept {
            return fixedpoint(first.buf + ((T)second << fracBits), true);
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        constexpr friend fixedpoint operator+(const I first, const fixedpoint second) noexcept {
            return fixedpoint(((T)first << fracBits) + second.buf, true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        constexpr friend fixedpoint operator+(const fixedpoint first, const FP second) noexcept {
            return fixedpoint(first.buf + second * ((T)1 << fracBits), true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        constexpr friend fixedpoint operator+(const FP first, const fixedpoint second) noexcept {
            return fixedpoint(first * ((T)1 << fracBits) + second.buf, true);
        }

        constexpr friend fixedpoint operator-(const fixedpoint first, const fixedpoint second) noexcept {
            return fixedpoint(first.buf - second.buf, true);
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        constexpr friend fixedpoint operator-(const fixedpoint first, const I second) noexcept  {
            return fixedpoint(first.buf - ((T)second << fracBits), true);
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        constexpr friend fixedpoint operator-(const I first, const fixedpoint second) noexcept {
            return fixedpoint(((T)first << fracBits) - second.buf, true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        constexpr friend fixedpoint operator-(const fixedpoint first, const FP second) noexcept {
            return fixedpoint(first.buf - second * ((T)1 << fracBits), true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        constexpr friend fixedpoint operator-(const FP first, const fixedpoint second) noexcept {
            return fixedpoint(first * ((T)1 << fracBits) - second.buf, true);
        }

        template <bool SM = simpleMultiplication>
        constexpr friend typename std::enable_if<SM, fixedpoint>::type
        operator*(const fixedpoint first, const fixedpoint second) noexcept {
            return fixedpoint((first.buf >> (fracBits - (fracBits >> 1))) * (second.buf >> (fracBits >> 1)), true);
        }

        template <bool SM = simpleMultiplication>
        constexpr friend typename std::enable_if<!SM, fixedpoint>::type
        operator*(const fixedpoint first, const fixedpoint second) noexcept {
            return fixedpoint(((TB)first.buf * (TB)(second.buf)) >> fracBits, true);
        }

        FIXED_POINT_ARITHMETIC_TEMPLATE
        constexpr friend fixedpoint operator*(const fixedpoint first, const A second) noexcept {
            return fixedpoint(first.buf * second, true);
        }

        FIXED_POINT_ARITHMETIC_TEMPLATE
        constexpr friend fixedpoint operator*(const A first, const fixedpoint second) noexcept {
            return fixedpoint(first * second.buf, true);
        }

        constexpr friend fixedpoint operator/(const fixedpoint first, const fixedpoint second) noexcept {
            return fixedpoint(((TB)first.buf << fracBits) / (TB)second.buf, true);
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        friend fixedpoint operator/(const fixedpoint first, const I second) noexcept {
            return fixedpoint(first.buf / second, true);
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        constexpr friend fixedpoint operator/(const I first, const fixedpoint second) noexcept {
            return fixedpoint(((TB)first << (2*fracBits)) / (TB)second.buf, true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        constexpr friend fixedpoint operator/(const fixedpoint first, const FP second) noexcept {
            return fixedpoint(((TB)first.buf << fracBits) / (TB)(second * ((T)1 << fracBits)), true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        constexpr friend fixedpoint operator/(const FP first, const fixedpoint second) noexcept {
            return fixedpoint((TB)(first * ((T)1 << (2*fracBits))) / (TB)second.buf, true);
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        constexpr friend fixedpoint operator<<(const fixedpoint first, const I second) noexcept {
            return fixedpoint(first.buf << second, true);
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
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
        #define FIXED_POINT_TEMPLATE_DECLARATION FIXED_POINT_INTEGRAL_TEMPLATE
        FixedPointOperatorsMaker(fixedpoint, first.buf, I, (T)second << fracBits)
        FixedPointOperatorsMaker(I, first << fracBits, fixedpoint, second.buf)
        #undef FIXED_POINT_TEMPLATE_DECLARATION
        #define FIXED_POINT_TEMPLATE_DECLARATION FIXED_POINT_FLOAT_TEMPLATE
        FixedPointOperatorsMaker(fixedpoint, first.buf, FP, second * ((T)1 << fracBits))
        FixedPointOperatorsMaker(FP, first * ((T)1 << fracBits), fixedpoint, second.buf)
        #undef FIXED_POINT_TEMPLATE_DECLARATION

        #undef FixedPointOperatorsMaker
        #undef FixedPointOperatorMaker

        constexpr fixedpoint& operator=(const fixedpoint&) noexcept = default;

        constexpr fixedpoint& operator=(fixedpoint&& other) noexcept = default;

        void operator+=(const fixedpoint another) noexcept {
            buf += another.buf;
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        void operator+=(const I another) noexcept {
            buf += (T)another << fracBits;
        }

        void operator-=(const fixedpoint another) noexcept {
            buf -= another.buf;
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        void operator-=(const I another) noexcept {
            buf -= (T)another << fracBits;
        }

        template <bool SM = simpleMultiplication>
        typename std::enable_if<SM, void>::type
        operator*=(const fixedpoint another) noexcept {
            buf = (buf >> (fracBits - (fracBits >> 1))) * (another.buf >> (fracBits >> 1));
        }

        template <bool SM = simpleMultiplication>
        typename std::enable_if<!SM, void>::type
        operator*=(const fixedpoint another) noexcept {
            buf = ((TB)buf * (TB)(another.buf)) >> fracBits;
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        void operator*=(const I another) {
            buf *= another;
        }

        void operator/=(const fixedpoint another) noexcept {
            buf = ((TB)buf << fracBits) / (TB)(another.buf);
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        void operator/=(const I another) noexcept {
            buf /= another;
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        void operator<<=(const I another) noexcept {
            buf <<= another;
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
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
            constexpr FP k = ((FP)1) / ((FP)((T)1 << fracBits));
            return (FP)buf * k;
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        constexpr operator I() const noexcept {
            return buf >> fracBits;
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
            return buf & (((T)1 << fracBits) - 1);
        }

        constexpr operator bool() noexcept {
            return (bool)buf;
        }

        unsigned toCharBuf(char* buffer, unsigned char base=10) const {
            char* wbuf = buffer;
            T tmpBuf = buf;
            if (tmpBuf < 0) {
                *(wbuf++) = '-';
                tmpBuf = -tmpBuf;
            }
            T intPart = tmpBuf >> fracBits;
            T fracPart = tmpBuf - (intPart << fracBits);
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
                while (fracPart) {
                    fracPart *= 10;
                    T digit = fracPart >> fracBits;
                    fracPart -= digit << fracBits;
                    *(wbuf++) = num2chr(digit);
                }
            }
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

        static const unsigned stream_buf_size = 64;

        template<class Stream>
        friend Stream& operator << (Stream& stream, const fixedpoint fp) {
            char buf[stream_buf_size];
            unsigned size = fp.toCharBuf(buf);
            buf[size] = 0;
            stream << buf;
            return stream;
        }

        template<class Stream>
        friend Stream& operator >> (Stream& stream, fixedpoint& fp) {
            char buf[stream_buf_size];
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

        friend struct std::numeric_limits<fixedpoint<T, TB, fracBits>>;

        template<typename T2, typename TB2, unsigned fracBits2, bool simpleMultiplication2>
        friend class fixedpoint;

        static_assert(std::is_arithmetic<T>::value, "Type for the buf must be arithmetic.");
        static_assert(std::is_arithmetic<TB>::value, "Type helper for multiplication must be arithmetic.");
        static_assert(std::numeric_limits<T>::radix == 2, "Type for the buf must be binary (radix == 2).");
        static_assert(std::numeric_limits<TB>::radix == 2, "Type helper for multiplication must be binary (radix == 2).");
        static_assert(std::numeric_limits<T>::is_integer, "Type for the buf must be an integer type.");
        static_assert(std::numeric_limits<TB>::is_integer, "Type for multiplication must be an integer type.");
        static_assert(std::is_signed<T>::value == std::is_signed<TB>::value, "T and TB must be both signed or unsigned.");
        static_assert(sizeof(T) * 8 - std::is_signed<T>::value > fracBits, "There must be less fraction bits than number of bits in the buf type.");
        static_assert(sizeof(T) <= sizeof(TB), "Type for multiplication must be equal or greater than buf type.");

        #undef FIXED_POINT_INTEGRAL_TEMPLATE
        #undef FIXED_POINT_FLOAT_TEMPLATE
        #undef FIXED_POINT_ARITHMETIC_TEMPLATE

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

    template<typename T, typename TB, unsigned fracBits>
    constexpr T floor(const fixedpoint<T, TB, fracBits> x) noexcept {
        return x.getBuf() >> fracBits;
    }

    template<typename T, typename TB, unsigned fracBits>
    constexpr T ceil(const fixedpoint<T, TB, fracBits> x) noexcept {
        return (x.getBuf() >> fracBits) + !!x.getfrac();
    }

    template<typename T, typename TB, unsigned fracBits>
    constexpr T round(const fixedpoint<T, TB, fracBits> x) noexcept {
        return (x.getBuf() >> fracBits) + (x.getfrac() >> (fracBits-1));
    }

    template<typename T, typename TB, unsigned fracBits>
    constexpr fixedpoint<T, TB, fracBits> abs(const fixedpoint<T, TB, fracBits> x) noexcept {
        return x.isNeg() ? -x : x;
    }

    template<typename T, typename TB, unsigned fracBits>
    constexpr fixedpoint<T, TB, fracBits> sign(const fixedpoint<T, TB, fracBits> x) noexcept {
        return x.getBuf() ? (x.isNeg() ? -1 : 1) : 0;
    }

    template<typename T, typename TB, unsigned fracBits>
    struct hash<fixedpoint<T, TB, fracBits>> {

        std::size_t operator()(const fixedpoint<T, TB, fracBits>& fp) const noexcept {
            std::size_t buf = hash_t(fp.getBuf()) ^ ((fracBits - 1) * (sizeof(T)-1));
            return buf + (buf << fracBits) + (buf >> fracBits);
        }

        std::hash<T> hash_t;
    };

    template<typename T, typename TB, unsigned fracBits>
    struct is_arithmetic<fixedpoint<T, TB, fracBits>> : public std::true_type {};

    template<typename T, typename TB, unsigned fracBits>
    struct is_scalar<fixedpoint<T, TB, fracBits>> : public std::true_type {};

    template<typename T, typename TB, unsigned fracBits>
    struct is_object<fixedpoint<T, TB, fracBits>> : public std::true_type {};

    template<typename T, typename TB, unsigned fracBits>
    struct is_signed<fixedpoint<T, TB, fracBits>> : public std::is_signed<T> {};

    template<typename T, typename TB, unsigned fracBits>
    struct is_unsigned<fixedpoint<T, TB, fracBits>> : public std::is_unsigned<T> {};

    template<typename T, typename TB, unsigned fracBits>
    struct make_signed<fixedpoint<T, TB, fracBits>> {
        fixedpoint<make_signed<T>, make_signed<TB>, fracBits> type;
    };

    template<typename T, typename TB, unsigned fracBits>
    struct make_unsigned<fixedpoint<T, TB, fracBits>> {
        fixedpoint<make_unsigned<T>, make_unsigned<TB>, fracBits> type;
    };

    template<typename T, typename TB, unsigned fracBits>
    struct numeric_limits<fixedpoint<T, TB, fracBits>> {

        using numeric_limits_t = numeric_limits<T>;
        using fp = numeric_limits<fixedpoint<T, TB, fracBits>>;

        static const auto is_specialized = true;
        static const auto is_signed = (bool)std::is_signed<T>::value;
        static const auto is_integer = false;
        static const auto is_exact = numeric_limits_t::is_exact;
        static const auto has_infinity = numeric_limits_t::has_infinity;
        static const auto has_quiet_NaN = numeric_limits_t::has_quiet_NaN;
        static const auto has_denorm = std::denorm_absent;
        static const auto has_denorm_loss = false;
        static const auto round_style = numeric_limits_t::round_toward_zero;
        static const auto is_iec559 = false;
        static const auto digits = numeric_limits_t::digits;
        static const auto digits10 = numeric_limits_t::digits10;
        static const auto radix = 2;
        static const auto min_exponent = 0;
        static const auto min_exponent10 = 0;
        static const auto max_exponent = 0;
        static const auto max_exponent10 = 0;
        static const auto traps = numeric_limits_t::traps;
        static const auto tinyness_before = numeric_limits_t::tinyness_before;

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

        using float_round_style = typename numeric_limits_t::float_round_style;
        using float_denorm_style = typename numeric_limits_t::float_denorm_style;

    };

}

#endif
