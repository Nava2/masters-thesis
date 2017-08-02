#pragma once
#ifndef PROCSIM_CONF_VALIDATE_HPP_
#define PROCSIM_CONF_VALIDATE_HPP_

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
#include "procsim/conf/Locale.hpp"
#include "procsim/util/StringIO.hpp"

#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include <boost/assert.hpp>

PROCSIM_NAMESPACE_START

namespace conf
{

/*!
 * Contains data passed between `ConfObj::Validate()` methods.
 */
class PROCSIM_EXPORT ValidateData : public std::enable_shared_from_this<ValidateData>
{

public:
    struct Msg
    {
        /// Type that caused this error
        const std::string type;

        /// Tag that caused the problem
        const std::unordered_set<std::string> tags;

        /// Message that will display
        const std::string message;
    };

    inline ValidateData(const std::vector<Msg>& errMsgs = {}, const std::unordered_set<std::string> usedTags = {})
        : _errMsgs(errMsgs), _usedTags(usedTags)
    {
    }

    ~ValidateData() = default;

    /// Add an error to the log
    ValidateData& error(const std::string& typeStr,
                        const std::unordered_set<std::string>& tags,
                        const std::string& msg) PROCSIM_NOEXCEPT;

    /// State that several tags were used
    template <typename... Args>
    inline ValidateData& tags(Args&... args)
    {
        this->_usedTags.insert({ args... });

        return *this;
    }

    /// Get the used tags
    inline const std::unordered_set<std::string>& tags() const PROCSIM_NOEXCEPT { return _usedTags; }

    /// Get the error messages that were found
    inline const std::vector<Msg>& errorMsgs() const PROCSIM_NOEXCEPT { return _errMsgs; }

    /// True if there have been no error messages
    inline const bool good() const PROCSIM_NOEXCEPT
    {
        // Do *not* check if tags has content, this is because some objects do not need to use tags because they're
        // functions
        // Thus, the tags are not always accurate.
        return _errMsgs.empty();
    }

    template <class T, typename = std::enable_if_t<std::is_nothrow_constructible<T>::value>>
    class PROCSIM_EXPORT StreamMsg PROCSIM_FINAL : public T
    {
    public:
        typedef typename T::end_t end_t;

        StreamMsg(StreamMsg&&) = default;

        // the "ended" marker is uniq so, don't allow copy, only move
        StreamMsg(const StreamMsg&) = delete;

        inline ~StreamMsg()
        {
            if (_ended)
            {
                BOOST_ASSERT_MSG(BOOST_LIKELY(!!_ended),
                                 "Message was not properly terminated before falling out of scope.");
            }
        }

        using T::operator<<;

        template <typename U, typename V>
        inline StreamMsg<T>& operator<<(std::tuple<U, V>&& t)
        {
            this->set_pair(std::get<0>(t), std::get<1>(t));
            return *this;
        }

        virtual ValidateData& operator<<(const end_t&) final
        {
            *_ended = true;

            BOOST_ASSERT_MSG(!_parent.expired(), "Parent pointer fell out of scope.");

            this->set_pair(locale::conf::type.name(), _type);

            // There aren't always tags, so don't always try to set them, otherwise it creates segfaults
            if (_tags.size() > 0)
            {
                this->set_pair(locale::conf::tag.name(), *(_tags.begin()));
                this->set_pair(locale::conf::tags.name(), stringio::join(_tags, ", ", "{", "}"));
            }

            this->ostream(*_msg_ss);

            T::operator<<(end_t{});

            auto pparent = _parent.lock();
            pparent->error(_type, _tags, _msg_ss->str());
            return *pparent;
        }

    private:
        inline StreamMsg(const std::weak_ptr<ValidateData> parent,
                         const std::shared_ptr<std::ostringstream>& msg_ss,
                         const std::string& type,
                         const std::unordered_set<std::string>& tags)
            : T(), _type(type), _tags(tags), _parent(parent), _msg_ss(msg_ss), _ended(new bool{ false })
        {
            BOOST_ASSERT_MSG(!parent.expired(), "Parent pointer expired before streaming.");
            BOOST_ASSERT(!!_msg_ss);
        }

        const std::string _type;
        const std::unordered_set<std::string> _tags;
        const std::weak_ptr<ValidateData> _parent;
        const std::shared_ptr<std::ostringstream> _msg_ss;

        /// signify that the operation completed, only checked if debugging.
        std::unique_ptr<bool> _ended;

        friend class ValidateData;
    };

    template <class T,
              typename = std::enable_if_t<std::is_base_of<locale::details_::message_t<T>, T>::value>,
              typename... Tags>
    StreamMsg<T> err(const std::string& type, Tags&... tags)
    {
        auto p_ss           = std::make_shared<std::ostringstream>();
        const auto tags_set = std::unordered_set<std::string>({ tags... });

        auto sm = StreamMsg<T>(shared_from_this(), p_ss, type, tags_set);
        return sm;
    }

private:
    /// Listing of error messages passed to ALL nodes
    std::vector<Msg> _errMsgs;
    std::unordered_set<std::string> _usedTags;
};

inline std::ostream& operator<<(std::ostream& os, const ValidateData::Msg& msg)
{
    os << msg.type << " @ ";

    if (msg.tags.size() > 0)
    {
        // create: {TAG1, TAG2}
        stringio::join(os, msg.tags, ", ", "{", "}");
    }

    os << ": " << msg.message;

    return os;
}

inline std::ostream& operator<<(std::ostream& os, const ValidateData& msg)
{
    os << "ValidateData{used_tags=";

    stringio::join(os, msg.tags(), ", ", "{", "}");
    os << ", msgs=";

    if (msg.errorMsgs().empty())
    {
        os << "{}";
    }
    else
    {
        stringio::join(os, msg.errorMsgs(), "', '", "{ '", "' }");
    }

    os << "}";

    return os;
}

} // end namespace conf

PROCSIM_NAMESPACE_END

#endif // PROCSIM_CONF_VALIDATE_HPP_
