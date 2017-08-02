#pragma once
#ifndef PROCSIM_ENCODING_ALGORITHM_HPP_
#define PROCSIM_ENCODING_ALGORITHM_HPP_

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

#include "Primitives.hpp"

#include "procsim/Macros.hpp"
#include "procsim/Types.hpp"
#include "procsim/util/Math.hpp"

#include <cstdint>
#include <memory>
#include <unordered_map>

PROCSIM_NAMESPACE_START

namespace encoding
{

/// Definition of how to encode variables
typedef std::unordered_map<std::string, TypedIndexedNumber> Definition;

/// Extracted variables from an encoding
typedef std::unordered_map<std::string, TypedValue> Extraction;

class PROCSIM_EXPORT Algorithm
{

public:
    inline Algorithm(const Definition& definition) : _encodeDefinition(new Definition(definition)) {}

    inline Algorithm() : _encodeDefinition(nullptr) {}

    inline Algorithm(const Algorithm&) = default;
    virtual ~Algorithm()
    {
        if (_encodeDefinition)
        {
            delete _encodeDefinition;
        }
    }

    inline bool operator==(const Algorithm& other) const PROCSIM_NOEXCEPT
    {
        if (this == &other)
        {
            return true;
        }

        // Check that the pointers are valid or invalid
        return ptr_eq(this->_encodeDefinition, other._encodeDefinition);
    }

    /// Calculate the minimum width required to represent all of the encoded values. This
    /// calculation finds the minimum number of bytes
    inline virtual const bytes minRep() const
    {
        if (!this->_encodeDefinition)
        {
            return bytes{ 0 };
        }

        uint64_t v = 0;
        for (const auto& p : *_encodeDefinition)
        {
            v |= p.second.mask;
        }

        return math::min_rep_width<bytes>(v);
    }

    /// Compute a mask for this encoding that hits all of the encoded positions.
    inline virtual const uint64_t mask() const { return math::build_mask<uint64_t>(minRep()); }

    /// Decodes a value into a map with all of the values by name with their types
    virtual const Extraction decode(const uint64_t value) const;

protected:
    /// Defines the encoded variables
    const Definition* const _encodeDefinition;
};

} // end namespace encoding

PROCSIM_NAMESPACE_END

#endif // PROCSIM_ENCODING_ALGORITHM_HPP_
