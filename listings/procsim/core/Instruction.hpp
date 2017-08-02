#pragma once
#ifndef PROCSIM_INSTRUCTION_HPP_
#define PROCSIM_INSTRUCTION_HPP_

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

#include "procsim/util/Units.hpp"

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include <boost/assert.hpp>
#include <boost/variant.hpp>

PROCSIM_NAMESPACE_START

// forwards:
namespace encoding
{

class Code;
class Operand;
}

class Proc;

// end forwards

class PROCSIM_EXPORT Instruction PROCSIM_FINAL
{

public:
    /// The type of function all Instructions use when executing
    typedef std::function<void(Proc&, encoding::Operand&)> exec_fn_t;

    struct Parameters
    {
        //! Byte codes for encoding, max size uint64
        const std::shared_ptr<encoding::Code> code;

        //! Number of cycles for function
        const uint8_t cycles;

        //! The encoding of the operand
        const std::shared_ptr<encoding::Operand> opEncoding;

        const exec_fn_t exec;
    };

    //!< Name of `this` type, all subclasses of `ConfObj` must have this.
    static PROCSIM_CONSTEXPR const char TYPE_NAME[] = "Instruction";

    /*!
     * Constructs a new instance <i>outside</i> of Lua
     * \param name Name of this instance (used with path)
     * \param parent A pointer to the parent
     */
    Instruction(const optional<std::string>& name,
                const std::shared_ptr<encoding::Code>& code,
                const uint8_t cycles,
                const std::shared_ptr<encoding::Operand>& opEncoding,
                const exec_fn_t& exec);

    Instruction(Parameters&& params, const optional<std::string>& name = nullopt);

    Instruction(Instruction&&)      = default;
    Instruction(const Instruction&) = default;
    ~Instruction()                  = default;

    bool operator==(const Instruction& other) const;

    /// Name (mnemonic) for this instruction
    inline const std::string& name() const { return *_name; }
    inline void set_name(const std::string& name) { _name = name; }

    inline const optional<std::string> name_opt() const { return _name; }

    inline const std::shared_ptr<encoding::Code> code() const { return _code; }

    /// Get the width of the code
    const bytes codeWidth() const;

    /// Get the cycles per instruction call
    inline const uint8_t cycles() const { return _cycles; }

    /// Get the encoding of the operand, if it exists
    inline const std::shared_ptr<encoding::Operand> opEncoding() const { return _opEncoding; }

    /// Get the execution function
    inline exec_fn_t& exec() { return _exec; }

private:
    optional<std::string> _name;

    /// Byte codes for encoding, max size uint64
    std::shared_ptr<encoding::Code> _code;

    //! Number of cycles for function
    const uint8_t _cycles;

    const std::shared_ptr<encoding::Operand> _opEncoding;

    exec_fn_t _exec;

    friend std::ostream& operator<<(std::ostream& os, const Instruction& ins);
};

export_smart_ptrs(Instruction)

PROCSIM_NAMESPACE_END

#endif // PROCSIM_INSTRUCTION_HPP_
