#pragma once
#ifndef PROCSIM_ENCODING_OPERAND_HPP_
#define PROCSIM_ENCODING_OPERAND_HPP_

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

#include "Algorithm.hpp"

#include "procsim/Macros.hpp"
#include "procsim/util/Math.hpp"

#include <cstdint>
#include <iostream>
#include <memory>

PROCSIM_NAMESPACE_START

namespace encoding
{

/// Used as a marker for an Algorithm as an Operand
class PROCSIM_EXPORT Operand PROCSIM_FINAL : public Algorithm
{
public:
    static PROCSIM_CONSTEXPR const char TYPE_NAME[] = "Operand";

    inline Operand(const bits bit_width, const Definition& definition) : Algorithm(definition), _bit_width(bit_width) {}

    inline Operand(const Operand&) = default;
    virtual ~Operand()             = default;

    const bool operator==(const Operand& other) const;

    /// Return the *bit* width of the operand
    inline const bits width() const PROCSIM_NOEXCEPT { return _bit_width; }

    /// Return the *byte* width of the operand
    inline const bytes byteWidth() const PROCSIM_NOEXCEPT { return _bit_width; }

private:
    friend std::ostream& operator<<(std::ostream& os, const Operand& enc);

    /// Width of the encoded operand
    const bits _bit_width;
};

export_smart_ptrs(Operand)

} // end namespace encoding

PROCSIM_NAMESPACE_END

#endif // PROCSIM_ENCODING_OPERAND_HPP_
