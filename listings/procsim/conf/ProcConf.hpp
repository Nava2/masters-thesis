#pragma once
#ifndef PROCSIM_CONF_PROC_INL_
#define PROCSIM_CONF_PROC_INL_

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

#include "procsim/core/Proc.hpp"

#include "procsim/core/Instruction.hpp"
#include "procsim/memory/MemoryUnit.hpp"
#include "procsim/memory/Register.hpp"
#include "procsim/time/Clock.hpp"

#include "Loader.hpp"

#include "ClockConf.hpp"
#include "InstructionConf.hpp"
#include "MemoryUnitConf.hpp"
#include "RegisterConf.hpp"

#include "procsim/Macros.hpp"

#include <memory>

#include <sol.hpp>

PROCSIM_NAMESPACE_START

namespace conf
{

namespace Proc_details_
{
    std::shared_ptr<Proc> fromTable(const sol::table& table, const std::shared_ptr<ValidateData> vd);
}

LoaderDef(Proc, time::Clock, Instruction, memory::MemoryUnit, memory::Register)
{
    // clang-format off
    sol::usertype<Proc> udata(
        sol::call_constructor, sol::no_constructor
        , "new", [vd](const sol::table& table) -> std::shared_ptr<Proc> {
            return Proc_details_::fromTable(table, vd);
        }
        
        // properties
        , "name", &Proc::name
        , "procClock", &Proc::procClock
        , "clocks", sol::property([](Proc& proc) -> std::unordered_map<std::string, std::shared_ptr<time::Clock>> 
            {
                return proc.clocks();
            })
        , "instructions", sol::property([](Proc& proc) -> std::unordered_map<std::string, std::shared_ptr<Instruction>> 
            {
                return proc.instructions();
            })
        , "registers", sol::property([](Proc& proc) -> std::unordered_map<std::string, std::shared_ptr<memory::Register>> 
            {
                return proc.registers();
            })
        , "memory", sol::property([](Proc& proc) -> std::vector<std::shared_ptr<memory::MemoryUnit>> 
            {
                return std::vector<std::shared_ptr<memory::MemoryUnit>>(proc.memory().begin(), proc.memory().cend());
            })
    );
    lua.set_usertype(udata);
    // clang-format on
}

} // end namespace conf

PROCSIM_NAMESPACE_END

#endif // PROCSIM_CONF_PROC_INL_
