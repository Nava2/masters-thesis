#pragma once
#ifndef PROCSIM_CONF_MEMORY_HPP_
#define PROCSIM_CONF_MEMORY_HPP_

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

#include "procsim/memory/MemoryUnit.hpp"

#include "ClockConf.hpp"
#include "Loader.hpp"
#include "Validate.hpp"

#include "procsim/Types.hpp"

#include <memory>

#include <sol.hpp>

PROCSIM_NAMESPACE_START

namespace conf
{

LoaderDefS(memory::Access)
{
    using namespace memory;

    // clang-format off
    lua["Access"] = lua.create_table_with(
            "ReadWrite", lua.create_table_with(1, Access::READ, 2, Access::WRITE)
            , "Read", Access::READ
            , "Write", Access::WRITE
        );
    // clang-format on
}

namespace MemoryUnit_details_
{

    namespace validate_
    {
        static PROCSIM_CONSTEXPR const char EXPECTED_ACCESS_TYPE[] = "Access.Value (e.g. Access.Read or Access.Write)";

        // Helpers to remove massive inline functions

        memory::Access::Set access_MU(const sol::object& o_access, const std::shared_ptr<ValidateData> vd);

        memory::Access::Set access_RAM(const sol::object& o_access, const std::shared_ptr<ValidateData> vd);

        memory::Access::Set access_ROM(const sol::object& o_access, const std::shared_ptr<ValidateData> vd);

        void
        common(const sol::table& table, const memory::Access::Set& access, const std::shared_ptr<ValidateData>& vd);
    }

    template <typename T>
    memory::Access::Set validateAccess(const sol::object& o_access, const std::shared_ptr<ValidateData>& vd);

    template <>
    inline memory::Access::Set validateAccess<memory::MemoryUnit>(const sol::object& o_access,
                                                                  const std::shared_ptr<ValidateData>& vd)
    {
        return validate_::access_MU(o_access, vd);
    }

    template <>
    inline memory::Access::Set validateAccess<memory::RAM>(const sol::object& o_access,
                                                           const std::shared_ptr<ValidateData>& vd)
    {
        return validate_::access_RAM(o_access, vd);
    }

    template <>
    inline memory::Access::Set validateAccess<memory::ROM>(const sol::object& o_access,
                                                           const std::shared_ptr<ValidateData>& vd)
    {
        return validate_::access_ROM(o_access, vd);
    }

    // verify the content of a table is correct
    template <typename T>
    inline void validate(const sol::table& table, const std::shared_ptr<ValidateData>& vd)
    {
        BOOST_ASSERT_MSG(vd, "ValidateData must not be null");

        namespace cnf = PROCSIM_N::locale::conf;
        namespace mu  = cnf::MemoryUnit;
        namespace co  = cnf::ConfObj;

        using namespace memory;
        // Access
        // We do access before read/write count since there's no need to check for read if write-only, etc.
        const sol::object o_access = table[mu::tags::ACCESS];
        Access::Set access         = validateAccess<T>(o_access, vd);
        vd->tags(mu::tags::ACCESS);

        validate_::common(table, access, vd);
    }

    namespace extract_
    { // Helpers to remove massive inline function
        memory::MemoryUnit::Parameters common(const sol::table& table);
    }

    // Extracts data from a table
    template <typename T>
    inline memory::MemoryUnit::Parameters extract(const sol::table& table)
    {
        using namespace memory;

        using namespace locale::conf::MemoryUnit::tags;

        MemoryUnit::Parameters params = extract_::common(table);

        const sol::object o_access     = table[ACCESS];
        const sol::object o_readCount  = table[READ_COUNT];
        const sol::object o_writeCount = table[WRITE_COUNT];

        // TODO refactor to remove nullptr
        params.access = validateAccess<T>(o_access, nullptr);

        // read/write count
        params.readCount  = 0;
        params.writeCount = 0;

        if (params.access.find(Access::WRITE) != params.access.end())
        {
            params.writeCount = o_writeCount.as<size_t>();
        }

        if (params.access.find(Access::READ) != params.access.end())
        {
            params.readCount = o_readCount.as<size_t>();
        }

        return params;
    }

    // Loads a unit from a table
    template <typename T>
    inline const std::shared_ptr<T> fromTable(const sol::table& table, const std::shared_ptr<ValidateData> vd)
    {
        static_assert(std::is_base_of<memory::MemoryUnit, T>::value, "T must be base of MemoryUnit");

        validate<T>(table, vd);

        if (vd->good())
        {
            memory::MemoryUnit::Parameters params = MemoryUnit_details_::extract<T>(table);

            const auto out = std::make_shared<T>(std::move(params));

            // handle the parent relationship here, we can't do it in the ctor because its dangerous w/
            // enable_shared_from_this
            if (std::shared_ptr<time::Clock>* clk = boost::get<std::shared_ptr<time::Clock>>(&params.clock))
            {
                out->set_clock(*clk);
            }

            return out;
        }
        else
        {
            return nullptr;
        }
    }

