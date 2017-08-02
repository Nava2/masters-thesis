#pragma once
#ifndef PROCSIM_ENCODING_CODE_HPP_
#define PROCSIM_ENCODING_CODE_HPP_

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
#include "procsim/Types.hpp"

#include <cstdint>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>

PROCSIM_NAMESPACE_START

namespace encoding
{

class Code;

namespace Code_details_
{
    // The following classes allow Encoding::Code to be a pimpl implementation, making the interface faster and
    // much more clean

    class PROCSIM_EXPORT CodeImpl : public Algorithm
    {

    public:
        inline CodeImpl(const std::unordered_map<std::string, uint64_t>& values, const Definition& encodingDefinition)
            : Algorithm(encodingDefinition), _values(values)
        {
            _valueSet.reserve(_values.size());

            for (const auto& p : _values)
            {
                _valueSet.insert(p.second);
            }
        }

        virtual ~CodeImpl() = default;

        inline virtual const bool operator==(const CodeImpl& other) const PROCSIM_NOEXCEPT
        {
            return (this == &other) || (Algorithm::operator==(other) && _values == other._values);
        }

        virtual const bool check(const uint64_t code) const PROCSIM_NOEXCEPT = 0;

        inline const std::unordered_set<uint64_t> values() const { return _valueSet; }

        inline const std::unordered_map<std::string, uint64_t> pairs() const { return _values; }

        virtual std::ostream& stream(std::ostream& os) const = 0;

    protected:
        virtual const uint8_t tag() const = 0;

        friend class Code;

    private:
        const std::unordered_map<std::string, uint64_t> _values;

        std::unordered_set<uint64_t> _valueSet;
    };
}

/// Encoding for constant codes and multi-valued codes
class PROCSIM_EXPORT Code PROCSIM_FINAL : public Algorithm
{
public:
    static const std::string TYPE_NAME;

    Code(const uint64_t constant);
    Code(const std::unordered_map<std::string, uint64_t>& alternates, const Definition& encodedVars = {});
    Code(const uint64_t constant, const Definition& encodedVars);

    Code(Code&&)      = default;
    Code(const Code&) = default;
    Code& operator=(const Code&) = default;
    virtual ~Code()
    {
        if (_impl)
        {
            delete _impl;
        }
    }

    const bool operator==(const Code& other) const PROCSIM_NOEXCEPT;

    virtual const bytes minRep() const override final;

    /// Compute a mask for this encoding that hits all of the encoded positions.
    virtual const uint64_t mask() const override final;

    /// Decodes a value into a map with all of the values by name with their types
    const Extraction decode(const uint64_t value) const override final;

    const bool check(const uint64_t code) const PROCSIM_NOEXCEPT;

    const std::unordered_set<uint64_t> values() const;

    const std::unordered_map<std::string, uint64_t> pairs() const;

private:
    // use pimpl to improve performance
    const Code_details_::CodeImpl* const _impl;

    friend std::ostream& operator<<(std::ostream& os, const Code& code);
};

export_smart_ptrs(Code)

} // end namespace Encoding

PROCSIM_NAMESPACE_END

#endif // PROCSIM_ENCODING_CODE_HPP_
