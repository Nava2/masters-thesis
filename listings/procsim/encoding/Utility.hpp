#pragma once
#ifndef PROCSIM_ENCODING_UTILITY_HPP_
#define PROCSIM_ENCODING_UTILITY_HPP_

/*!
 * The following is part of the Procsim Library written and developed
 * by Kevin Brightwell and Ramesh Raj under the supervision of
 * Dr. Kenneth McIssac at Western University, Canada.
 *
 * \author Kevin Brightwell
 *
 * This software is provided as is and all rights are maintained by
 * Kevin Brightwell
 */

#include "procsim/Macros.hpp"
#include "procsim/TypeTraits.hpp"
#include "procsim/util/Math.hpp"

#include <cstdint>
#include <type_traits>

PROCSIM_NAMESPACE_START

namespace encoding
{

namespace details_
{
    template <typename T, typename... Tn>
    inline PROCSIM_CONSTEXPR const std::make_unsigned_t<std::common_type_t<T, Tn...>>
    a_and_b(const T v, const Tn... args) PROCSIM_NOEXCEPT;

    template <typename T>
    inline PROCSIM_CONSTEXPR const std::make_unsigned_t<T> a_and_b(const T a, const T b) PROCSIM_NOEXCEPT
    {
        return a & b;
    }

    template <typename T, typename... Tn>
    inline PROCSIM_CONSTEXPR const std::make_unsigned_t<std::common_type_t<T, Tn...>>
    a_and_b(const T a, const Tn... rest) PROCSIM_NOEXCEPT
    {
        return a & a_and_b(rest...);
    }

    template <typename... Tn>
    inline PROCSIM_CONSTEXPR const std::make_unsigned_t<std::common_type_t<Tn...>>
    a_xor_not_b(const Tn...) PROCSIM_NOEXCEPT;

    template <typename T>
    inline PROCSIM_CONSTEXPR const std::make_unsigned_t<T> a_xor_not_b(const T a, const T b) PROCSIM_NOEXCEPT
    {
        return a ^ ~b;
    }

    template <typename T>
    inline PROCSIM_CONSTEXPR const std::make_unsigned_t<T> a_xor_not_b(const T a) PROCSIM_NOEXCEPT
    {
        return a;
    }

    template <typename T, typename... Tn>
    inline PROCSIM_CONSTEXPR const std::make_unsigned_t<std::common_type_t<T, Tn...>>
    a_xor_not_b(const T v, const Tn... rest) PROCSIM_NOEXCEPT
    {
        return a_and_b(a_xor_not_b(v, rest)...);
    }

    /// Tests:
    static_assert(a_and_b(0xFF, 0xFF) == 0xFF, "a_and_b(0xFF, 0xFF) != 0xFF");
    static_assert(a_and_b(0xF0, 0x0F) == 0x00, "a_and_b(0xF0, 0x0F) != 0x00");

    static_assert(static_cast<uint8_t>(a_xor_not_b(0xF0, 0x81)) == 0x8E, "a_xor_not_b(0xF0, 0x81) != 0x8E");
    static_assert(static_cast<uint8_t>(a_xor_not_b(0xF0, 0x81, 0x8F)) == 0x80, "a_xor_not_b(0xF0, 0x81, 0x8F) != 0x80");
}

/// Return 0 because if there's no input, no mask can be created.
inline PROCSIM_CONSTEXPR const uint8_t const_mask() PROCSIM_NOEXCEPT { return 0; }

template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
auto const_mask(const std::vector<T>& values) PROCSIM_NOEXCEPT -> const std::make_unsigned_t<T>
{
    typedef std::make_unsigned_t<T> out_t;

    // get the max value of all the inputs
    out_t max = std::numeric_limits<out_t>::min();
    for (const auto v : values)
    {
        max = std::max(max, static_cast<out_t>(v));
    }

    // get the width so it's masked properly
    const bytes width = math::min_rep_width(max);
    out_t out_mask    = math::build_mask<out_t>(width);

    for (auto it1 = values.begin(); it1 != values.end() - 1; ++it1)
    {
        const auto v1 = *it1;
        for (auto it2 = it1 + 1; it2 != values.end(); ++it2)
        {
            out_mask &= details_::a_xor_not_b(v1, *it2);
        }
    }

    return out_mask;
}

template <typename Container,
          typename ValueType = typename Container::value_type,
          typename           = std::enable_if_t<std::is_integral<ValueType>::value>>
auto const_mask(Container values) PROCSIM_NOEXCEPT -> const std::make_unsigned_t<ValueType>
{
    return const_mask(std::vector<ValueType>(values.begin(), values.end()));
}

template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
auto const_mask(std::initializer_list<T> values) PROCSIM_NOEXCEPT -> const std::make_unsigned_t<T>
{
    return const_mask(std::vector<T>(values));
}

template <typename... Args, typename = std::enable_if_t<meta::conjunction_v<std::is_integral<Args>...>>>
inline PROCSIM_RCONSTEXPR auto const_mask(Args... args) PROCSIM_NOEXCEPT
    -> const std::common_type_t<std::make_unsigned_t<Args>...>
{
    typedef std::common_type_t<std::make_unsigned_t<Args>...> out_t;
    const bytes width = math::min_rep_width(std::max({ static_cast<out_t>(args)... }));

    return math::build_mask<out_t>(width) & details_::a_xor_not_b(static_cast<out_t>(args)...);
}

} // end namespace Encoding

PROCSIM_NAMESPACE_END

#endif // PROCSIM_ENCODING_UTILITY_HPP_