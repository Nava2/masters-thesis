#pragma once
#ifndef PROCSIM_CONF_ENCODINGCONF_HPP_
#define PROCSIM_CONF_ENCODINGCONF_HPP_

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

#include "Loader.hpp"

#include "procsim/Types.hpp"

#include "procsim/encoding/Code.hpp"
#include "procsim/encoding/Operand.hpp"
#include "procsim/encoding/Primitives.hpp"

#include <array>
#include <memory>
#include <type_traits>

#include <boost/assert.hpp>

#include <sol.hpp>

PROCSIM_NAMESPACE_START

namespace conf
{

namespace Primitives_details_
{

    static const std::array<bits, 10> SHORTHAND_SIZES{ { bits{ 2 },
                                                         bits{ 3 },
                                                         bits{ 4 },
                                                         bits{ 5 },
                                                         bits{ 6 },
                                                         bits{ 7 },
                                                         bits{ 8 },
                                                         bits{ 16 },
                                                         bits{ 32 },
                                                         bits{ 64 } } };

    namespace enc = PROCSIM_N::encoding;

    const bits idxFromObj(const std::shared_ptr<ValidateData>& vd, const std::string tag, const sol::object obj);

} // end namespace Primitives_details_

namespace TypedNumber_details_
{
    namespace lambdas
    {
        namespace enc = PROCSIM_N::encoding;

        inline PROCSIM_CONSTEXPR enc::TypedNumber ctor(const enc::SignType sign, const bits::value_t width)
        {
            return enc::TypedNumber(sign, bits{ width });
        }

        inline decltype(auto) get_width(const enc::TypedNumber& n) { return n.width.value; }

        inline decltype(auto) get_idx_mask(const enc::TypedNumber& n, const bits::value_t idx)
        {
            return n.idx_mask<uint64_t>(bits{ idx });
        }
    }
}

LoaderDefS(encoding::TypedNumber)
{
    ignore(vd);

    using namespace TypedNumber_details_::lambdas;
    namespace enc = PROCSIM_N::encoding;

    lua.new_enum("SignType", "Signed", enc::SignType::Signed, "Unsigned", enc::SignType::Unsigned);

    // clang-format off
    sol::usertype<enc::TypedNumber> utype(
            sol::call_constructor, &ctor
            , "sign", &enc::TypedNumber::sign
            , "width", &get_width
            , "mask", &enc::TypedNumber::mask<uint64_t>
            , "idx_mask", &get_idx_mask);
    lua.set_usertype(utype);
    // clang-format on
}

namespace TypedValue_lambdas_
{
    namespace enc = PROCSIM_N::encoding;
}

LoaderDef(encoding::TypedValue, encoding::TypedNumber)
{
    ignore(vd);
    namespace enc = PROCSIM_N::encoding;

    // clang-format off
    sol::usertype<enc::TypedValue> ut(
            sol::constructors<sol::types<const enc::TypedNumber, const uint64_t>>{}
            , "type", &enc::TypedValue::type
            , "value", &enc::TypedValue::value
        );
    lua.set_usertype(ut);
    // clang-format on
}
namespace TypedIndexedNumber_details_
{
    namespace enc = PROCSIM_N::encoding;

    enc::TypedIndexedNumber
    checkedTypeNumber(const std::shared_ptr<ValidateData>& vd, const enc::TypedNumber type, const bits idx);

    struct tyidx_shorthand_fn
    {
        inline tyidx_shorthand_fn(const std::shared_ptr<ValidateData>& vd, const enc::TypedNumber type)
            : type(type), _vd(vd)
        {
        }

        inline tyidx_shorthand_fn(const enc::TypedNumber type) : type(type), _vd(nullptr) {}

        inline tyidx_shorthand_fn(const tyidx_shorthand_fn&) = default;
        inline tyidx_shorthand_fn& operator=(tyidx_shorthand_fn&) = default;

        const enc::TypedNumber type;

        inline bool operator==(const tyidx_shorthand_fn& other) const
        {
            return (this == &other) || (type == other.type);
        }

        enc::TypedIndexedNumber operator()(const uint8_t idx) const;

        operator std::string() const PROCSIM_NOEXCEPT;

    private:
        const std::shared_ptr<ValidateData> _vd;
    };

    namespace lambdas
    {

        inline PROCSIM_CONSTEXPR enc::TypedIndexedNumber ctor(const bits::value_t idx, const enc::TypedNumber type)
        {
            return enc::TypedIndexedNumber(bits{ idx }, type);
        }

