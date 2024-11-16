#ifndef FIXED_POINT
#define FIXED_POINT

#include <type_traits>

// T - type for data storage
// TB - type used for multiplication and division
// fracBits - count of fraction bits (bits after point)

template<typename T, typename TB, unsigned fracBits>
class FixedPoint {

    public:

        #define FIXED_POINT_INTEGRAL_TEMPLATE template<typename I, typename std::enable_if<std::is_integral<I>::value, I>::type* = nullptr>
        #define FIXED_POINT_FLOAT_TEMPLATE template<typename FP, typename std::enable_if<std::is_floating_point<FP>::value, FP>::type* = nullptr>

        FIXED_POINT_INTEGRAL_TEMPLATE
        FixedPoint(const I value) : FixedPoint(value << fracBits, true) {}

        FIXED_POINT_FLOAT_TEMPLATE
        FixedPoint(const FP value) : FixedPoint(value * (1 << fracBits), true) {}

        FixedPoint() : FixedPoint(0, true) {}

        FixedPoint(const FixedPoint& another) : FixedPoint(another.buf, true) {}

        template<typename T2, typename TB2> 
        FixedPoint(const FixedPoint<T2, TB2, fracBits>& another)
         : buf(another.getBuf()) {}

        template<typename T2, typename TB2, unsigned fracBits2> 
        FixedPoint(const FixedPoint<T2, TB2, fracBits2>& another)
         : buf(another.getBuf() << (int)((int)fracBits - (int)fracBits2)) {}


        friend FixedPoint operator+(const FixedPoint first, const FixedPoint second) {
            return FixedPoint(first.buf + second.buf, true);
        }


