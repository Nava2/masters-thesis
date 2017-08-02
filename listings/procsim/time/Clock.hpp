#pragma once
#ifndef PROCSIM_TIME_CLOCK_HPP_
#define PROCSIM_TIME_CLOCK_HPP_

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

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>

PROCSIM_NAMESPACE_START

namespace time
{

class Clock PROCSIM_FINAL
{

public:
    enum class PulseType
    {
        SQUARE    = 0,
        PULSE     = 1,
        INV_PULSE = 2
    };

    //!< Name of `this` type
    static PROCSIM_CONSTEXPR const char TYPE_NAME[] = "Clock";

    //!< Minimum allowable clock speed
    static const std::chrono::nanoseconds MINIMUM_CLOCK_TIME;

    /*!
     * Constructs a new instance <i>outside</i> of Lua
     * \param name Name of this instance (used with path)
     * \param parent A pointer to the parent
     */
    Clock(const optional<std::string>& name, const std::chrono::nanoseconds& period, const PulseType pulseType);

    Clock(Clock&&)      = default;
    Clock(const Clock&) = default;

    virtual ~Clock() = default;

    inline bool operator==(const Clock& other) const
    {
        return (this == &other || (_name == other._name && _pulseType == other._pulseType && _period == other._period));
    }

    inline const std::string& name() const { return *_name; }

    inline const optional<std::string>& name_opt() const PROCSIM_NOEXCEPT { return _name; }
    inline void set_name(const std::string& name) { _name = name; }

    //!< Name of the current node
    template <class TimeUnit = std::chrono::nanoseconds>
    inline const TimeUnit period() const PROCSIM_NOEXCEPT
    {
        return _period;
    }

    inline const PulseType pulseType() const PROCSIM_NOEXCEPT { return _pulseType; }

private:
    optional<std::string> _name;

    //!< Name of the current node
    const std::chrono::nanoseconds _period;
    const PulseType _pulseType;

    friend std::ostream& operator<<(std::ostream&, const Clock&);
};

const std::shared_ptr<Clock>& inherit_clock();

template <>
inline const uint64_t Clock::period<uint64_t>() const PROCSIM_NOEXCEPT
{
    return _period.count();
}

inline std::ostream& operator<<(std::ostream& os, const Clock::PulseType pt)
{
    os << "PulseType::";
    switch (pt)
    {
    case Clock::PulseType::PULSE:
        os << "PULSE";
        break;

    case Clock::PulseType::INV_PULSE:
        os << "INV_PULSE";
        break;

    case Clock::PulseType::SQUARE:
        os << "SQUARE";
        break;

    default:
        break;
    }

    return os;
}

} // end namespace time

PROCSIM_NAMESPACE_END

#endif // PROCSIM_TIME_CLOCK_HPP_
