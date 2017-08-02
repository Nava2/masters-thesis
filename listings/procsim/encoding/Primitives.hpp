#pragma once
#ifndef PROCSIM_ENCODING_PRIMITIVES_HPP_
#define PROCSIM_ENCODING_PRIMITIVES_HPP_

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
#include "procsim/Types.hpp"

#include "procsim/util/Math.hpp"
#include "procsim/util/Units.hpp"

#include <cstdint>
#include <iostream>

PROCSIM_NAMESPACE_START

namespace encoding
{

enum class SignType : int8_t
{
    Signed   = -1,
    Unsigned = 1
};

struct PROCSIM_EXPORT TypedNumber PROCSIM_FINAL
{
    static const char* const TYPE_NAME;

    inline PROCSIM_CONSTEXPR TypedNumber(const SignType type, const bits width) : sign(type), width(width) {}

    const SignType sign;

    /// Width of number in bits
    const bits width;

    /// Width of a number in bytes
    inline PROCSIM_CONSTEXPR bytes byteWidth() const { return width; }

    inline PROCSIM_CONSTEXPR bool operator==(const TypedNumber& other) const
    {
        return (this == &other) || (sign == other.sign && width == other.width);
    }

    template <typename T = uint64_t>
    inline PROCSIM_CONSTEXPR const std::make_unsigned_t<T> idx_mask(const bits index) const
    {
        return math::build_mask<std::make_unsigned_t<T>>(width, index);
    }

    template <typename T = uint64_t>
    inline PROCSIM_CONSTEXPR const T mask() const
    {
        return math::build_mask<T>(width);
    }
};

static PROCSIM_CONSTEXPR const TypedNumber u1{ SignType::Unsigned, bits{ 1 } };
static PROCSIM_CONSTEXPR const TypedNumber u2{ SignType::Unsigned, bits{ 2 } };
static PROCSIM_CONSTEXPR const TypedNumber u3{ SignType::Unsigned, bits{ 3 } };
static PROCSIM_CONSTEXPR const TypedNumber u4{ SignType::Unsigned, bits{ 4 } };
static PROCSIM_CONSTEXPR const TypedNumber u8{ SignType::Unsigned, bits{ 8 } };
static PROCSIM_CONSTEXPR const TypedNumber u16{ SignType::Unsigned, bits{ 16 } };
static PROCSIM_CONSTEXPR const TypedNumber u32{ SignType::Unsigned, bits{ 32 } };
static PROCSIM_CONSTEXPR const TypedNumber u64{ SignType::Unsigned, bits{ 64 } };

static PROCSIM_CONSTEXPR const TypedNumber i2{ SignType::Signed, bits{ 2 } };
static PROCSIM_CONSTEXPR const TypedNumber i3{ SignType::Signed, bits{ 3 } };
static PROCSIM_CONSTEXPR const TypedNumber i4{ SignType::Signed, bits{ 4 } };
static PROCSIM_CONSTEXPR const TypedNumber i8{ SignType::Signed, bits{ 8 } };
static PROCSIM_CONSTEXPR const TypedNumber i16{ SignType::Signed, bits{ 16 } };
static PROCSIM_CONSTEXPR const TypedNumber i32{ SignType::Signed, bits{ 32 } };
static PROCSIM_CONSTEXPR const TypedNumber i64{ SignType::Signed, bits{ 64 } };

/// Stores a value, `uint64_t` that is paired with a type so it can be masked appropriately
struct PROCSIM_EXPORT TypedValue PROCSIM_FINAL
{
    static const char* const TYPE_NAME;

    template <typename T>
    inline PROCSIM_CONSTEXPR TypedValue(const TypedNumber type, const T value)
        : type(type), value(math::signExtend<uint64_t>(value & type.mask<T>()))
    {
    }

    /// The underlying type of the value
    const TypedNumber type;

    /// Actual value stored as a uint64_t
    const uint64_t value;

    inline PROCSIM_CONSTEXPR bool operator==(const TypedValue& other) const
    {
        return (this == &other) || (type == other.type && value == other.value);
    }

    inline PROCSIM_CONSTEXPR const TypedValue operator=(const TypedValue other) const { return TypedValue(other); }

    inline PROCSIM_CONSTEXPR const TypedValue operator=(const uint64_t v) const { return TypedValue(type, v); }

    inline PROCSIM_CONSTEXPR const TypedValue operator+(const TypedValue other) const
    {
        return TypedValue(TypedNumber(std::min(type.sign, other.type.sign), std::max(type.width, other.type.width)),
                          value + other.value);
    }

    inline PROCSIM_CONSTEXPR const TypedValue operator+(const uint64_t other) const
    {
        return TypedValue(TypedNumber(type.sign, type.width), value + other);
    }
};

/// Index and Type pairing
struct PROCSIM_EXPORT TypedIndexedNumber PROCSIM_FINAL
{

public:
    static const char* const TYPE_NAME;

    /// Index within the operand
    const bits index;

    /// Type of operand when extracted
    const TypedNumber type;

    /// Get the mask of this indexed number
    const uint64_t mask;

    inline PROCSIM_CONSTEXPR TypedIndexedNumber(const bits index, const TypedNumber type)
        : index(index), type(type), mask(type.idx_mask(index))
    {
    }

    inline PROCSIM_CONSTEXPR TypedIndexedNumber(const TypedIndexedNumber&) = default;
    inline PROCSIM_CONSTEXPR TypedIndexedNumber(TypedIndexedNumber&&)      = default;

    /// Check if two are equal
    inline PROCSIM_CONSTEXPR bool operator==(const TypedIndexedNumber& other) const
    {
        return (this == &other || (this->index == other.index && this->type == other.type));
    }

    /// Create a new instance that is a copy of \p other.
    inline PROCSIM_CONSTEXPR const TypedIndexedNumber operator=(const TypedIndexedNumber& other) const
    {
        return TypedIndexedNumber(other);
    }

    inline PROCSIM_CONSTEXPR const TypedValue extract(const uint64_t val) const
    {
        return TypedValue(type, (val & mask) >> (index - (type.width - bits{ 1 })));
    }

    /// Return true if the underlying type is signed.
    inline PROCSIM_CONSTEXPR const bool isSigned() const { return type.sign == SignType::Signed; }
};

/// Streams 'u' or 'i' for `Unsigned` or `Signed` respectively.
inline std::ostream& operator<<(std::ostream& os, const SignType sign)
{
    switch (sign)
    {
    case SignType::Unsigned:
        os << "u";
        break;

    case SignType::Signed:
        os << "i";
        break;

    default:
        BOOST_ASSERT_MSG(false, "Invalid SignType");
    }

    return os;
}
inline std::ostream& operator<<(std::ostream& os, const TypedNumber tn) { return os << tn.sign << tn.width; }

inline std::ostream& operator<<(std::ostream& os, const TypedIndexedNumber& it)
{
    return os << it.type << "@" << it.index;
}

inline std::ostream& operator<<(std::ostream& os, const TypedValue tv)
{
    return os << tv.type << "{" << tv.value << "}";
}

} // end namespace encoding

PROCSIM_NAMESPACE_END

#endif // PROCSIM_ENCODING_PRIMITIVES_HPP_
