#pragma once
#ifndef PROCSIM_MEMORY_MEMORYUNIT_HPP_
#define PROCSIM_MEMORY_MEMORYUNIT_HPP_

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

#include "procsim/Types.hpp"

#include "procsim/util/StringIO.hpp"

#include <memory>
#include <set>
#include <string>

#include <boost/variant.hpp>

PROCSIM_NAMESPACE_START

namespace time
{
class Clock;
}

namespace memory
{

/*!
 * \brief Simple marker class for how to access memory
 * Memory and register access is completed by using an `unordered_set` of `Access::Value`s.
 */
struct Access
{

public:
    /*!
     * Marker for Access Types
     */
    enum Value
    {
        _MIN = 0,

        READ  = 1, /// Readable
        WRITE = 2, /// Writable

        _MAX = 3
    };

    /// Shortcut typedef for a set of `Access::Value`s.
    typedef std::set<Value> Set;

    // signifies Read ability
    static const Access::Set Read;

    // Write ability
    static const Access::Set Write;

    /// Shorcut member for Read+Write
    static const Access::Set ReadWrite;

private:
    Access()  = delete;
    ~Access() = delete;

}; // end struct Access

/// Stream the \t Access::Value enumeration
inline std::ostream& operator<<(std::ostream& os, const Access::Value v)
{
    switch (v)
    {
    case Access::READ:
        os << "Access::Read";
        break;
    case Access::WRITE:
        os << "Access::Write";
        break;
    default:
        os << "Access::UNKNOWN{" << static_cast<int32_t>(v) << "}";
        break;
    }

    return os;
}

/// Stream the \t Access::Value enumeration
inline std::ostream& operator<<(std::ostream& os, const Access::Set& v)
{
    if (v == Access::ReadWrite)
    {
        os << "Access::ReadWrite";
    }
    else if (v.size() == 1)
    {
        os << *v.begin();
    }
    else
    { // this doesn't make much sense
        stringio::join(os, v, ", ", '{', '}');
    }

    return os;
}

/*!
 * \brief Describes a unit of memory, for example a ROM or RAM
 */
class PROCSIM_EXPORT MemoryUnit
{

public:
    /// Simple struct for memory parameters, there's too many for function calls.
    struct PROCSIM_EXPORT Parameters
    {
        /// Name for the parameters
        optional<std::string> name;

        /// A name of a \c conf::Clock that will be found outside or a definition
        boost::variant<std::shared_ptr<time::Clock>, std::string> clock;

        /// Cycle count before returning a read value
        size_t readCount;

        /// Cycle count before writing a value
        size_t writeCount;

        /// Offset into processor memory
        optional<size_t> offset;

        /// Size of this memory unit in bytes
        optional<size_t> size;

        /// Access pattern for this memory
        Access::Set access;
    };

    /// Name of `this` type, all subclasses of `ConfObj` must have this.
    static PROCSIM_CONSTEXPR const char TYPE_NAME[] = "MemoryUnit";

    /// Base constructor
    MemoryUnit(const Parameters&& params);

    MemoryUnit(MemoryUnit&&)      = default;
    MemoryUnit(const MemoryUnit&) = default;
    MemoryUnit& operator=(const MemoryUnit&) = default;
    virtual ~MemoryUnit()                    = default;

    /// Check for equality
    bool operator==(const MemoryUnit& other) const;

    inline const std::string name() const { return *_name; }

    inline const optional<std::string> name_opt() const PROCSIM_NOEXCEPT { return _name; }
    inline void set_name(const std::string& name) { _name = name; }

    /// Get the clock value this will crash if the clock value has not been resolved
    inline const std::shared_ptr<time::Clock> clock() const { return boost::get<std::shared_ptr<time::Clock>>(_clock); }

    /// Get the clock value, will either be a \c conf::Clock or a path.
    inline const boost::variant<std::shared_ptr<time::Clock>, std::string> clock_var() const PROCSIM_NOEXCEPT
    {
        return _clock;
    }

    /// Get the read count for this instance.
    inline const size_t readCount() const PROCSIM_NOEXCEPT { return _readCount; }

    /// How many cycles to write a value
    inline const size_t writeCount() const PROCSIM_NOEXCEPT { return _writeCount; }

    /// The offset this memory lives at in processor memory (in bytes)
    inline const optional<size_t> offset() const PROCSIM_NOEXCEPT { return _offset; }

    /// The size of this memory in bytes
    inline const optional<size_t> size() const PROCSIM_NOEXCEPT { return _size; }

    /// The Access pattern for this memory
    inline const Access::Set& access() const PROCSIM_NOEXCEPT { return _access; }

    /*!
     * Set the clock instance.
     * \param pclock \c conf::Clock instance to set
     */
    void set_clock(const std::shared_ptr<time::Clock>& pclock);

protected:
    /// Name of the memory unit
    optional<std::string> _name;

    /// How many cycles to read a value
    size_t _readCount;

    /// How many cycles to write a value
    size_t _writeCount;

    /// The offset this memory lives at in processor memory (in bytes)
    const optional<size_t> _offset;

    /// The size of this memory in bytes
    optional<size_t> _size;

    /// The Access pattern for this memory
    Access::Set _access;

private:
    /// Clock configuration
    boost::variant<std::shared_ptr<time::Clock>, std::string> _clock;

    friend std::ostream& operator<<(std::ostream& os, const MemoryUnit& mu);
};

export_smart_ptrs(MemoryUnit)

/*!
 * \brief Describes a unit of RAM memory
 */
class PROCSIM_EXPORT RAM PROCSIM_FINAL : public MemoryUnit
{

public:
    /// Name of `this` type, all subclasses of `ConfObj` must have this.
    static PROCSIM_CONSTEXPR const char TYPE_NAME[] = "RAM";

    /// Base constructor
    RAM(const MemoryUnit::Parameters& params);

    RAM(RAM&&)      = default;
    RAM(const RAM&) = default;
    RAM& operator=(const RAM&) = default;
    virtual ~RAM()             = default;

    /// Check for equality
    inline bool operator==(const RAM& other) const
    {
        return _access == other.access() && MemoryUnit::operator==(other);
    }
    inline bool check_eq(const MemoryUnit& other) const
    {
        return _access == other.access() && MemoryUnit::operator==(other);
    }
};

export_smart_ptrs(RAM)

/*!
 * \brief Describes a unit of ROM memory
 */
class PROCSIM_EXPORT ROM PROCSIM_FINAL : public MemoryUnit
{

public:
    /// Name of `this` type, all subclasses of `ConfObj` must have this.
    static PROCSIM_CONSTEXPR const char TYPE_NAME[] = "ROM";

    /// Base constructor
    ROM(const MemoryUnit::Parameters& params);

    ROM(ROM&&)      = default;
    ROM(const ROM&) = default;
    ROM& operator=(const ROM&) = default;
    virtual ~ROM()             = default;

    /// Check for equality
    inline bool operator==(const ROM& other) const
    {
        return _access == other.access() && MemoryUnit::operator==(other);
    }
    inline bool check_eq(const MemoryUnit& other) const
    {
        return _access == other.access() && MemoryUnit::operator==(other);
    }
};

export_smart_ptrs(ROM)

// Out of line operators
bool operator==(const RAM& lhs, const MemoryUnit& rhs);
bool operator==(const MemoryUnit& lhs, const RAM& rhs);

bool operator==(const ROM& lhs, const MemoryUnit& rhs);
bool operator==(const MemoryUnit& lhs, const ROM& rhs);

} // end namespace memory

PROCSIM_NAMESPACE_END

#endif // PROCSIM_MEMORY_MEMORYUNIT_HPP_
