/** @file SOAIteratorRange.h
 *
 * @brief a range given by two iterators
 *
 * @author Manuel Schiller <Manuel.Schiller@glasgow.ac.uk>
 * @date 2018-04-04
 *
 * For copyright and license information, see the end of the file.
 */
#ifndef SOA_ITERATOR_RANGE_H
#define SOA_ITERATOR_RANGE_H

#include <array>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "c++14_compat.h"

#ifndef __GNUC__
#define __builtin_expect(c, val) ((c))
#else // __GNUC__
// gnu/clang/... and friends have __builtin_expect
#endif // __GNUC__

namespace SOA {
    /// @brief tag to mark iterator ranges
    struct iterator_range_tag {};

    /** @brief your standard, dumb iterator range
     *
     * This class models an iterator range, i.e. the range between two elements
     * first and last, where first is included in the range, but last is not.
     *
     * The class supports the usual typedefs (iterator_category,
     * difference_type, size_type, value_type, reference, pointer, iterator,
     * and, if IT is at least a bidirectional iterator, also reverse_iterator).
     *
     * Supported operations are empty(), size(), operator[], at(), begin(),
     * end(), front(), back(), and, if IT is at least a bidirectional iterator,
     * rbegin(), rend().
     *
     * @tparam IT       type of iterator
     *
     * IT must be at least a forward iterator.
     */
    template <typename IT>
    class iterator_range : public SOA::iterator_range_tag {
        private:
            IT m_first, m_last; ///< two iterators

        public:
            /// iterator_category
            using iterator_category = typename std::iterator_traits<IT>::iterator_category;
            // check constraints on IT
            static_assert(std::is_base_of<std::forward_iterator_tag,
                    iterator_category>::value,
                    "IT must be at least a forward iterator");
            /// all the remaining usual typedefs
            using difference_type = typename std::iterator_traits<IT>::difference_type;
            using size_type = typename std::make_unsigned<difference_type>::type;
            using value_type = typename std::iterator_traits<IT>::value_type;
            using reference = typename std::iterator_traits<IT>::reference;
            using pointer = typename std::iterator_traits<IT>::pointer;
            using iterator = IT;
            using reverse_iterator = std::reverse_iterator<iterator>;

        private:
            // check if we can figure out if iterator points to contiguous memory
            template <typename T, std::size_t = 0, bool ISCONT = false>
            struct is_contiguous_iterator
                    : std::integral_constant<bool, ISCONT> {};
#if __cplusplus > 201703L
            static constexpr bool _is_contiguous() noexcept
            {
                return std::is_base_of<std::contiguous_iterator_tag,
                                       iterator_category>::value;
            }
#else // __cplusplus > 201703L
            static constexpr bool _is_contiguous() noexcept
            {
                return std::is_pointer<
                               typename std::remove_cv<IT>::type>::value;
            }
#endif // __cplusplus > 201703L

        public:
            /// is range a contiguous range?
            enum { is_contiguous = _is_contiguous() };

            /// construct from a pair of iterators
            template <typename ITFWD1, typename ITFWD2,
                      typename DUMMY1 = ITFWD1, typename DUMMY2 = ITFWD2,
                      typename = typename std::enable_if<
                              (std::is_constructible<IT, DUMMY1>::value ||
                               std::is_convertible<DUMMY1, IT>::value) &&
                              (std::is_constructible<IT, DUMMY2>::value ||
                               std::is_convertible<DUMMY2, IT>::value)>::type>
            constexpr explicit iterator_range(ITFWD1&& first, ITFWD2&& last)
                    : m_first(std::forward<ITFWD1>(first)),
                      m_last(std::forward<ITFWD2>(last))
            {}

            /// construct from another range (if iterators are convertible)
            template <typename JT, typename DUMMY = JT, typename = typename
                std::enable_if<std::is_constructible<IT, JT>::value ||
                std::is_convertible<JT, IT>::value>::type>
            constexpr iterator_range(const SOA::iterator_range<JT>& range) :
                iterator_range(range.begin(), range.end()) {}

