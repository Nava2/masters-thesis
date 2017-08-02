#pragma once
#ifndef PROCSIM_CONF_PROCBUILDER_HPP_
#define PROCSIM_CONF_PROCBUILDER_HPP_

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

#include "ProcConf.hpp"
#include "Validate.hpp"

#include "procsim/memory/MemoryUnit.hpp"
#include "procsim/memory/Register.hpp"
#include "procsim/time/Clock.hpp"

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <sol.hpp>

PROCSIM_NAMESPACE_START

// forward
class Proc;

namespace conf
{

/// Builds Proc by using a two-pass systems to build up the instances.
class PROCSIM_EXPORT ProcBuilder PROCSIM_FINAL
{

public:
    ProcBuilder(const std::shared_ptr<ValidateData>& vd) : _vd(vd) {}

    ProcBuilder(ProcBuilder&&)      = default;
    ProcBuilder(const ProcBuilder&) = default;
    ProcBuilder& operator=(const ProcBuilder&) = default;

    ~ProcBuilder() = default;

    std::shared_ptr<Proc> build(const sol::table& procTable) const;

private:
    const std::shared_ptr<ValidateData> _vd;
};

namespace pass
{
    // Forwards
    class Second;

    /// Perform first pass parsing of a Proc structure
    class PROCSIM_EXPORT First PROCSIM_FINAL
    {

    public:
        inline First(const std::shared_ptr<ValidateData>& vd) : _vd(vd) {}

        void run(const sol::table& table);

        inline const std::string& name() const { return _name; }

        inline const std::shared_ptr<time::Clock>& procClock() const { return _procClock; }

        /// all of the clocks
        inline const named_map<time::Clock>& clocks() const { return _clocks; }

        inline const named_map<Instruction>& ins() const { return _instructions; }

        inline const named_map<memory::Register>& regs() const { return _registers; }

        inline const std::unordered_set<std::shared_ptr<memory::MemoryUnit>>& memory() const { return _memory; }

    private:
        /// Set the processor's main clock
        inline void procClock(std::shared_ptr<time::Clock>& clock)
        {
            BOOST_ASSERT_MSG(!!clock, "clock == nullptr");

            _procClock = clock;

            _clocks.insert({ clock->name(), clock });
        }

        /// Add a clock to the processor
        inline void clock(const std::string& name, std::shared_ptr<time::Clock>& clock)
        {
            BOOST_ASSERT_MSG(!!clock, "clock == nullptr");

            _clocks.insert({ name, clock });
        }

        /// Add an instruction to the processor
        inline void instruction(const std::string& name, std::shared_ptr<Instruction>& ins)
        {
            BOOST_ASSERT_MSG(!!ins, "ins == nullptr");

            _instructions.insert({ name, ins });
        }

        /// Add a register to the processor
        inline void reg(const std::string& name, std::shared_ptr<memory::Register>& reg)
        {
            BOOST_ASSERT_MSG(!!reg, "reg == nullptr");

            _registers.insert({ name, reg });
        }

        /// Add a memory unit to the processor
        inline void memory(std::shared_ptr<memory::MemoryUnit>& memory)
        {
            BOOST_ASSERT_MSG(!!memory, "memory == nullptr");

            _memory.insert(memory);
        }

        template <typename T, typename Fn>
        void processTable(Fn fn, const char* tag, const sol::table& table)
        {
            namespace cnf  = locale::conf;
            namespace proc = cnf::Proc;

            auto mfn = std::mem_fn(fn);

            const sol::object& otbl = table[tag];
            if (otbl.valid())
            {
                // extra clocks specified
                if (otbl.is<sol::table>())
                {
                    const sol::table tbl = otbl.as<sol::table>();
                    tbl.for_each([=](const sol::object& okey, const sol::object& value) {

                        std::shared_ptr<T> tval;
                        if (!value.valid())
                        {
                            _vd->err<cnf::NIL_INST>(Proc::TYPE_NAME, tag) << cnf::NIL_INST::arg_type % T::TYPE_NAME
                                                                          << cnf::NIL_INST::end;
                        }
                        else if (value.is<std::shared_ptr<T>>())
                        {
                            tval = value.as<std::shared_ptr<T>>();
                            BOOST_ASSERT(tval);

                            std::string key;
                            if (okey.is<std::string>())
                            {
                                // key is name, thus need to set it
                                key = okey.as<std::string>();
                                tval->set_name(key);
                            }
                            else
                            {
                                key = tval->name();
                                if (key.empty())
                                {
                                    _vd->err<cnf::NOT_SPEC>(T::TYPE_NAME, cnf::ConfObj::tags::NAME)
                                        << cnf::NOT_SPEC::end;
                                }
                            }

                            mfn(this, key, tval);
                        }
                        else // if is shared_ptr
                        {
                            _vd->err<cnf::BAD_TYPE>(Proc::TYPE_NAME, tag) << cnf::BAD_TYPE::exp_type % T::TYPE_NAME
                                                                          << cnf::BAD_TYPE::act_type % value.get_type()
                                                                          << cnf::BAD_TYPE::end;
                        }

                    });
                }
                else
                {
                    _vd->err<cnf::BAD_VALUE>(Proc::TYPE_NAME, tag)
                        << cnf::BAD_VALUE::exp_type % (std::string(T::TYPE_NAME) + "[]") << cnf::BAD_VALUE::end;
                }
            }

            _vd->tags(tag);
        }

        void processMemory(const sol::table& table);

        // Fields:

        const std::shared_ptr<ValidateData> _vd;

        std::string _name;

        std::shared_ptr<time::Clock> _procClock;

        /// all of the clocks
        named_map<time::Clock> _clocks;
        named_map<Instruction> _instructions;
        named_map<memory::Register> _registers;
        std::unordered_set<std::shared_ptr<memory::MemoryUnit>> _memory;

        friend class Second;
    };

    class PROCSIM_EXPORT Second PROCSIM_FINAL
    {

    public:
        inline Second(const First&& first)
            : _vd(std::move(first)._vd)
            , _name(std::move(first)._name)
            , _procClock(std::move(first)._procClock)
            , _clocks(std::move(first)._clocks)
            , _instructions(std::move(first)._instructions)
            , _registers(std::move(first)._registers)
            , _memory(std::move(first)._memory)
        {
        }

        void run(const sol::table& table);

        inline const std::string& name() const { return _name; }

        inline const std::shared_ptr<time::Clock>& procClock() const { return _procClock; }

        /// all of the clocks
        inline const named_map<time::Clock>& clocks() const { return _clocks; }

        inline const named_map<Instruction>& ins() const { return _instructions; }

        inline const named_map<memory::Register>& regs() const { return _registers; }

        inline const std::unordered_set<std::shared_ptr<memory::MemoryUnit>>& memory() const { return _memory; }

    private:
        // Fields:

        const std::shared_ptr<ValidateData> _vd;

        std::string _name;

        std::shared_ptr<time::Clock> _procClock;

        /// all of the clocks
        named_map<time::Clock> _clocks;
        named_map<Instruction> _instructions;
        named_map<memory::Register> _registers;
        std::unordered_set<std::shared_ptr<memory::MemoryUnit>> _memory;

        friend conf::ProcBuilder;
    };
}

} // end namespace conf

PROCSIM_NAMESPACE_END

#endif // PROCSIM_CONF_PROCBUILDER_HPP_
