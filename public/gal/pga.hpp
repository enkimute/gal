#pragma once

#include "engine.hpp"
#include "entity.hpp"
#include "geometric_algebra.hpp"

#include <cmath>

// The Projective Geometric Algebra for representing Euclidean 3-space
// NOTE: In comments, we often write "the PGA" to mean literally "the projective geometric algebra."

namespace gal
{
namespace pga
{
    // NOTE: the inner product of e0 can be set to +1 or -1 without any change in the algebra's geometric
    // interpretation. Here, we opt to define e0^2 := 1 by convention
    using pga_metric = ::gal::metric<3, 0, 1>;

    // The PGA is a graded algebra with 16 basis elements
    using pga_algebra = gal::algebra<pga_metric>;

    constexpr inline auto e = gal::e<pga_algebra, 0>;
    constexpr inline auto e0 = gal::e<pga_algebra, 0b1>;
    constexpr inline auto e1 = gal::e<pga_algebra, 0b10>;
    constexpr inline auto e2 = gal::e<pga_algebra, 0b100>;
    constexpr inline auto e3 = gal::e<pga_algebra, 0b1000>;
    constexpr inline auto e01 = gal::e<pga_algebra, 0b11>;
    constexpr inline auto e02 = gal::e<pga_algebra, 0b101>;
    constexpr inline auto e03 = gal::e<pga_algebra, 0b1001>;
    constexpr inline auto e12 = gal::e<pga_algebra, 0b110>;
    constexpr inline auto e13 = gal::e<pga_algebra, 0b1010>;
    constexpr inline auto e23 = gal::e<pga_algebra, 0b1100>;
    constexpr inline auto e012 = gal::e<pga_algebra, 0b111>;
    constexpr inline auto e013 = gal::e<pga_algebra, 0b1011>;
    constexpr inline auto e023 = gal::e<pga_algebra, 0b1101>;
    constexpr inline auto e123 = gal::e<pga_algebra, 0b1110>;
    constexpr inline auto e1234 = gal::e<pga_algebra, 0b1111>;
}

template <typename T>
struct expr<expr_op::identity, mv<pga::pga_algebra, 0, 1, 1>, detail::pseudoscalar_tag<T>>
{
    using value_t               = T;
    using algebra_t             = pga::pga_algebra;
    constexpr static expr_op op = expr_op::identity;
    constexpr static auto lhs   = pga::pga_algebra::pseudoscalar;
};

template <typename T>
struct expr<expr_op::identity, mv<pga::pga_algebra, 0, 1, 1>, detail::pseudoscalar_inv_tag<T>>
{
    using value_t               = T;
    using algebra_t             = pga::pga_algebra;
    constexpr static expr_op op = expr_op::identity;
    constexpr static auto lhs   = pga::pga_algebra::pseudoscalar_inv;
};

namespace pga {
    template <typename T = float>
    constexpr inline expr<expr_op::identity, mv<pga_algebra, 0, 1, 1>, ::gal::detail::pseudoscalar_tag<T>> ps;

    template <typename T = float>
    constexpr inline expr<expr_op::identity, mv<pga_algebra, 0, 1, 1>, ::gal::detail::pseudoscalar_inv_tag<T>> ips;

    // TODO: It isn't great that we cache the cos and sin of the rotor since storing 5 elements prevents a more natural
    // tightly-packed alignment
    template <typename T = float>
    union rotor
    {
        using algebra_t = pga_algebra;
        using value_t = T;

        [[nodiscard]] constexpr static size_t size() noexcept
        {
            return 5;
        }

        [[nodiscard]] constexpr static uint32_t ind_count() noexcept
        {
            return 5;
        }

        std::array<T, 5> data;
        struct
        {
            T cos_theta;
            T sin_theta;
            T x;
            T y;
            T z;
        };

        constexpr rotor(T theta, T x, T y, T z) noexcept
            : cos_theta{std::cos(T{0.5} * theta)}
            , sin_theta{std::sin(T{0.5} * theta)}
            , x{x}
            , y{y}
            , z{z}
        {}

