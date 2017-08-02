#pragma once
#ifndef PROCSIM_CONF_CLOCK_INL_
#define PROCSIM_CONF_CLOCK_INL_

/*
 * The following is part of the Procsim Library written and developed
 * by Kevin Brightwell under the supervision of Dr. Kenneth McIssac
 * and Dr. Roy Eagleson at Western University, Canada.
 *
 * \author Kevin Brightwell
 *
 * This software is provided as is and all rights are maintained by
 * Kevin Brightwell
 */

#include "procsim/time/Clock.hpp"

#include "Loader.hpp"

#include <chrono>

#include <boost/assert.hpp>

#include <sol.hpp>

PROCSIM_NAMESPACE_START

namespace conf
{

template <typename TimeUnit = std::chrono::nanoseconds>
inline const TimeUnit getTime(const sol::object& timeVal) PROCSIM_NOEXCEPT;

template <>
inline const std::chrono::nanoseconds getTime(const sol::object& timeVal) PROCSIM_NOEXCEPT
{
    BOOST_ASSERT(timeVal.valid());

    if (timeVal.is<int64_t>())
    {
        return std::chrono::nanoseconds(timeVal.as<int64_t>());
    }
    else
    {
        BOOST_ASSERT_MSG(0,
                         "Unknown Time type in Table, explain how this happened. "
                         "I'm genuinely curious.");
        return std::chrono::nanoseconds(-1); // won't happen, but appeases GCC
    }
}

template <typename TimeUnit>
inline const TimeUnit getTime(const sol::object& timeVal) PROCSIM_NOEXCEPT
{
    BOOST_ASSERT(timeVal.valid());

    if (timeVal.is<int64_t>())
    {
        return TimeUnit(std::chrono::nanoseconds(timeVal.as<int64_t>()));
    }
    else
    {
        BOOST_ASSERT_MSG(0,
                         "Unknown Time type in Table, explain how this happened. "
                         "I'm genuinely curious.");
        return TimeUnit(0); // won't happen, but appeases GCC
    }
}

namespace ChronoHelpers_details
{

    template <typename D>
    inline std::function<int64_t(int64_t)> makeChrono()
    {
        return [](const int64_t duration) -> int64_t {
            const D real(duration);

            return std::chrono::nanoseconds(real).count();
        };
    }

    template <>
    inline std::function<int64_t(int64_t)> makeChrono<std::chrono::nanoseconds>()
    {
        return [](const int64_t duration) -> int64_t { return duration; };
    }

} // end anonymous ChronoHelpers_details

NSLoaderDefS(ChronoHelpers)
{
    ignore(vd);

    lua.set_function("nanos", ChronoHelpers_details::makeChrono<std::chrono::nanoseconds>());
    lua.set_function("micros", ChronoHelpers_details::makeChrono<std::chrono::microseconds>());
    lua.set_function("millis", ChronoHelpers_details::makeChrono<std::chrono::milliseconds>());
    lua.set_function("seconds", ChronoHelpers_details::makeChrono<std::chrono::seconds>());

    // frequencies
    lua.set_function("Hz", ChronoHelpers_details::makeChrono<std::chrono::seconds>());
    lua.set_function("MHz", ChronoHelpers_details::makeChrono<std::chrono::milliseconds>());
    lua.set_function("GHz", ChronoHelpers_details::makeChrono<std::chrono::microseconds>());
    lua.set_function("THz", ChronoHelpers_details::makeChrono<std::chrono::nanoseconds>());
}

namespace Clock_details_
{
    // Extract the values from the table: name, period, pulseType
    std::tuple<optional<std::string>, std::chrono::nanoseconds, time::Clock::PulseType>
    extract(const sol::table& table);

    const std::shared_ptr<time::Clock> fromTable(const sol::table& table, const std::shared_ptr<ValidateData> vd);

    void validate(const sol::table& table, const std::shared_ptr<ValidateData> vd);
}

LoaderDef(time::Clock, ChronoHelpers)
{
    // clang-format off
    lua.new_enum("PulseType"
                 , "INV_PULSE", time::Clock::PulseType::INV_PULSE
                 , "PULSE", time::Clock::PulseType::PULSE
                 , "SQUARE", time::Clock::PulseType::SQUARE);

    sol::usertype<time::Clock> utype(sol::call_constructor, sol::no_constructor
                                     , "new", [vd](const sol::table& table) -> const std::shared_ptr<time::Clock> {
                                         return Clock_details_::fromTable(table, vd);
                                     }
                                     , "period", sol::property(&time::Clock::period<uint64_t>)
                                     , "pulseType", sol::property(&time::Clock::pulseType));
    lua.set_usertype(utype);
    // clang-format on 

    lua["inherit_clk"] = time::inherit_clock();
}

} // end namespace conf

PROCSIM_NAMESPACE_END

#endif // PROCSIM_CONF_CLOCK_INL_