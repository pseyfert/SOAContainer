/* @file SOADressedTuple.h
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-05-09
 */

#ifndef SOADRESSEDTUPLE_H
#define SOADRESSEDTUPLE_H

#include <tuple>

/** @brief dress std::tuple with the get interface of SOAObjectProxy
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-05-09
 *
 * @tparam TUPLE        an instantiation of std::tuple
 * @tparam CONTAINER    underlying SOAContainer
 */
template <typename TUPLE, typename CONTAINER>
class DressedTuple : public TUPLE
{
    public:
        /// convenience typedef
        typedef DressedTuple<TUPLE, CONTAINER> self_type;

        /// use TUPLE's constructors if possible
        using TUPLE::TUPLE;
        /// use TUPLE's assignment operators if possible
        using TUPLE::operator=;

        /// forward to TUPLE's constructor(s) if constructible from args
        template <typename... ARGS,
                 typename std::enable_if<
                     std::is_constructible<TUPLE, ARGS...>::value,
                    int>::type = 0>
        constexpr DressedTuple(ARGS&&... args) :
            TUPLE(std::forward<ARGS>(args)...) { }

        /// forward to TUPLE's copy assignment if assignable from args
        template <typename ARG, typename std::enable_if<
            std::is_assignable<TUPLE, const ARG&>::value
            >::type = 0>
        self_type& operator=(const ARG& other) noexcept(noexcept(
                    std::declval<TUPLE>().operator=(other)))
        { TUPLE::operator=(other); return *this; }

        /// forward to TUPLE's move assignment if assignable from args
        template <typename ARG, typename std::enable_if<
            std::is_assignable<TUPLE, ARG&&>::value
            >::type = 0>
        self_type& operator=(ARG&& other) noexcept(noexcept(
                    std::declval<TUPLE>().operator=(std::move(other))))
        { TUPLE::operator=(std::move(other)); return *this; }

        /// provide the member function template get interface of proxies
        template<typename CONTAINER::size_type MEMBERNO>
        auto get() noexcept -> decltype(std::get<MEMBERNO>(
                    *static_cast<self_type*>(nullptr)))
        { return std::get<MEMBERNO>(*this); }

        /// provide the member function template get interface of proxies
        template<typename CONTAINER::size_type MEMBERNO>
        auto get() const noexcept -> decltype(std::get<MEMBERNO>(
                    *static_cast<const self_type*>(nullptr)))
        { return std::get<MEMBERNO>(*this); }

        /// provide the member function template get interface of proxies
        template<typename MEMBER, typename CONTAINER::size_type MEMBERNO =
            CONTAINER::template memberno<MEMBER>()>
        auto get() noexcept -> decltype(std::get<MEMBERNO>(
                    *static_cast<self_type*>(nullptr)))
        {
            static_assert(CONTAINER::template memberno<MEMBER>() ==
                    MEMBERNO, "Called with wrong template argument(s).");
            return std::get<MEMBERNO>(*this);
        }

        /// provide the member function template get interface of proxies
        template<typename MEMBER, typename CONTAINER::size_type MEMBERNO =
            CONTAINER::template memberno<MEMBER>()>
        auto get() const noexcept -> decltype(std::get<MEMBERNO>(
                    *static_cast<const self_type*>(nullptr)))
        {
            static_assert(CONTAINER::template memberno<MEMBER>() ==
                    MEMBERNO, "Called with wrong template argument(s).");
            return std::get<MEMBERNO>(*this);
        }
};

#endif // SOADRESSEDTUPLE_H

// vim: sw=4:tw=78:ft=cpp:et