        // As always when doing any normalization operation, NaNs are producible when normalizing
        // vectors of zero length. This is not checked for!
        void normalize() noexcept
        {
            auto l2_inv = T{1} / std::sqrt(x * x + y * y + z * z);
            x           = x * l2_inv;
            y           = y * l2_inv;
            z           = z * l2_inv;
        }

        // Cos\theta := ID 0
        // Sin\theta := ID 1
        // x := ID 2
        // y := ID 3
        // z := ID 4
        // A rotation t around a line is given by the expression cos(t/2) + sin(t/2)(l_x + l_y + l_z)
        [[nodiscard]] constexpr static mv<pga_algebra, 8, 4, 4> ie(uint32_t id) noexcept
        {
            return {mv_size{7, 4, 4},
                    {ind{id, one},     // cos(t/2)
                     ind{id + 1, one}, // z * sin(t/2)
                     ind{id + 4, one},
                     ind{id + 1, one}, // -y * sin(t/2)
                     ind{id + 3, one},
                     ind{id + 1, one}, // x * sin(t/2)
                     ind{id + 2, one}},
                    {mon{one, one, 1, 0}, mon{one, one, 1, 1}, mon{minus_one, rat{2}, 2, 2}, mon{one, rat{2}, 2, 3}},
                    {term{1, 0, 0b0}, term{1, 1, 0b110}, term{1, 2, 0b1010}, term{1, 3, 0b1100}}};
        }

        [[nodiscard]] constexpr T const& operator[](size_t index) const noexcept
        {
            return data[index];
        }

        [[nodiscard]] constexpr T& operator[](size_t index) noexcept
        {
            return data[index];
        }

        [[nodiscard]] constexpr T get(size_t i) const noexcept
        {
            return NAN;
        }
    };

    template <typename T = float>
    union translator
    {
        using algebra_t = pga_algebra;
        using value_t = T;

        [[nodiscard]] constexpr static size_t size() noexcept
        {
            return 4;
        }

        [[nodiscard]] constexpr static uint32_t ind_count() noexcept
        {
            return 4;
        }

        std::array<T, 4> data;
        struct
        {
            T d;
            T x;
            T y;
            T z;
        };

        constexpr translator(T d, T x, T y, T z) noexcept
            : d{d}
            , x{x}
            , y{y}
            , z{z}
        {}

        // As always when doing any normalization operation, NaNs are producible when normalizing
        // vectors of zero length. This is not checked for!
        void normalize() noexcept
        {
            auto l2_inv = T{1} / std::sqrt(x * x + y * y + z * z);
            x           = x * l2_inv;
            y           = y * l2_inv;
            z           = z * l2_inv;
        }

        // A translation along a line with distance d is given by the expression 1 + d/2(P_inf)
        [[nodiscard]] constexpr static mv<pga_algebra, 6, 4, 4> ie(uint32_t id) noexcept
        {
            return {mv_size{6, 4, 4},
                    {ind{id, one}, // d * l_x
                     ind{id + 1, one},
                     ind{id, one}, // d * l_y
                     ind{id + 1, one},
                     ind{id, one}, // d * l_z
                     ind{id + 2, one}},
                    {
                        mon{one, zero, 0, 0},            // 1
                        mon{minus_one_half, rat{2}, 2, 1}, // -1/2 * l_x
                        mon{minus_one_half, rat{2}, 2, 2}, // -1/2 * l_y
                        mon{minus_one_half, rat{2}, 2, 3}  // -1/2 * l_z
                    },
                    {term{1, 0, 0b0}, term{1, 1, 0b11}, term{1, 2, 0b101}, term{1, 3, 0b1001}}};
        }

        [[nodiscard]] constexpr T const& operator[](size_t index) const noexcept
        {
            return data[index];
        }

        [[nodiscard]] constexpr T& operator[](size_t index) noexcept
        {
            return data[index];
        }

        [[nodiscard]] constexpr T get(size_t i) const noexcept
        {
            return NAN;
        }
    };

