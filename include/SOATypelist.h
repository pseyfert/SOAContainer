/** @file SOATypelist.h
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-10
 * - initial related
 *
 * @author Manuel Schiller <Manuel.Schiller@glasgow.ac.uk>
 * @date 2016-11-17
 * - rewrite to use better coding, more C++11 constructs
 */

#ifndef SOATYPELIST_H
#define SOATYPELIST_H

#include <cstdint>
#include <type_traits>

/// namespace for typelist type used by SOAContainer and related utilities
namespace SOATypelist {
    // forward declarations
    namespace typelist_impl { struct empty_typelist; }
    template <typename HEAD = typelist_impl::empty_typelist, typename... ARGS>
    struct typelist;

    namespace typelist_impl {
    /// the empty typelist
    struct empty_typelist {
        /// typelist empty?
        constexpr static bool empty() noexcept { return true; }
        /// size of typelist
        constexpr static std::size_t size() noexcept { return 0; }
        /// no types at any indices!
        template <std::size_t IDX> struct at {};
    };

    /// little switch for typelist
    template <std::size_t LEN, typename... ARGS> struct __typelist {
        typedef typelist<ARGS...> type;
    };
    /// little switch for typelist (specialisation)
    template <typename... ARGS> struct __typelist<0, ARGS...> {
        typedef empty_typelist type;
    };
    } // namespace typelist_impl

    template <typename HEAD, typename... ARGS>
    struct typelist {
        /// first element
        typedef HEAD head_type;
        /// typelist empty?
        constexpr static bool empty() noexcept
        { return std::is_same<HEAD, typelist_impl::empty_typelist>::value; }
        /// size of typelist
        constexpr static std::size_t size() noexcept
        { return (!empty()) + sizeof...(ARGS); }
        /// tail typelist
        typedef typename typelist_impl::__typelist<size(), ARGS...>::type
        tail_types;
        /// return type at index IDX
        template <std::size_t IDX, bool PASTEND =
            (IDX >= typelist<HEAD, ARGS...>::size()), int DUMMY = 0>
        struct at;
        /// specialisation: short-circuit reads past end of list early
        template <std::size_t IDX, int DUMMY> struct at<IDX, true, DUMMY> {};
        /// specialisation: specialisation for reading types at valid indices
        template <std::size_t IDX, int DUMMY> struct at<IDX, false, DUMMY>
        { typedef typename tail_types::template at<IDX - 1>::type type; };
        /// specialisation: specialisation for reading types at valid indices
        template <int DUMMY> struct at<0, false, DUMMY>
        { typedef head_type type; };
        // make sure we construct only valid typelists
        static_assert(!empty() || (empty() && 0 == size()),
                "typelist: head empty_typelist with non-empty tail not allowed!");
    };

    // check basic properties to validate implementation
    static_assert(typelist<>::empty(), "implementation error");
    static_assert(typelist<>::size() == 0, "implementation error");
    static_assert(!typelist<int>::empty(), "implementation error");
    static_assert(typelist<int>::size() == 1, "implementation error");
    static_assert(std::is_same<typelist<int>::at<0>::type, int>::value,
            "implementation error");
    static_assert(!typelist<int, bool>::empty(), "implementation error");
    static_assert(typelist<int, bool>::size() == 2, "implementation error");
    static_assert(std::is_same<typelist<int, bool>::at<0>::type, int>::value,
            "implementation error");
    static_assert(std::is_same<typelist<int, bool>::at<1>::type, bool>::value,
            "implementation error");

    /// return type at index idx in typelist TL
    template <typename TL, std::size_t idx>
    struct at { typedef typename TL::template at<idx>::type type; };
}

#endif // SOATYPELIST_H

// vim: sw=4:tw=78:ft=cpp
