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

#ifndef POLY_APPROX
#define POLY_APPROX

#include <functional>
#include <vector>
#include <array>


namespace std {
    template<typename T>
    auto floor(T x);
}


template <typename Storable>
class PolyApprox {

    public:

        PolyApprox() = default;
        PolyApprox(const PolyApprox&) = default;
        PolyApprox(PolyApprox&&) = default;

        template <typename Calculable>
        PolyApprox(const std::function<Calculable(Calculable)>& src, unsigned part_count, Calculable range_min, Calculable range_max, Calculable dx=1e-3) {
            fit<Calculable>(src, part_count, range_min, range_max, dx);
        }


        template <typename Calculable>
        static PolyApprox create(const std::function<Calculable(Calculable)>& src, unsigned part_count, Calculable range_min, Calculable range_max, Calculable dx=1e-3) {
            return PolyApprox(src, part_count, range_min, range_max, dx);
        }


        template <typename Calculable>
        void fit(const std::function<Calculable(Calculable)>& src, unsigned part_count, Calculable range_min, Calculable range_max, Calculable dx=1e-3) {
            coefficients.resize(part_count);

            this->range_min = range_min;
            this->range_max = range_max;
            Calculable inc = (range_max - range_min) / part_count;
            Calculable inc2 = inc*inc;
            Calculable inc3 = inc2*inc;
            inv_incrementator = Calculable(1)/inc;

            coefficients.resize(part_count);

            Calculable prev_value = src(range_min);
            Calculable prev_deriverate = (src(range_min+dx) - prev_value) * inc / dx;
            Calculable prev_x = range_min;

            for (unsigned i=1;i<=part_count;i++) {
                Calculable x = range_min + i * inc;
                Calculable value = src(x);
                Calculable new_deriverate = ((i<part_count) ? ((src(x+dx) - src(x-dx)) / (2*dx)) : ((value - src(x-dx)) / dx)) * inc;
                Calculable deriverate = new_deriverate;
                Calculable vx = value - prev_value;
                Calculable max_deriverate = 3 * abs<Calculable>(vx);
                if (abs<Calculable>(prev_deriverate) > max_deriverate)
                    prev_deriverate = max_deriverate*sign<Calculable>(prev_deriverate);
                if (abs<Calculable>(deriverate) > max_deriverate)
                    deriverate = max_deriverate*sign<Calculable>(deriverate);

                auto& coeffs = coefficients[i-1];
                coeffs[4] = prev_x;
                //              value           derivate at max     derivate at min
                coeffs[3] = (   (-2) * vx       + deriverate        + prev_deriverate       ) / inc3;
                coeffs[2] = (   3 * vx          - deriverate        - 2 * prev_deriverate   ) / inc2;
                coeffs[1] = (                                       prev_deriverate         ) / inc;
                coeffs[0] =     prev_value;

                prev_x = x;
                prev_value = value;
                prev_deriverate = new_deriverate;
            }

        }


        Storable call(Storable x) const noexcept {
            unsigned part = std::floor((x - range_min) * inv_incrementator);
            if (part >= coefficients.size()) part--;
            const auto& coeff = coefficients[part];
            x -= coeff[4];
            return ((coeff[3] * x + coeff[2]) * x + coeff[1]) * x + coeff[0];
        }


        Storable operator()(Storable x) const noexcept {
            return call(x);
        }


        template <typename Calculable>
        Calculable getMaxError(const std::function<Calculable(Calculable)>& fun, Calculable dx=1e-6) const {
            Calculable x = range_min;
            Calculable result = 0;
            while (x <= range_max) {
                Calculable y1 = call(x);
                Calculable y2 = fun(x);
                Calculable diff = (y1 > y2) ? (y1 - y2) : (y2 - y1);
                if (diff > result)
                    result = diff;
                x += dx;
            }
            return result;
        }


        PolyApprox& operator=(const PolyApprox&) = default;
        PolyApprox& operator=(PolyApprox&&) = default;


    public:
        std::vector<std::array<Storable, 5>> coefficients;
        Storable range_min;
        Storable range_max;
        Storable inv_incrementator;

        template<typename T>
        T abs(T x) {
            return x < 0 ? -x : x;
        }

        template<typename T>
        int sign(T x) {
            if (x < 0) return -1;
            if (x > 0) return 1;
            return 0;
        }

};

#endif