            /// is range empty
            constexpr bool empty() const { return m_last == m_first; }
            /// size of range
            constexpr size_type size() const
            { return std::distance(m_first, m_last); }
            /// element at position idx
            template <typename SZ = std::size_t>
            constexpr typename std::enable_if<
                    std::is_integral<SZ>::value &&
                    std::is_base_of<std::random_access_iterator_tag,
                    iterator_category>::value,
            reference>::type operator[](SZ idx) const
            { return *(m_first + idx); }
            /// check access to element at idx (can throw!)
            template <typename SZ = std::size_t>
            typename std::enable_if<
                    std::is_integral<SZ>::value &&
                    std::is_base_of<std::random_access_iterator_tag,
                    iterator_category>::value,
            reference>::type at(SZ idx) const
            {
                if (__builtin_expect(std::size_t(idx) >= size(), false)) {
                    throw std::out_of_range("SOA::iterator_range::at("
                            "std::size_t): out of bounds");
                }
                return *(m_first + idx);
            }

            /// start of range
            constexpr iterator begin() const
            { return m_first; }
            /// end of range
            constexpr iterator end() const
            { return m_last; }
            /// start of reverse range
            template <typename DUMMY = typename std::enable_if<
                std::is_same<std::bidirectional_iterator_tag,
                iterator_category>::value>*>
            constexpr reverse_iterator rbegin(DUMMY = nullptr) const
            { return reverse_iterator{ m_last }; }
            /// end of reverse range
            template <typename DUMMY = typename std::enable_if<
                std::is_same<std::bidirectional_iterator_tag,
                iterator_category>::value>*>
            constexpr reverse_iterator rend(DUMMY = nullptr) const
            { return reverse_iterator{ m_first }; }
            /// access to first element
            constexpr reference front() const { return *begin(); }
            /// access to last element
            template <typename SZ = void*>
            typename std::enable_if<
                    std::is_same<SZ, void*>::value &&
                    !std::is_base_of<std::bidirectional_iterator_tag,
                    iterator_category>::value,
            reference>::type back(SZ = nullptr) const
            {
                auto p = begin();
                std::advance(p, size() - 1);
                return *p;
            }
            /// access to last element
            template <typename SZ = void*>
            typename std::enable_if<
                    std::is_same<SZ, void*>::value &&
                    std::is_base_of<std::bidirectional_iterator_tag,
                    iterator_category>::value &&
                    !std::is_base_of<std::random_access_iterator_tag,
                    iterator_category>::value,
            reference>::type back(SZ = nullptr) const
            { auto p = end(); --p; return *p; }
            /// access to last element
            template <typename SZ = void*>
            constexpr typename std::enable_if<
                    std::is_same<SZ, void*>::value &&
                    std::is_base_of<std::bidirectional_iterator_tag,
                    iterator_category>::value,
            reference>::type back(SZ = nullptr) const
            { return *(end() - 1); }

            /// access to underlying storage (if contiguous in memory)
            template <bool ISCONT = is_contiguous>
            typename std::enable_if<ISCONT, value_type*>::type data() noexcept
            { return &*begin(); }
            /// access to underlying storage (if contiguous in memory)
            template <bool ISCONT = is_contiguous>
            constexpr typename std::enable_if<ISCONT, const value_type*>::type
            data() const noexcept
            { return &*begin(); }
    };

    /// implementation detail
    namespace impl {
        /// type trait: is T a range that's contiguous in memory?
        template <typename T, typename = void>
        struct is_contiguous_range : std::false_type {};
        // pointers to or arrays of T are contiguous
        template <typename T>
        struct is_contiguous_range<T*, void> : std::true_type {};
        template <typename T>
        struct is_contiguous_range<const T*, void> : std::true_type {};
        template <typename T>
        struct is_contiguous_range<volatile T*, void> : std::true_type {};
        template <typename T>
        struct is_contiguous_range<const volatile T*, void> : std::true_type {
        };
        template <typename T, std::size_t N>
        struct is_contiguous_range<T[N], void> : std::true_type {};
        template <typename T, std::size_t N>
        struct is_contiguous_range<const T[N], void> : std::true_type {};
        template <typename T, std::size_t N>
        struct is_contiguous_range<volatile T[N], void> : std::true_type {};
        template <typename T, std::size_t N>
        struct is_contiguous_range<const volatile T[N], void>
                : std::true_type {};
        // anything that has a data() member is contiguous
        template <typename T>
        struct is_contiguous_range<
                T,
                std::void_t<decltype(std::declval<T&>().data() == nullptr)>>
                : std::true_type {};
        template <typename T>
        struct is_contiguous_range<std::initializer_list<T>, void>
                : std::true_type {};

