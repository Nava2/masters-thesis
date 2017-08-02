#pragma once
#ifndef PROCSIM_CONF_SOL_IO_HPP_
#define PROCSIM_CONF_SOL_IO_HPP_

/*!
 * The following is part of the Procsim Library written and developed
 * by Kevin Brightwell under the supervision of Dr. Kenneth McIssac
 * and Dr. Roy Eagleson at Western University, Canada.
 *
 * \author Kevin Brightwell
 *
 * This software is provided as is and all rights are maintained by
 * Kevin Brightwell
 */

#include <iostream>

#include <sol.hpp>

#include <boost/assert.hpp>

namespace sol
{

inline std::ostream& operator<<(std::ostream& os, const type& type)
{

    switch (type)
    {
    case sol::type::nil:
        os << "nil";
        break;
    case sol::type::string:
        os << "string";
        break;
    case sol::type::number:
        os << "number";
        break;
    case sol::type::thread:
        os << "thread";
        break;
    case sol::type::boolean:
        os << "boolean";
        break;
    case sol::type::function:
        os << "function";
        break;
    case sol::type::userdata:
        os << "userdata";
        break;
    case sol::type::lightuserdata:
        os << "lightuserdata";
        break;
    case sol::type::table:
        os << "table";
        break;
    case sol::type::poly:
        os << "poly";
        break;
    default:
        BOOST_ASSERT_MSG(false, "Unknown type, adjust.");
        BOOST_UNREACHABLE_RETURN(os);
        break;
    }
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const object& obj)
{
    os << "object{type=" << obj.get_type() << "}";

    return os;
}

} // end namespace sol

#endif // PROCSIM_CONF_SOL_IO_HPP_