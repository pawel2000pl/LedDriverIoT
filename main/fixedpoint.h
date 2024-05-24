#pragma once

// T - signed type for data storage
// TB - signed type used for multiplication and division
// fraqBits - count of fraction bits (bits after point)

template<typename T, typename TB, unsigned fraqBits>
class FixedPoint {

    public:

        FixedPoint(const int value) : FixedPoint(value << fraqBits, true) {}
        FixedPoint(const unsigned value) : FixedPoint(value << fraqBits, true) {}
        FixedPoint(const float value) : FixedPoint(value * (1 << fraqBits), true) {}
        FixedPoint(const double value) : FixedPoint(value * (1 << fraqBits), true) {}
        FixedPoint() : FixedPoint(0) {}
        FixedPoint(const FixedPoint& another) : FixedPoint(another.buf, true) {}
        template<typename T2, typename TB2> 
        FixedPoint(const FixedPoint<T2, TB2, fraqBits>& another)
         : buf(another.getBuf()) {}
        template<typename T2, typename TB2, unsigned fraqBits2> 
        FixedPoint(const FixedPoint<T2, TB2, fraqBits2>& another)
         : buf(another.getBuf() << (int)((int)fraqBits) - (int)fraqBits2) {}


        friend FixedPoint operator+(const FixedPoint first, const FixedPoint second) {
            return FixedPoint(first.buf + second.buf, true);
        }

        friend FixedPoint operator+(const FixedPoint first, const int second) {
            return FixedPoint(first.buf + (second << fraqBits), true);
        }

        friend FixedPoint operator+(const int first, const FixedPoint second) {
            return FixedPoint((first << fraqBits) + second.buf, true);
        }

        friend FixedPoint operator+(const FixedPoint first, const float second) {
            return FixedPoint(first.buf + second * (1 << fraqBits), true);
        }

        friend FixedPoint operator+(const float first, const FixedPoint second) {
            return FixedPoint(first * (1 << fraqBits) + second.buf, true);
        }

        friend FixedPoint operator-(const FixedPoint first, const FixedPoint second) {
            return FixedPoint(first.buf - second.buf, true);
        }

        friend FixedPoint operator-(const FixedPoint first, const int second)  {
            return FixedPoint(first.buf - (second << fraqBits), true);
        }

        friend FixedPoint operator-(const int first, const FixedPoint second) {
            return FixedPoint((first << fraqBits) - second.buf, true);
        }

        friend FixedPoint operator-(const FixedPoint first, const float second) {
            return FixedPoint(first.buf - second * (1 << fraqBits), true);
        }

        friend FixedPoint operator-(const float first, const FixedPoint second) {
            return FixedPoint(first * (1 << fraqBits) - second.buf, true);
        }

        friend FixedPoint operator*(const FixedPoint first, const FixedPoint second) {
            return FixedPoint(((TB)first.buf * (TB)(second.buf)) >> fraqBits, true);
        }

        friend FixedPoint operator*(const FixedPoint first, const int second) {
            return FixedPoint(first.buf * second, true);
        }

        friend FixedPoint operator*(const int first, const FixedPoint second) {
            return FixedPoint(first * second.buf, true);
        }

        friend FixedPoint operator*(const FixedPoint first, const float second) {
            return FixedPoint(first.buf * second, true);
        }

        friend FixedPoint operator*(const float first, const FixedPoint second) {
            return FixedPoint(first * second.buf, true);
        }

        friend FixedPoint operator/(const FixedPoint first, const FixedPoint second) {
            return FixedPoint(((TB)first.buf << fraqBits) / (TB)second.buf, true);
        }

        friend FixedPoint operator/(const FixedPoint first, const int second) {
            return FixedPoint(first.buf / second, true);
        }

        friend FixedPoint operator/(const int first, const FixedPoint second) {
            return FixedPoint(((TB)first << (2*fraqBits)) / (TB)second.buf, true);
        }

        friend FixedPoint operator/(const FixedPoint first, const float second) {
            return FixedPoint(((TB)first.buf << fraqBits) / (TB)(second * (1 << fraqBits)), true);
        }

        friend FixedPoint operator/(const float first, const FixedPoint second) {
            return FixedPoint((TB)(first * (1 << (2*fraqBits))) / (TB)second.buf, true);
        }

        #define FixedPointOperatorMaker(oper, type1, param1, type2, param2) \
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

        FixedPointOperatorsMaker(FixedPoint, first.buf, FixedPoint, second.buf)
        FixedPointOperatorsMaker(FixedPoint, first.buf, int, second << fraqBits)
        FixedPointOperatorsMaker(int, first << fraqBits, FixedPoint, second.buf)
        FixedPointOperatorsMaker(FixedPoint, first.buf, float, second * (1 << fraqBits))
        FixedPointOperatorsMaker(float, first * (1 << fraqBits), FixedPoint, second.buf)

        #undef FixedPointOperatorsMaker
        #undef FixedPointOperatorMaker

        void operator+=(const FixedPoint another) {
            buf += another.buf;
        }

        void operator-=(const FixedPoint another) {
            buf -= another.buf;
        }

        void operator*=(const FixedPoint another) {
            buf = ((TB)buf * (TB)(another.buf)) >> fraqBits;
        }

        void operator/=(const FixedPoint another) {
            buf = ((TB)buf << fraqBits) / (TB)(another.buf);
        }

        FixedPoint operator+() const {
            return FixedPoint(buf, true);
        }

        FixedPoint operator-() const {
            return FixedPoint(-buf, true);
        }

        operator float() const {
            return (float)buf / (1 << fraqBits);
        }

        operator int() const {
            return buf >> fraqBits;
        }

        unsigned getBuf() const {
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

        static unsigned getFraqBits() {
            return fraqBits;
        }

        template<class Stream>
        friend Stream& operator << (Stream& stream, const FixedPoint fp) {
            stream << (float)fp;
            return stream;
        }

        template<class Stream>
        friend Stream& operator >> (Stream& stream, FixedPoint& fp) {
            float f;
            stream >> f;
            fp = FixedPoint(f);
            return stream;
        }

    private:
        T buf;

        FixedPoint(const T new_buf, bool) : buf(new_buf) {}

};

namespace std {
    template<typename T, typename TB, unsigned fraqBits>
        FixedPoint<T, TB, fraqBits> abs(const FixedPoint<T, TB, fraqBits> x) {
            return x.isNeg() ? -x : x;
        }
}