        // simple unit test fo is_contiguous_range
        static_assert(!is_contiguous_range<int>::value,
                      "bug in is_contiguous_range");
        static_assert(is_contiguous_range<int*>::value,
                      "bug in is_contiguous_range");
        static_assert(is_contiguous_range<const int*>::value,
                      "bug in is_contiguous_range");
        static_assert(is_contiguous_range<volatile int*>::value,
                      "bug in is_contiguous_range");
        static_assert(is_contiguous_range<const volatile int*>::value,
                      "bug in is_contiguous_range");
        static_assert(is_contiguous_range<std::array<int, 3>>::value,
                      "bug in is_contiguous_range");
        static_assert(is_contiguous_range<int[3]>::value,
                      "bug in is_contiguous_range");
        static_assert(is_contiguous_range<std::array<int, 3>>::value,
                      "bug in is_contiguous_range");
        static_assert(is_contiguous_range<std::vector<int>>::value,
                      "bug in is_contiguous_range");
        static_assert(is_contiguous_range<std::initializer_list<int>>::value,
                      "bug in is_contiguous_range");

        constexpr std::false_type _is_contiguous_iterator(...) noexcept;
        template <typename T>
        constexpr std::true_type
        _is_contiguous_iterator(T* /* unused */) noexcept;
        template <typename T>
        constexpr std::true_type
        _is_contiguous_iterator(const T* /* unused */) noexcept;
        template <typename T>
        constexpr std::true_type
        _is_contiguous_iterator(volatile T* /* unused */) noexcept;
        template <typename T>
        constexpr std::true_type
        _is_contiguous_iterator(const volatile T* /* unused */) noexcept;
        template <typename C>
        constexpr is_contiguous_range<C> _is_contiguous_iterator(
                typename C::iterator /* unused */) noexcept;
        template <typename C>
        constexpr is_contiguous_range<C> _is_contiguous_iterator(
                typename C::const_iterator /* unused */) noexcept;
#if defined(__GLIBCXX__)
        // GNU's libstdc++'s vector and friends have iterators that are
        // not member classes of their container types, so extract in a
        // different manner
        template <typename P, typename C>
        constexpr is_contiguous_range<C> _is_contiguous_iterator(
                __gnu_cxx::__normal_iterator<P, C> /* unused */) noexcept;
#endif // defined(__GLIBCXX__)
#if defined(_LIBCPP_VERSION)
        template <typename IT>
        constexpr is_contiguous_range<IT> _is_contiguous_iterator(
               std::__wrap_iter<IT> /* unused */) noexcept;
#endif

        /// type trait: is T iterator type that refers to contiguous storage
        template <typename T>
        struct is_contiguous_iterator
                : decltype(_is_contiguous_iterator(std::declval<T>())) {};

        static_assert(!is_contiguous_iterator<int>::value,
                      "bug in is_contiguous_iterator");
        static_assert(is_contiguous_iterator<int*>::value,
                      "bug in is_contiguous_iterator");
        static_assert(is_contiguous_iterator<const int*>::value,
                      "bug in is_contiguous_iterator");
        static_assert(is_contiguous_iterator<volatile int*>::value,
                      "bug in is_contiguous_iterator");
        static_assert(is_contiguous_iterator<const volatile int*>::value,
                      "bug in is_contiguous_iterator");
        static_assert(
                is_contiguous_iterator<std::array<int, 3>::iterator>::value,
                "bug in is_contiguous_iterator");
        static_assert(is_contiguous_iterator<
                              std::array<int, 3>::const_iterator>::value,
                      "bug in is_contiguous_iterator");
        static_assert(
                is_contiguous_iterator<std::vector<int>::iterator>::value,
                "bug in is_contiguous_iterator");
        static_assert(is_contiguous_iterator<
                              std::vector<int>::const_iterator>::value,
                      "bug in is_contiguous_iterator");
        static_assert(is_contiguous_iterator<
                              std::initializer_list<int>::iterator>::value,
                      "bug in is_contiguous_iterator");
        static_assert(
                is_contiguous_iterator<
                        std::initializer_list<int>::const_iterator>::value,
                "bug in is_contiguous_iterator");
    } // namespace impl

