#pragma once
#ifndef PROCSIM_CONF_REGISTERCONF_HPP_
#define PROCSIM_CONF_REGISTERCONF_HPP_

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

#include "procsim/memory/Register.hpp"

#include "Loader.hpp"
#include "Validate.hpp"

#include "MemoryUnitConf.hpp"

#include "procsim/Types.hpp"

#include "procsim/memory/MemoryUnit.hpp"
#include "procsim/time/Clock.hpp"

#include <memory>

#include <sol.hpp>

PROCSIM_NAMESPACE_START

namespace conf
{

namespace Register_details_
{

    // verify the content of a table is correct
    // &vd used because no copy is needed
    void validate(const sol::table& table, const std::shared_ptr<ValidateData>& vd);

    // Loads a unit from a table
    // no &vd is used because taking a reference into a lambda
    const std::shared_ptr<memory::Register> fromTable(const sol::table& table, const std::shared_ptr<ValidateData> vd);
}

LoaderDef(memory::Register, memory::MemoryUnit, time::Clock)
{
    using namespace memory;

    // clang-format off
    sol::usertype<Register> utype(sol::call_constructor, sol::no_constructor
            , "new", [vd](const sol::table& table) -> std::shared_ptr<Register> { 
                return Register_details_::fromTable(table, vd); 
            }
            , sol::base_classes, sol::bases<MemoryUnit>()

            // TODO metatable methods for operations
        );
    lua.set_usertype(utype);
    // clang-format on
}

} // end namespace conf

PROCSIM_NAMESPACE_END

#endif // PROCSIM_CONF_REGISTERCONF_HPP_