    // A motor occupies the even subalgebra
    template <typename T = float>
    using motor = entity<pga_algebra, T, 0, 0b11, 0b101, 0b110, 0b1001, 0b1010, 0b1100, 0b1111>;

    template <typename T = float>
    union plane
    {
        using algebra_t = pga_algebra;
        using value_t   = T;
        constexpr static bool is_dual = true;

        std::array<T, 4> data;
        struct
        {
            T d;
            T x;
            T y;
            T z;
        };

        [[nodiscard]] constexpr static auto ie(uint32_t id) noexcept
        {
            return detail::construct_ie<algebra_t>(
                id, std::make_integer_sequence<width_t, 4>{}, std::integer_sequence<uint8_t, 0b1, 0b10, 0b100, 0b1000>{});
        }

        [[nodiscard]] constexpr static size_t size() noexcept
        {
            return 4;
        }

        [[nodiscard]] constexpr static uint32_t ind_count() noexcept
        {
            return 4;
        }

        constexpr plane(T d, T x, T y, T z) noexcept
            : d{d}
            , x{x}
            , y{y}
            , z{z}
        {}

        template <uint8_t... E>
        constexpr plane(entity<pga_algebra, T, E...> in) noexcept
            : data{in.template select<0b1, 0b10, 0b100, 0b1000>()}
        {
        }

        [[nodiscard]] constexpr T const& operator[](size_t index) const noexcept
        {
            return data[index];
        }

        [[nodiscard]] constexpr T& operator[](size_t index) noexcept
        {
            return data[index];
        }

        [[nodiscard]] constexpr T get(size_t i) const noexcept
        {
            // Unused
            return NAN;
        }
    };

    template <typename T = float>
    union point
    {
        using algebra_t = pga_algebra;
        using value_t   = T;
        constexpr static bool is_dual = true;

        std::array<T, 3> data;
        struct
        {
            T x;
            T y;
            T z;
        };

        // Like planes, points are represented dually as the intersection of three planes
        [[nodiscard]] constexpr static mv<algebra_t, 3, 4, 4> ie(uint32_t id) noexcept
        {
            return {mv_size{3, 4, 4},
                    {
                        ind{id + 2, one}, // -z
                        ind{id + 1, one}, // y
                        ind{id, one}      // -x
                    },
                    {mon{minus_one, one, 1, 0}, // -z
                     mon{one, one, 1, 1},       // y
                     mon{minus_one, one, 1, 2}, // -x
                     mon{one, zero, 0, 0}},
                    {
                        term{1, 0, 0b111},  // -z * e012
                        term{1, 1, 0b1011}, // y * e013
                        term{1, 2, 0b1101}, // x * e023
                        term{1, 3, 0b1110}  // e123
                    }};
        }

        [[nodiscard]] constexpr static size_t size() noexcept
        {
            return 3;
        }

        [[nodiscard]] constexpr static uint32_t ind_count() noexcept
        {
            return 3;
        }

        constexpr point(T x, T y, T z) noexcept
            : x{x}
            , y{y}
            , z{z}
        {}

        template <uint8_t... E>
        constexpr point(entity<pga_algebra, T, E...> in) noexcept
            : data{}
        {
            auto input = in.template select<0b111, 0b1011, 0b1101, 0b1110>();
            auto w_inv = T{1} / input[3];
            z = -input[0] * w_inv;
            y = input[1] * w_inv;
            x = -input[2] * w_inv;
        }

        [[nodiscard]] constexpr T const& operator[](size_t index) const noexcept
        {
            return data[index];
        }

        [[nodiscard]] constexpr T& operator[](size_t index) noexcept
        {
            return data[index];
        }

        [[nodiscard]] constexpr T get(size_t i) const noexcept
        {
            // Unused
            return NAN;
        }
    };

    template <typename T = float>
    union vector
    {
        using algebra_t = pga_algebra;
        using value_t   = T;
        constexpr static bool is_dual = true;

        std::array<T, 3> data;
        struct
        {
            T x;
            T y;
            T z;
        };