    /** @brief build an iterator_range given two iterators
     *
     * @note make_iterator_range will try to erase they iterator type if the
     *
     * storage that underlies the container is contiguous.
     * @tparam IT               iterator type (at least forward iterators)
     * @tparam JT               iterator type (at least forward iterators)
     * @tparam RANGEHINT        optional range type hint (used for guessing
     *                          if iterator points to a range contiguous in
     *                          memory)
     *
     * @param first     start of range
     * @param last      one past end of range
     *
     * @returns iterator_range<IT> from [first, last(
     *
     * Specialization for non-contiguous storage.
     */
    template <typename RANGEHINT = void, typename IT, typename JT,
              typename = typename std::enable_if<
                      std::is_same<typename std::remove_cv<
                                           typename std::remove_reference<
                                                   IT>::type>::type,
                                   typename std::remove_cv<
                                           typename std::remove_reference<
                                                   JT>::type>::type>::value &&
                      !((SOA::impl::is_contiguous_iterator<IT>::value &&
                         SOA::impl::is_contiguous_iterator<JT>::value) ||
                        SOA::impl::is_contiguous_range<RANGEHINT>::value)>::
                      type>
    constexpr iterator_range<typename std::remove_reference<IT>::type>
    make_iterator_range(IT&& first, JT&& last)
    {
        return iterator_range<typename std::remove_reference<IT>::type>{
            std::forward<IT>(first), std::forward<JT>(last) };
    }
    /** @brief build an iterator_range given two iterators
     *
     * @note make_iterator_range will try to erase they iterator type if the
     *
     * storage that underlies the container is contiguous.
     * @tparam IT               iterator type (at least forward iterators)
     * @tparam JT               iterator type (at least forward iterators)
     * @tparam RANGEHINT        optional range type hint (used for guessing
     *                          if iterator points to a range contiguous in
     *                          memory)
     *
     * @param first     start of range
     * @param last      one past end of range
     *
     * @returns iterator_range<IT> from [first, last(
     *
     * Specialization for contiguous storage.
     */
     template <typename RANGEHINT = void, typename IT, typename JT,
              typename = typename std::enable_if<
                      std::is_same<typename std::remove_cv<
                                           typename std::remove_reference<
                                                   IT>::type>::type,
                                   typename std::remove_cv<
                                           typename std::remove_reference<
                                                   JT>::type>::type>::value &&
                      ((SOA::impl::is_contiguous_iterator<IT>::value &&
                         SOA::impl::is_contiguous_iterator<JT>::value) ||
                        SOA::impl::is_contiguous_range<RANGEHINT>::value)>::
                      type>
    constexpr iterator_range<decltype(
            &*std::declval<typename std::remove_reference<IT>::type>())>
    make_iterator_range(IT&& first, JT&& last)
    {
        return (first != last) ?
            iterator_range<decltype(
                &*std::declval<typename std::remove_reference<IT>::type>())>{
                &*first, &*last} :
            iterator_range<decltype(
                &*std::declval<typename std::remove_reference<IT>::type>())>{
                nullptr, nullptr };
    }
} // namespace SOA

#ifndef __GNUC__
#undef __builtin_expect // clean up
#endif // __GNUC__

#endif // SOA_ITERATOR_RANGE_H

/* Copyright (C) CERN for the benefit of the LHCb collaboration
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * In applying this licence, CERN does not waive the privileges and immunities
 * granted to it by virtue of its status as an Intergovernmental Organization
 * or submit itself to any jurisdiction.
 */

// vim: sw=4:tw=78:ft=cpp:et
