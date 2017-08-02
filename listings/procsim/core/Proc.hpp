#pragma once
#ifndef PROCSIM_CORE_PROC_HPP_
#define PROCSIM_CORE_PROC_HPP_

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

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

PROCSIM_NAMESPACE_START

// Forwards:
namespace time
{
class Clock;
} // time

namespace memory
{
class MemoryUnit;
class Register;
} // memory

class Instruction;

namespace conf
{
class ProcBuilder;
} // conf

// end forwards

class PROCSIM_EXPORT Proc PROCSIM_FINAL
{

public:
    //!< Name of `this` type, all subclasses of `ConfObj` must have this.
    static PROCSIM_CONSTEXPR const char TYPE_NAME[] = "Proc";

    /*!
     * Constructs a new instance <i>outside</i> of Lua
     * \param name Name of this instance (used with path)
     * \param parent A pointer to the parent
     */
    Proc(const std::shared_ptr<time::Clock>& procClock,
         const std::string& name,
         const named_map<time::Clock>& clocks,
         const named_map<Instruction>& instructions,
         const named_map<memory::Register>& registers,
         const std::unordered_set<std::shared_ptr<memory::MemoryUnit>>& memory);

    Proc(Proc&&)      = default;
    Proc(const Proc&) = default;

    ~Proc() = default;

    bool operator==(const Proc& other) const;

    const std::string& name() const PROCSIM_NOEXCEPT;

    const std::shared_ptr<time::Clock>& procClock() const PROCSIM_NOEXCEPT;

    const named_map<time::Clock>& clocks() const PROCSIM_NOEXCEPT;

    const named_map<Instruction>& instructions() const PROCSIM_NOEXCEPT;

    const named_map<memory::Register>& registers() const PROCSIM_NOEXCEPT;

    const std::unordered_set<std::shared_ptr<memory::MemoryUnit>>& memory() const PROCSIM_NOEXCEPT;

private:
    const std::string _name;

    const std::shared_ptr<time::Clock> _procClock;

    /// all known clocks
    const named_map<time::Clock> _clocks;
    const named_map<Instruction> _instructions;
    const named_map<memory::Register> _registers;
    const std::unordered_set<std::shared_ptr<memory::MemoryUnit>> _memory;

    friend class conf::ProcBuilder;
    friend std::ostream& operator<<(std::ostream& os, const Proc& proc);
};

PROCSIM_NAMESPACE_END

#endif // PROCSIM_CORE_PROC_HPP_