        // Like planes, points are represented dually as the intersection of three planes
        [[nodiscard]] constexpr static mv<algebra_t, 3, 3, 3> ie(uint32_t id) noexcept
        {
            return {mv_size{3, 4, 4},
                    {
                        ind{id + 2, one}, // -z
                        ind{id + 1, one}, // y
                        ind{id, one}      // -x
                    },
                    {
                        mon{minus_one, one, 1, 0}, // -z
                        mon{one, one, 1, 1},       // y
                        mon{minus_one, one, 1, 2}  // -x
                    },
                    {
                        term{1, 0, 0b111},  // -z * e012
                        term{1, 1, 0b1011}, // y * e013
                        term{1, 2, 0b1101}  // x * e023
                    }};
        }

        [[nodiscard]] constexpr static size_t size() noexcept
        {
            return 3;
        }

        [[nodiscard]] constexpr static uint32_t ind_count() noexcept
        {
            return 3;
        }

        constexpr vector(T x, T y, T z) noexcept
            : x{x}
            , y{y}
            , z{z}
        {}

        template <uint8_t... E>
        constexpr vector(entity<pga_algebra, T, E...> in) noexcept
            : data{}
        {
            auto input = in.template select<0b111, 0b1011, 0b1101>();
            z = -input[0];
            y = input[1];
            x = -input[2];
        }

        [[nodiscard]] constexpr T const& operator[](size_t index) const noexcept
        {
            return data[index];
        }

        [[nodiscard]] constexpr T& operator[](size_t index) noexcept
        {
            return data[index];
        }

        [[nodiscard]] constexpr T get(size_t i) const noexcept
        {
            // Unused
            return NAN;
        }
    };

    // Lines in P^3 are defined using Plücker coordinates: https://en.wikipedia.org/wiki/Plücker_coordinates
    // The lines e01, e02, and e03 are the ideal lines representing the intersections of e1, e2, and e3 with the ideal
    // plane respectively. The lines e23, e31, and e12 are lines through the origin in the x, y, and z directions
    // respectively.
    template <typename T = float>
    union line
    {
        using algebra_t = pga_algebra;
        using value_t = T;
        constexpr static bool is_dual = true;

        std::array<T, 6> data;
        struct
        {
            T dx;
            T dy;
            T dz;
            T mx;
            T my;
            T mz;
        };

        [[nodiscard]] constexpr static mv<algebra_t, 6, 6, 6> ie(uint32_t id) noexcept
        {
            return {mv_size{6, 6, 6},
                    {
                        ind{id + 3, one}, // mx
                        ind{id + 4, one}, // my
                        ind{id + 2, one}, // dz
                        ind{id + 5, one}, // mz
                        ind{id + 1, one}, // dy
                        ind{id, one}      // dx
                    },
                    {
                        mon{one, one, 1, 0}, // mx
                        mon{one, one, 1, 1}, // my
                        mon{one, one, 1, 2}, // dz
                        mon{one, one, 1, 3}, // mz
                        mon{one, one, 1, 4}, // dy
                        mon{one, one, 1, 5}  // dx
                    },
                    {
                        term{1, 0, 0b11},   // e01
                        term{1, 1, 0b101},  // e02
                        term{1, 2, 0b110},  // e12
                        term{1, 3, 0b1001}, // e03
                        term{1, 4, 0b1010}, // e13
                        term{1, 5, 0b1100}  // e23
                    }};
        }

        [[nodiscard]] constexpr static size_t size() noexcept
        {
            return 6;
        }

        [[nodiscard]] constexpr static uint32_t ind_count() noexcept
        {
            return 6;
        }

        constexpr line(T dx, T dy, T dz, T mx, T my, T mz) noexcept
            : dx{dx}
            , dy{dy}
            , dz{dz}
            , mx{mx}
            , my{my}
            , mz{mz}
        {}

        template <uint8_t... E>
        constexpr line(entity<pga_algebra, T, E...> in) noexcept
            : data{in.template select<0b11, 0b101, 0b100, 0b1001, 0b1010, 0b1100>()}
        {}