        FIXED_POINT_INTEGRAL_TEMPLATE
        friend FixedPoint operator+(const FixedPoint first, const I second) {
            return FixedPoint(first.buf + (second << fracBits), true);
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        friend FixedPoint operator+(const I first, const FixedPoint second) {
            return FixedPoint((first << fracBits) + second.buf, true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        friend FixedPoint operator+(const FixedPoint first, const FP second) {
            return FixedPoint(first.buf + second * (1 << fracBits), true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        friend FixedPoint operator+(const FP first, const FixedPoint second) {
            return FixedPoint(first * (1 << fracBits) + second.buf, true);
        }

        friend FixedPoint operator-(const FixedPoint first, const FixedPoint second) {
            return FixedPoint(first.buf - second.buf, true);
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        friend FixedPoint operator-(const FixedPoint first, const I second)  {
            return FixedPoint(first.buf - (second << fracBits), true);
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        friend FixedPoint operator-(const I first, const FixedPoint second) {
            return FixedPoint((first << fracBits) - second.buf, true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        friend FixedPoint operator-(const FixedPoint first, const FP second) {
            return FixedPoint(first.buf - second * (1 << fracBits), true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        friend FixedPoint operator-(const FP first, const FixedPoint second) {
            return FixedPoint(first * (1 << fracBits) - second.buf, true);
        }

        friend FixedPoint operator*(const FixedPoint first, const FixedPoint second) {
            return FixedPoint(((TB)first.buf * (TB)(second.buf)) >> fracBits, true);
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        friend FixedPoint operator*(const FixedPoint first, const I second) {
            return FixedPoint(first.buf * second, true);
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        friend FixedPoint operator*(const I first, const FixedPoint second) {
            return FixedPoint(first * second.buf, true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        friend FixedPoint operator*(const FixedPoint first, const FP second) {
            return FixedPoint(first.buf * second, true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        friend FixedPoint operator*(const FP first, const FixedPoint second) {
            return FixedPoint(first * second.buf, true);
        }

        friend FixedPoint operator/(const FixedPoint first, const FixedPoint second) {
            return FixedPoint(((TB)first.buf << fracBits) / (TB)second.buf, true);
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        friend FixedPoint operator/(const FixedPoint first, const I second) {
            return FixedPoint(first.buf / second, true);
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        friend FixedPoint operator/(const I first, const FixedPoint second) {
            return FixedPoint(((TB)first << (2*fracBits)) / (TB)second.buf, true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        friend FixedPoint operator/(const FixedPoint first, const FP second) {
            return FixedPoint(((TB)first.buf << fracBits) / (TB)(second * (1 << fracBits)), true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        friend FixedPoint operator/(const FP first, const FixedPoint second) {
            return FixedPoint((TB)(first * (1 << (2*fracBits))) / (TB)second.buf, true);
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        friend FixedPoint operator<<(const FixedPoint first, const I second) {
            return FixedPoint(first.buf << second, true);
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        friend FixedPoint operator>>(const FixedPoint first, const I second) {
            return FixedPoint(first.buf >> second, true);
        }

        #define FixedPointOperatorMaker(oper, type1, param1, type2, param2) \
        FIXED_POINT_TEMPLATE_DECLARATION \
        friend bool operator oper (const type1& first, const type2& second) { \
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
        FixedPointOperatorsMaker(FixedPoint, first.buf, FixedPoint, second.buf)
        #undef FIXED_POINT_TEMPLATE_DECLARATION
        #define FIXED_POINT_TEMPLATE_DECLARATION FIXED_POINT_INTEGRAL_TEMPLATE
        FixedPointOperatorsMaker(FixedPoint, first.buf, I, second << fracBits)
        FixedPointOperatorsMaker(I, first << fracBits, FixedPoint, second.buf)
        #undef FIXED_POINT_TEMPLATE_DECLARATION
        #define FIXED_POINT_TEMPLATE_DECLARATION FIXED_POINT_FLOAT_TEMPLATE
        FixedPointOperatorsMaker(FixedPoint, first.buf, FP, second * (1 << fracBits))
        FixedPointOperatorsMaker(FP, first * (1 << fracBits), FixedPoint, second.buf)
        #undef FIXED_POINT_TEMPLATE_DECLARATION

        #undef FixedPointOperatorsMaker
        #undef FixedPointOperatorMaker

        void operator+=(const FixedPoint another) {
            buf += another.buf;
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        void operator+=(const I another) {
            buf += another << fracBits;
        }

        void operator-=(const FixedPoint another) {
            buf -= another.buf;
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        void operator-=(const I another) {
            buf -= another << fracBits;
        }

        void operator*=(const FixedPoint another) {
            buf = ((TB)buf * (TB)(another.buf)) >> fracBits;
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        void operator*=(const I another) {
            buf *= another;
        }

        void operator/=(const FixedPoint another) {
            buf = ((TB)buf << fracBits) / (TB)(another.buf);
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        void operator/=(const I another) {
            buf /= another;
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        void operator<<=(const I another) {
            buf <<= another;
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        void operator>>=(const I another) {
            buf >>= another;
        }

        FixedPoint operator+() const {
            return FixedPoint(buf, true);
        }

        FixedPoint operator-() const {
            return FixedPoint(-buf, true);
        }

        FIXED_POINT_FLOAT_TEMPLATE
        operator FP() const {
            constexpr FP k = ((FP)1) / ((FP)(1 << fracBits));
            return (FP)buf * k;
        }

        FIXED_POINT_INTEGRAL_TEMPLATE
        operator I() const {
            return buf >> fracBits;
        }

        T getBuf() const {
            return buf;
        }

        bool isNeg() const {
            return buf < 0;
        }

        bool isPos() const {
            return buf > 0;
        }

        bool isZero() const {
            return buf == 0;
        }

        T getfrac() {
            return buf & ((1 << fracBits) - 1);
        }

        template<class Stream>
        friend Stream& operator << (Stream& stream, const FixedPoint fp) {
            stream << (float)fp;
            return stream;
        }

        template<class Stream>
        friend Stream& operator >> (Stream& stream, FixedPoint& fp) {
            float FP;
            stream >> FP;
            fp = FixedPoint(FP);
            return stream;
        }

    static_assert(std::is_signed<T>::value == std::is_signed<TB>::value, "T and TB must be both signed or unsigned.");
    static_assert(sizeof(T) * 8 >= fracBits, "There cannot be more fraction bits that bits in buf type.");
    static_assert(sizeof(T) <= sizeof(TB), "Type for multiplication must be equal or greater than buf type.");

    #undef FIXED_POINT_INTEGRAL_TEMPLATE
    #undef FIXED_POINT_FLOAT_TEMPLATE

    private:
        T buf;

        FixedPoint(const T new_buf, bool) : buf(new_buf) {}

};


namespace std {

    template<typename T, typename TB, unsigned fracBits>
    T floor(const FixedPoint<T, TB, fracBits> x) {
        return x.getBuf() >> fracBits;
    }


    template<typename T, typename TB, unsigned fracBits>
    T ceil(const FixedPoint<T, TB, fracBits> x) {
        return (x.getBuf() >> fracBits) + !!x.getfrac();
    }


    template<typename T, typename TB, unsigned fracBits>
    T round(const FixedPoint<T, TB, fracBits> x) {
        return (x.getBuf() >> fracBits) + (x.getfrac() >> (fracBits-1));
    }


    template<typename T, typename TB, unsigned fracBits>
    FixedPoint<T, TB, fracBits> abs(const FixedPoint<T, TB, fracBits> x) {
        return x.isNeg() ? -x : x;
    }


    template<typename T, typename TB, unsigned fracBits>
    FixedPoint<T, TB, fracBits> sign(const FixedPoint<T, TB, fracBits> x) {
        return x.getBuf() ? (x.isNeg() ? -1 : 1) : 0;
    }
}

#endif
