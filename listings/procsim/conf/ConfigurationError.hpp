#pragma once
#ifndef CONF_CONFIGURATION_ERROR_HPP_
#define CONF_CONFIGURATION_ERROR_HPP_

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

#include "procsim/util/StringIO.hpp"

#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

PROCSIM_NAMESPACE_START

namespace conf
{

class Error : public std::logic_error
{

public:
    Error(const Error& other) : std::logic_error(other), _keyPath(other._keyPath), _messages(other._messages) {}

    Error(const std::string& keyPath, const std::vector<std::string>& messages)
        : std::logic_error("Error " + keyPath + ":\n\t" + PROCSIM_N::stringio::join(messages, "\n\t"))
        , _keyPath(keyPath)
        , _messages(messages)
    {
    }

    virtual ~Error() PROCSIM_NOEXCEPT{};

    //!< The path where the issue arose
    const std::string& keyPath() { return _keyPath; }

    //!< Message explaining the error
    const std::vector<std::string>& messages() { return _messages; }

private:
    const std::string _keyPath;
    const std::vector<std::string> _messages;
};

} // end namespace conf

PROCSIM_NAMESPACE_END

#endif // CONF_CONFIGURATION_ERROR_HPP_