        constexpr line(std::array<T, 6> in) noexcept
            : data{in}
        {}

        [[nodiscard]] constexpr T const& operator[](size_t index) const noexcept
        {
            return data[index];
        }

        [[nodiscard]] constexpr T& operator[](size_t index) noexcept
        {
            return data[index];
        }

        [[nodiscard]] constexpr T get(size_t i) const noexcept
        {
            // Unused
            return NAN;
        }
    };

    // A bivector has a closed-form exponential solution which can be used to produce a motor
    template <typename T>
    [[nodiscard]] constexpr motor<T> exp(line<T> const& l) noexcept
    {
        // We need to decompose the line l into two parts which scale the normalized euclidean and ideal components of
        // the line L.
        // L being a bivector squares to produce a quantity of the form L^2 = s + p * I.
        // norm = sqrt(-L^2) = sqrt(-s - p * I) = sqrt(-s) + p * sqrt(-s) / (2s) * I
        // The inverse of the norm is given by norm_inv = 1/sqrt(-s) - p / sqrt(-s) * I
        // So L_norm can be written as norm_inv * L
        auto l2 = compute([](auto l) { return l * l; }, l);
        static_assert(l2.size() == 2);
        auto s = -l2[0];
        auto u = std::sqrt(s);
        auto v = -l2[1] / (2 * u);
        l2[0]  = u;
        l2[1]  = v;

        decltype(l2) inv_norm{T{1} / u, -l2[1] / s};
        auto cos_u = std::cos(u);
        auto sin_u = std::sin(u);
        decltype(l2) real{cos_u, -v * sin_u};
        decltype(l2) ideal{sin_u, v * cos_u};
        return compute([](auto real, auto ideal, auto inv_norm, auto l) { return real + ideal * inv_norm * l; },
                       real,
                       ideal,
                       inv_norm,
                       l);
    }

    // A closed-form solution of the logarithm of an element of the even subalgebra also exists
    template <typename T>
    [[nodiscard]] constexpr line<T> log(motor<T> const& m) noexcept
    {
        // When normalized, the motor has the form <m>_0 + <m>_2 + <m>4
        // m := s1 + L + p1 * I
        // Decompose L into L = (s2 + p2 * I) * L_norm => m = s1 + p1 * I + (s2 + p2 * I) * L_norm
        // The norm is computed as above with norm = sqrt(-s2) + p2/(2 * sqrt(-s2)) * I
        // so norm_inv is 1/sqrt(-s2) + p2/sqrt(-s2) * I
        // The exponential has the form (cosu - vsinu * I) + (sinu + vcosu * I) * L_norm = e^((u + v*I) * L_norm)
        // Comparing the two equations leads to:
        // s1 = cosu
        // p2 = vcosu
        // s2 = sinu
        // p1 = -vsinu
        //
        // If s1 isn't 0, we can solve for u as arctan(s2/s1) and v = p2/s1 using the first 2 equations
        // If s1 is 0, we take instead the second and fourth equation so u = arctan(-p2/p1) and v = -p1/s2
        auto s1 = m[0]; // <m>_0
        auto p1 = m[7]; // <m>_4

        line<T> l{m.template select<0b11, 0b101, 0b110, 0b1001, 0b1010, 0b1100>()};
        // s + p * I
        auto l2 = compute([](auto l) { return l * l; }, l);
        static_assert(l2.size() == 2);
        auto s2 = std::sqrt(-l2[0]);
        auto p2 = -l2[1] / (2 * s2);

        bool s1_zero = std::abs(s1) < T{1e-6};

        scalar<pga_algebra, T> u{s1_zero ? std::atan2(-p1, p2) : std::atan2(s2, s1)};
        scalar<pga_algebra, T> v{s1_zero ? -p1 / s2 : p2 / s1};
        decltype(l2) norm_inv{-s2, p2 / (2 * s2)};
        return compute(
            [](auto norm_inv, auto l, auto u, auto v) { return (u + v * ps<T>)*norm_inv * l; }, norm_inv, l, u, v);
    }
} // namespace pga
} // namespace gal