#ifndef CONF_REGISTER_HPP_
#define CONF_REGISTER_HPP_

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

#include "MemoryUnit.hpp"

#include "procsim/Types.hpp"

#include <map>
#include <set>
#include <string>
#include <vector>

#include <boost/variant.hpp>

PROCSIM_NAMESPACE_START

namespace memory
{

class Register : public MemoryUnit
{

public:
    // FIXME Make this actually work
    /// Location map for joined registers
    typedef std::map<size_t, boost::variant<std::shared_ptr<Register>, std::string>> joined_map_t;

    /// Name of the class "Register"
    static const std::string TYPE_NAME;

    /*!
     * \brief Base ctor for memory units, this ctor is a convenience function
     * \param name Name of the Register
     * \param params Basic \c MemoryUnit::Parameters for a MemoryUnit
     * \param joined Optional joined parameter
     */
    Register(const MemoryUnit::Parameters&& params, const optional<joined_map_t>& joined = nullopt);

    /*!
     * \brief Creates a mapped register based on offsets and underlying registers.
     *
     * Creates a Register configured to be overlapping or a combination. For example, in the PROCSIM,
     * A + B = D where the map would be `{ 0 = A, 8 = B }`.
     *
     * The `readCount` and `writeCount` are calculated from the underlying `Register`s as the `max()`
     * of them.
     *
     * If no `access` is set, the default setting is to calculate the gcd of the access patterns of
     * the underlying registers. If the underlying set is empty, this is an `Error` as is an illegal
     * specification.
     *
     * The `size` of the register is calculated by adding the first bit index + last index + last
     * `Register`'s `size` to get the minimum size required which is then mapped to a valid `uint*`
     * type.
     *
     * There is no way to memory map a joined register, if the underlying components are valid, it
     * will "just work".
     *
     * \param joined Mapping of index to `conf::Register` where the underlying Registers are
     *               verified to be in valid positions. They may overlap or be separated.
     * \param access The access pattern for this shared register. `Access::Write` must be
     *               transitive, that is, you can not have underlying read-only registers and have
     *               write access. This applies for `Access::Read` but conversely.
     */
    static Register
    make_joined(const std::string& name, const joined_map_t& joined, const Access::Set& access = Access::Set());

    Register(Register&&) = default;
    Register(Register&)  = default;

    virtual ~Register() = default;

    inline bool operator==(const Register& other) const
    {
        return (this == &other || (MemoryUnit::operator==(other) && _joined == other._joined));
    }

    /// Get the width of the \c Register in bits.
    inline const size_t bitWidth() const { return (*_size) * 8; }

    /// Get the joined mapping if it exists
    const optional<joined_map_t>& joined() const { return _joined; }

protected:
    void joinedRegister(const std::shared_ptr<Register> pregister);

private:
    optional<joined_map_t> _joined;

    friend std::ostream& operator<<(std::ostream&, const Register&);
};

export_smart_ptrs(Register)

} // end namespace memory

PROCSIM_NAMESPACE_END

#endif // CONF_REGISTER_HPP_
