/* @file SOADressedTuple.h
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-05-09
 */

#ifndef SOADRESSEDTUPLE_H
#define SOADRESSEDTUPLE_H

#include <tuple>

/// namespace to encapsulate SOA stuff
namespace SOA {
    /** @brief dress std::tuple with the get interface of SOAObjectProxy
     *
     * @author Manuel Schiller <Manuel.Schiller@cern.ch>
     * @date 2015-05-09
     *
     * @tparam TUPLE        an instantiation of std::tuple
     * @tparam CONTAINER    underlying SOAContainer
     */
    template <typename TUPLE, typename CONTAINER>
    struct DressedTuple : TUPLE
    {
        /// convenience typedef
        using self_type = DressedTuple<TUPLE, CONTAINER>;

        /// for everything else, use TUPLE's constructor
        using TUPLE::TUPLE;
        // for anything deriving from TUPLE, the same holds
        template <typename T, typename DUMMY = typename std::enable_if<
            std::is_base_of<TUPLE, T>::value>::type>
        DressedTuple(T&& t, DUMMY* = nullptr) noexcept(noexcept(TUPLE(
                            std::forward<T>(t)))) : TUPLE(std::forward<T>(t))
        {}
        /// for everything else, use TUPLE's assignment operators
        using TUPLE::operator=;
        // for anything deriving from TUPLE, the same holds
        template <typename T>
        typename std::enable_if<std::is_base_of<TUPLE, T>::value, self_type>::type&
        operator=(T&& t) noexcept(noexcept(TUPLE::operator=(
                            std::forward<T>(t))))
        { TUPLE::operator=(std::forward<T>(t)); return *this; }

        /// provide the member function template get interface of proxies
        template<typename CONTAINER::size_type MEMBERNO>
        auto get() noexcept -> decltype(std::get<MEMBERNO>(std::declval<self_type&>()))
        { return std::get<MEMBERNO>(*this); }

        /// provide the member function template get interface of proxies
        template<typename CONTAINER::size_type MEMBERNO>
        auto get() const noexcept -> decltype(std::get<MEMBERNO>(
                    std::declval<const self_type&>()))
        { return std::get<MEMBERNO>(*this); }

        /// provide the member function template get interface of proxies
        template<typename MEMBER, typename CONTAINER::size_type MEMBERNO =
            CONTAINER::template memberno<MEMBER>()>
        auto get() noexcept -> decltype(std::get<MEMBERNO>(std::declval<self_type&>()))
        {
            static_assert(CONTAINER::template memberno<MEMBER>() ==
                    MEMBERNO, "Called with wrong template argument(s).");
            return std::get<MEMBERNO>(*this);
        }

        /// provide the member function template get interface of proxies
        template<typename MEMBER, typename CONTAINER::size_type MEMBERNO =
            CONTAINER::template memberno<MEMBER>()>
        auto get() const noexcept -> decltype(std::get<MEMBERNO>(
                    std::declval<const self_type&>()))
        {
            static_assert(CONTAINER::template memberno<MEMBER>() ==
                    MEMBERNO, "Called with wrong template argument(s).");
            return std::get<MEMBERNO>(*this);
        }
    };
} // namespace SOA

#endif // SOADRESSEDTUPLE_H

// vim: sw=4:tw=78:ft=cpp:et
