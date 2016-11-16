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

        /// forward constructor calls to TUPLE's constructor(s)
        template <typename... ARGS>
        DressedTuple(ARGS&&... args) : TUPLE(std::forward<ARGS>(args)...) { }
        
        /// forward (copy) assignment to the TUPLE implementation
        template <typename ARG>
        self_type& operator=(const ARG& other) noexcept(noexcept(
                    static_cast<TUPLE*>(nullptr)->operator=(other)))
        { TUPLE::operator=(other); return *this; }

        /// forward (move) assignment to the TUPLE implementation
        template <typename ARG>
        self_type& operator=(ARG&& other) noexcept(noexcept(
                    static_cast<TUPLE*>(nullptr)->operator=(std::move(other))))
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
        template<typename MEMBER>
        auto get() noexcept -> decltype(std::get<
                CONTAINER::template memberno<MEMBER>()>(
                    *static_cast<self_type*>(nullptr)))
        { return std::get<CONTAINER::template memberno<MEMBER>()>(*this); }

        /// provide the member function template get interface of proxies
        template<typename MEMBER>
        auto get() const noexcept -> decltype(
                std::get<CONTAINER::template memberno<MEMBER>()>(
                    *static_cast<const self_type*>(nullptr)))
        { return std::get<CONTAINER::template memberno<MEMBER>()>(*this); }
};

#endif // SOADRESSEDTUPLE_H

// vim: sw=4:tw=78:ft=cpp