        inline PROCSIM_CONSTEXPR decltype(auto) get_idx(const enc::TypedIndexedNumber& n) { return n.index.value; }
    }
}

LoaderDef(encoding::TypedIndexedNumber, encoding::TypedValue, encoding::TypedNumber)
{
    using namespace encoding;
    using namespace Primitives_details_;

    using namespace TypedIndexedNumber_details_;
    using namespace TypedIndexedNumber_details_::lambdas;

    // clang-format off
    sol::usertype<TypedIndexedNumber> utype(
            sol::call_constructor, &ctor
            , "index", &get_idx
            , "type", &TypedIndexedNumber::type
            , "mask", &TypedIndexedNumber::mask
            , "extract", &TypedIndexedNumber::extract
            , "isSigned", &TypedIndexedNumber::isSigned);
    lua.set_usertype(utype);

    sol::usertype<tyidx_shorthand_fn> tyidx_utype(sol::call_constructor, sol::no_constructor
                                                  , sol::meta_function::call_function, &tyidx_shorthand_fn::operator()
                                                  , sol::meta_function::to_string, &tyidx_shorthand_fn::operator std::string);
    lua.set_usertype(tyidx_utype);
    // clang-format on

    // Helpers for TypedIndexedNumber
    lua.set("flag", tyidx_shorthand_fn(vd, { SignType::Unsigned, bytes{ 1 } }));

    for (const auto w : Primitives_details_::SHORTHAND_SIZES)
    {
        lua["u" + w.value_str()] = tyidx_shorthand_fn(vd, { SignType::Unsigned, w });
        lua["i" + w.value_str()] = tyidx_shorthand_fn(vd, { SignType::Signed, w });
    }

    // clang-format off
    lua.set_function("uint",
                     sol::overload([vd](const bits::value_t width, const bits::value_t idx) {
                                        return checkedTypeNumber(vd, { SignType::Unsigned, bits{ width } }, bits{ idx });
                                    }
                                   // Get a curried function
                                   , [vd](const bits::value_t width) -> tyidx_shorthand_fn {
                                        return tyidx_shorthand_fn(vd, { SignType::Unsigned, bits{ width } });
                                   })
        );

    lua.set_function("int",
                     sol::overload([vd](const bits::value_t width, const bits::value_t idx) {
                                        return checkedTypeNumber(vd, { SignType::Signed, bits{ width } }, bits{ idx });
                                    }
                                                         // Get a curried function
                                    , [vd](const bits::value_t width) -> tyidx_shorthand_fn {
                                        return tyidx_shorthand_fn(vd, { SignType::Signed, bits{ width } });
                                    })
        );
    // clang-format on
}

NSLoaderDef(Primitives, encoding::TypedIndexedNumber, encoding::TypedValue, encoding::TypedNumber) { ignore(lua, vd); }

// Definition and Extraction Maps

namespace Code_details_
{
    using namespace encoding;

    std::shared_ptr<Code> alts_def(const std::unordered_map<std::string, uint64_t>& altTable,
                                   const Definition& definition,
                                   const std::shared_ptr<ValidateData> vd);
}

LoaderDef(encoding::Code, Primitives)
{
    using namespace encoding;
    using namespace Code_details_;

    // clang-format off
    sol::usertype<Code> code_ut(
        sol::call_constructor, sol::no_constructor
        , "new", sol::overload(
            [vd](const std::unordered_map<std::string, uint64_t>& altTable, const Definition& definition) {
                return alts_def(altTable, definition, vd);
            }
            , [](const uint64_t constant) -> decltype(auto) { 
                return std::make_shared<Code>(constant); 
            }
            , [](const uint64_t constant, const Definition& encodedVars) -> decltype(auto) {
                return std::make_shared<Code>(constant, encodedVars);
            })
        , "minRep", sol::property(&Code::minRep)
        , "mask", sol::property(&Code::mask)
        , "check", &Code::check);
    lua.set_usertype(code_ut);
    // clang-format on
}

namespace Operand_details_
{
    using namespace encoding;

    inline const encoding::TypedNumber fn_to_type(const std::function<TypedIndexedNumber(const bits::value_t)> fn)
    {
        BOOST_ASSERT(!!fn);

        // use 63 because it's always > width - 1
        return fn(63).type;
    }

    /// Compute the width in bits of the Definition
    const bits compute_def_width(const Definition& def);

    std::shared_ptr<Operand>
    with_checks(const std::shared_ptr<ValidateData>& vd, const bits width, const Definition& def);
}

LoaderDef(encoding::Operand, Primitives)
{
    using namespace encoding;
    using namespace Operand_details_;

    typedef std::function<TypedIndexedNumber(const bits::value_t)> shorthand_fn_t;

    // clang-format off
    sol::usertype<Operand> utype(sol::call_constructor, sol::no_constructor
            , "new", sol::overload(
                [vd](const shorthand_fn_t& fn, const Definition& definition) -> std::shared_ptr<Operand> {
                    const auto width = fn_to_type(fn).width;

                    if (!vd->good())
                    { // bad definition
                        return nullptr;
                    }

                    return with_checks(vd, width, definition);
                }
                , [vd](const bits::value_t width, const Definition& def) -> std::shared_ptr<Operand> {

                    if (!vd->good())
                    { // bad definition
                        return nullptr;
                    }

                    return with_checks(vd, bits{ width }, def);
                })
        , "width", sol::property(&Operand::width)
        , "minRep", sol::property(&Operand::minRep)
        , "mask", sol::property(&Operand::mask)
    );
    lua.set_usertype(utype);
    // clang-format on
}

NSLoaderDef(Encoding, Primitives, encoding::Code, encoding::Operand) {}

} // end namespace conf

PROCSIM_NAMESPACE_END

#endif // PROCSIM_CONF_ENCODINGCONF_HPP_
