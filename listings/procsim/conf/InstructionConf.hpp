#pragma once
#ifndef PROCSIM_CONF_INSTRUCTION_INL_
#define PROCSIM_CONF_INSTRUCTION_INL_

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

#include "procsim/core/Instruction.hpp"

#include "Loader.hpp"

#include "EncodingConf.hpp"

#include "procsim/core/Proc.hpp"
#include "procsim/encoding/Operand.hpp"

#include <sol.hpp>

PROCSIM_NAMESPACE_START

namespace conf
{

namespace Instruction_details_
{
    void validate(const sol::table& table, const std::shared_ptr<ValidateData> vd);

    std::shared_ptr<Instruction> fromTable(const sol::table& table, const std::shared_ptr<ValidateData> vd);

    Instruction::Parameters paramsFromTable(const sol::table& table, const std::shared_ptr<ValidateData>& vd);
}

LoaderDef(Instruction, Encoding)
{
    // clang-format off
    sol::usertype<Instruction> udata(sol::call_constructor, sol::no_constructor
                                     , "new", [vd](const sol::table& table) -> std::shared_ptr<Instruction> {
                                         return Instruction_details_::fromTable(table, vd);
                                     }
                                     // properties
                                     , "name", sol::property(&Instruction::name)
                                     , "code", sol::property(&Instruction::code)
                                     , "codeWidth", sol::property(&Instruction::codeWidth)
                                     , "cycles", sol::property(&Instruction::cycles)
                                     , "encoding", sol::property(&Instruction::opEncoding)
                                     , "exec", sol::property(&Instruction::exec));
    lua.set_usertype(udata);
    // clang-format on
}

} // end namespace conf

PROCSIM_NAMESPACE_END

#endif // PROCSIM_CONF_INSTRUCTION_INL_
