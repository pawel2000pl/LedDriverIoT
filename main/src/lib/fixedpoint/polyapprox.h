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
#include <type_traits>
#include <vector>
#include <array>


namespace std {
    template<typename T>
    auto floor(T x);
}


template <typename Storable, unsigned static_part_count = 0>
class PolyApprox {

    public:

        constexpr const static bool is_static = static_part_count > 0;

        PolyApprox() = default;
        PolyApprox(const PolyApprox&) = default;
        PolyApprox(PolyApprox&&) = default;


        template <typename Calculable, bool enable_part_count = !is_static, typename std::enable_if<enable_part_count, void>::type* = nullptr>
        PolyApprox(const std::function<Calculable(Calculable)>& src, unsigned part_count, Calculable range_min, Calculable range_max, Calculable dx=1e-3) {
            __fit<Calculable>(src, part_count, range_min, range_max, dx);
        }


        template <typename Calculable, bool enable_part_count = !is_static, typename std::enable_if<!enable_part_count, void>::type* = nullptr>
        PolyApprox(const std::function<Calculable(Calculable)>& src, Calculable range_min, Calculable range_max, Calculable dx=1e-3) {
            __fit<Calculable>(src, static_part_count, range_min, range_max, dx);
        }


        template <typename Calculable, bool enable_part_count = !is_static, typename std::enable_if<enable_part_count, void>::type* = nullptr>
        static PolyApprox create(const std::function<Calculable(Calculable)>& src, unsigned part_count, Calculable range_min, Calculable range_max, Calculable dx=1e-3) {
            return PolyApprox(src, part_count, range_min, range_max, dx);
        }


        template <typename Calculable, bool enable_part_count = !is_static, typename std::enable_if<!enable_part_count, void>::type* = nullptr>
        static PolyApprox create(const std::function<Calculable(Calculable)>& src, Calculable range_min, Calculable range_max, Calculable dx=1e-3) {
            return PolyApprox(src, range_min, range_max, dx);
        }


        template <typename Calculable, bool enable_part_count = !is_static, typename std::enable_if<enable_part_count, void>::type* = nullptr>
        void fit(const std::function<Calculable(Calculable)>& src, unsigned part_count, Calculable range_min, Calculable range_max, Calculable dx=1e-3) {
            __fit(src, part_count, range_min, range_max, dx);
        }


        template <typename Calculable, bool enable_part_count = !is_static, typename std::enable_if<!enable_part_count, void>::type* = nullptr>
        void fit(const std::function<Calculable(Calculable)>& src, Calculable range_min, Calculable range_max, Calculable dx=1e-3) {
            __fit(src, static_part_count, range_min, range_max, dx);
        }


        Storable call(Storable x) const noexcept {
            unsigned part = std::floor((x - range_min) * inv_incrementator);
            unsigned max_part = coefficients.size();
            if (part >= max_part) part = max_part-1;
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
            Calculable range_max = range_min + Calculable(coefficients.size()) / Calculable(inv_incrementator);
            Calculable result = 0;
            while (x <= range_max) {
                Calculable y1 = call(x);
                Calculable y2 = fun(x);
                Calculable diff = abs<Calculable>(y1 - y2);
                if (diff > result)
                    result = diff;
                x += dx;
            }
            return result;
        }


        PolyApprox& operator=(const PolyApprox&) = default;
        PolyApprox& operator=(PolyApprox&&) = default;


    private:

        using static_coefficients = std::array<std::array<Storable, 5>, static_part_count>;
        using dynamic_coefficients = std::vector<std::array<Storable, 5>>;
        using coefficients_type = typename std::conditional<is_static, static_coefficients, dynamic_coefficients>::type;

        coefficients_type coefficients;
        Storable range_min;
        Storable inv_incrementator;


        template<typename T>
        T abs(T x) {
            return x < 0 ? -x : x;
        }


        template<typename T>
        T max(T a, T b) {
            return a > b ? a : b;
        }

                
        template<typename T>
        T absmax(T a, T b) {
            return max<T>(abs<T>(a), abs<T>(b));
        }


        template<typename T>
        int sign(T x) {
            if (x < 0) return -1;
            if (x > 0) return 1;
            return 0;
        }


        template<typename CT>
        typename std::enable_if<std::is_same<CT, dynamic_coefficients>::value, void>::type 
        resize_coefficients(unsigned new_size) {
            coefficients.resize(new_size);
        }
        

        template<typename CT>
        typename std::enable_if<std::is_same<CT, static_coefficients>::value, void>::type 
        resize_coefficients(unsigned) {}


        template <typename Calculable>
        void __fit(const std::function<Calculable(Calculable)>& src, unsigned part_count, Calculable range_min, Calculable range_max, Calculable dx=1e-3) {
            
            constexpr const Calculable mid_der_k = Calculable(3) / 2;
            Calculable inc = (range_max - range_min) / part_count;
            Calculable half_inc = inc / 2;
            Calculable inv_inc = Calculable(1) / inc;
            Calculable inv_inc2 = inv_inc * inv_inc;
            Calculable inv_inc3 = inv_inc2 * inv_inc;
            Calculable inc_idx = inc / dx;
            Calculable half_inc_idx = inc_idx / 2;
            this->range_min = range_min;
            this->inv_incrementator = inv_inc;
            this->resize_coefficients<coefficients_type>(part_count);

            Calculable prev_value = src(range_min);
            Calculable prev_deriverate = (src(range_min+dx) - prev_value) * inc_idx;
            Calculable prev_x = range_min;

            for (unsigned i=1;i<=part_count;i++) {
                Calculable x = range_min + i * inc;
                Calculable value = src(x);
                Calculable v_mid = src(x - half_inc);
                Calculable new_deriverate = ((i<part_count) ? ((src(x+dx) - src(x-dx)) * half_inc_idx) : ((value - src(x-dx)) * inc_idx));
                Calculable deriverate = new_deriverate;
                Calculable vx = value - prev_value;
                Calculable max_prev_deriverate = 3 * absmax<Calculable>(mid_der_k * (v_mid - prev_value), vx);
                Calculable max_deriverate = 3 * absmax<Calculable>(mid_der_k * (v_mid - value), vx);
                if (abs<Calculable>(prev_deriverate) > max_prev_deriverate)
                    prev_deriverate = max_prev_deriverate*sign<Calculable>(prev_deriverate);
                if (abs<Calculable>(deriverate) > max_deriverate)
                    deriverate = max_deriverate*sign<Calculable>(deriverate);

                auto& coeffs = this->coefficients[i-1];
                coeffs[4] = prev_x;
                //              value           derivate at max     derivate at min
                coeffs[3] = (   (-2) * vx       + deriverate        + prev_deriverate       ) * inv_inc3;
                coeffs[2] = (   3 * vx          - deriverate        - 2 * prev_deriverate   ) * inv_inc2;
                coeffs[1] = (                                       prev_deriverate         ) * inv_inc;
                coeffs[0] =     prev_value;

                prev_x = x;
                prev_value = value;
                prev_deriverate = new_deriverate;
            }

        }

};

#endif