    template <typename T, typename U>
    inline bool eq_mem(const T& lhs, const U& rhs)
    {
        return lhs == rhs;
    }

    template <typename NewFn>
    inline sol::usertype<memory::MemoryUnit> memory_ut(NewFn&& new_fn)
    {
        // clang-format off
        return sol::usertype<memory::MemoryUnit>(
            sol::call_constructor, sol::no_constructor
            , "new", new_fn
            , "clock", sol::property(&memory::MemoryUnit::clock)
            , "readCount", sol::property(&memory::MemoryUnit::readCount)
            , "writeCount", sol::property(&memory::MemoryUnit::writeCount)
            , "offset", sol::property(&memory::MemoryUnit::offset)
            , "size", sol::property(&memory::MemoryUnit::size)
            , "access", sol::property([](memory::MemoryUnit& unit) -> std::vector<memory::Access::Value> 
                { 
                    return std::vector<memory::Access::Value>(unit.access().begin(), unit.access().cend()); 
                })
            , sol::meta_function::equal_to, [](const memory::MemoryUnit& lhs, const memory::MemoryUnit& rhs) { return lhs == rhs; }
        );
        // clang-format on
    }

    template <typename NewFn>
    inline sol::usertype<memory::ROM> memory_ut_ROM(NewFn&& new_fn)
    {
        // clang-format off
        return sol::usertype<memory::ROM>(
                sol::call_constructor, sol::no_constructor
                , "new", new_fn
                , "clock", sol::property(&memory::ROM::clock)
                , "readCount", sol::property(&memory::ROM::readCount)
                , "writeCount", sol::property(&memory::ROM::writeCount)
                , "offset", sol::property(&memory::ROM::offset)
                , "size", sol::property(&memory::ROM::size)
                , "access", sol::property([](memory::ROM& unit) -> std::vector<memory::Access::Value> 
                    { 
                        return std::vector<memory::Access::Value>(unit.access().begin(), unit.access().cend()); 
                    })
                , sol::meta_function::equal_to, sol::overload(
                      [](const memory::ROM& lhs, const memory::MemoryUnit& rhs) { return lhs.check_eq(rhs); }
                    , [](const memory::ROM& lhs, const memory::ROM& rhs) { return lhs == rhs; })
                , sol::base_classes, sol::bases<memory::MemoryUnit>()
            );
        // clang-format on
    }

    template <typename NewFn>
    inline sol::usertype<memory::RAM> memory_ut_RAM(NewFn&& new_fn)
    {
        // clang-format off
        return sol::usertype<memory::RAM>(
            sol::call_constructor, sol::no_constructor
            , "new", new_fn
            , "clock", sol::property(&memory::RAM::clock)
            , "readCount", sol::property(&memory::RAM::readCount)
            , "writeCount", sol::property(&memory::RAM::writeCount)
            , "offset", sol::property(&memory::RAM::offset)
            , "size", sol::property(&memory::RAM::size)
            , "access", sol::property([](memory::RAM& unit) -> std::vector<memory::Access::Value> 
                    { 
                        return std::vector<memory::Access::Value>(unit.access().begin(), unit.access().cend()); 
                    })
            , sol::meta_function::equal_to, sol::overload(
                  [](const memory::RAM& lhs, const memory::MemoryUnit& rhs) { return lhs.check_eq(rhs); }
                , [](const memory::RAM& lhs, const memory::RAM& rhs) { return lhs == rhs; })
            , sol::base_classes, sol::bases<memory::MemoryUnit>()
            );
        // clang-format on
    }

    template <typename T>
    inline std::function<std::shared_ptr<T>(const sol::table&)> bindFromTable(const std::shared_ptr<ValidateData> vd)
    {
        return std::bind(&fromTable<T>, std::placeholders::_1, vd);
    }
}

LoaderDef(memory::MemoryUnit, memory::Access, time::Clock)
{
    using namespace memory;

    using namespace MemoryUnit_details_;

    sol::usertype<MemoryUnit> memUnit_ut(memory_ut(bindFromTable<MemoryUnit>(vd)));
    lua.set_usertype(memUnit_ut);

    sol::usertype<RAM> ramtype(memory_ut_RAM(bindFromTable<RAM>(vd)));
    lua.set_usertype(ramtype);

    sol::usertype<ROM> romtype(memory_ut_ROM(bindFromTable<ROM>(vd)));
    lua.set_usertype(romtype);
}

} // end namespace conf

PROCSIM_NAMESPACE_END

#endif // PROCSIM_CONF_MEMORY_HPP_
