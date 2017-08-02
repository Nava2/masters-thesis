#pragma once
#ifndef PROCSIM_CONF_LOADER_HPP_
#define PROCSIM_CONF_LOADER_HPP_

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

#include "Validate.hpp"
#include "sol_io.hpp"

#include "procsim/Macros.hpp"
#include "procsim/TypeTraits.hpp"

#include <boost/assert.hpp>

#include <sol.hpp>

PROCSIM_NAMESPACE_START

namespace conf
{

namespace util
{
    template <typename T>
    inline sol::table to_table(sol::state& lua, const named_map<T>& umap)
    {
        sol::table table = lua.create_table(umap.size());

        for (const auto& p : umap)
        {
            table.add(p.first, p.second);
        }

        return table;
    }

    template <typename T>
    inline sol::table to_table(sol::state& lua, const std::unordered_set<T>& set)
    {
        sol::table table = lua.create_table(set.size());

        auto it = set.begin();
        for (size_t idx = 1; it != set.cend(); ++it)
        {
            table.add(idx, *it);
        }

        return table;
    }
}

namespace loader
{
    typedef size_t marker_t;

    typedef std::vector<std::pair<marker_t, std::string>> tracker_t;
}

template <typename V>
struct PROCSIM_EXPORT loader_base_t
{
};

namespace loader
{

    template <typename... Tn>
    struct LoadHelper;

    template <typename T, typename... Tn>
    struct LoadHelper<T, Tn...>
    {
        static inline void load(sol::state& lua,
                                const std::shared_ptr<ValidateData>& vd,
                                std::unordered_set<marker_t>& loaded,
                                loader::tracker_t* const loadOrder)
        {
            typedef loader_base_t<T> templ_t;

            if (loaded.find(templ_t::marker()) == loaded.cend())
            {
                // need to load it in reverse order
                LoadHelper<typename templ_t::deps_t>::load(lua, vd, loaded, loadOrder);

                if (loaded.find(templ_t::marker()) == loaded.cend())
                {
                    templ_t::load(lua, vd);
                    loaded.insert(templ_t::marker());
                    if (!!loadOrder)
                    {
                        loadOrder->push_back({ templ_t::marker(), templ_t::tag() });
                    }
                }
            }

            LoadHelper<Tn...>::load(lua, vd, loaded, loadOrder);
        }
    };

    template <typename... Tn>
    struct LoadHelper<std::tuple<Tn...>> : LoadHelper<Tn...>
    {
    };

    template <>
    struct LoadHelper<>
    {
        static inline void load(sol::state& lua,
                                const std::shared_ptr<ValidateData>& vd,
                                std::unordered_set<marker_t>& loaded,
                                loader::tracker_t* const loadOrder)
        {
            ignore(lua, vd, loaded, loadOrder);
        }
    };

} // end namespace loader

/// Loads the Types requested into the `sol::state` passed. This will load each type only once
/// regardless of the dependency graph created by the Types. The order of invocation is *not* known.
template <typename... Tn>
inline PROCSIM_EXPORT void
loadSol(sol::state* const lua, const std::shared_ptr<ValidateData>& vd, loader::tracker_t* const loadOrder = nullptr)
{
    BOOST_ASSERT(!!lua);
    BOOST_ASSERT(!!vd);

    std::unordered_set<loader::marker_t> loaded(10);

    loader::LoadHelper<Tn...>::load(
        std::forward<sol::state&>(*lua), std::forward<const std::shared_ptr<ValidateData>&>(vd), loaded, loadOrder);
}

#define _Marker(T)                                                        \
    static const char* tag() { return "MARKER#" #T; }                     \
    static inline const loader::marker_t marker()                         \
    {                                                                     \
        static const size_t HASH_VALUE = std::hash<std::string>()(tag()); \
        return HASH_VALUE;                                                \
    }                                                                     \
    typedef void __ignore_t

#define LoaderDef(T, ...)                                                                 \
    template <>                                                                           \
    struct PROCSIM_EXPORT loader_base_t<T>                                                \
    {                                                                                     \
        _Marker(T);                                                                       \
        typedef std::tuple<__VA_ARGS__> deps_t;                                           \
                                                                                          \
        static inline void load(sol::state& lua, const std::shared_ptr<ValidateData> vd); \
    };                                                                                    \
                                                                                          \
    inline void loader_base_t<T>::load(sol::state& lua, const std::shared_ptr<ValidateData> vd)

#define ExternLoader(T) extern template struct PROCSIM_N::conf::loader_base_t<T>

/// Create a loader def with no dependencies

#define LoaderDefS(T)                                                                     \
    template <>                                                                           \
    struct PROCSIM_EXPORT loader_base_t<T>                                                \
    {                                                                                     \
        _Marker(T);                                                                       \
        typedef std::tuple<> deps_t;                                                      \
                                                                                          \
        static inline void load(sol::state& lua, const std::shared_ptr<ValidateData> vd); \
    };                                                                                    \
                                                                                          \
    inline void loader_base_t<T>::load(sol::state& lua, const std::shared_ptr<ValidateData> vd)

#define NSLoaderDef(Name, ...) \
    struct PROCSIM_EXPORT Name \
    {                          \
    };                         \
    LoaderDef(Name, __VA_ARGS__)

#define NSLoaderDefS(Name)     \
    struct PROCSIM_EXPORT Name \
    {                          \
    };                         \
    LoaderDefS(Name)

} // end namespace conf

PROCSIM_NAMESPACE_END

#endif // PROCSIM_CONF_LOADER_HPP_
