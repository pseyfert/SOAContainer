/** @file SOAIterator.h
 *
 * @brief iterator for SOA containers and views.
 *
 * @author Manuel Schiller <Manuel.Schiller@glasgow.ac.uk>
 * @date 2018-04-10
 *
 * For copyright and license information, see the end of the file.
 */

#ifndef SOAITERATOR_H
#define SOAITERATOR_H

#include <ostream>
#include <iterator>
#include <utility>

namespace SOA {
    template < template <typename...> class CONTAINER,
        template <typename> class SKIN, typename... FIELDS>
    class _Container;

    template <typename POSITION, bool ISCONST>
    class Iterator : protected POSITION {
    private:
        template < template <typename...> class CONTAINER,
            template <typename> class SKIN, typename... FIELDS>
        friend class _Container;
        friend class Iterator<POSITION, !ISCONST>;

        using POSITION::stor;
        using POSITION::idx;
        // "magic" constructor to be used by SOA container classes only
        using POSITION::POSITION;

    public:
        using parent_type = typename POSITION::parent_type;
        using iterator_category = std::random_access_iterator_tag;
        using reference = typename std::conditional<ISCONST,
              typename parent_type::const_reference,
              typename parent_type::reference>::type;
        using size_type = typename parent_type::size_type;
        using difference_type = typename parent_type::difference_type;
        using value_type = typename parent_type::value_type;
        using pointer = Iterator<POSITION, ISCONST>;
        using const_pointer = Iterator<POSITION, true>;

        // test for nullness
        constexpr explicit operator bool() const noexcept
        { return stor() && (idx() < std::get<0>(*stor()).size()); }
        // test for non-nullness
        constexpr bool operator!() const noexcept
        { return !bool(*this); }

        // deference/index/operator->
        constexpr reference operator*() const noexcept
        { return reference{ stor(), idx() }; }
        reference* operator->() noexcept
        { return reinterpret_cast<reference*>(this); }
        constexpr reference operator[](size_type ofs) const noexcept
        { return *((*this) + ofs); }

        // pointers convert to const_pointers, but not vice-versa
        constexpr operator const_pointer() const noexcept
        { return const_pointer{ stor(), idx() }; }

        /// pointer arithmetic
        pointer& operator++() noexcept { ++idx(); return *this; }
        pointer& operator--() noexcept { --idx(); return *this; }
        pointer operator++(int) noexcept
        { auto retVal(*this); ++idx(); return retVal; }
        pointer operator--(int) noexcept
        { auto retVal(*this); --idx(); return retVal; }

        pointer& operator+=(difference_type inc) noexcept
        { idx() += inc; return *this; }
        pointer& operator-=(difference_type inc) noexcept
        { idx() -= inc; return *this; }

        // distance between iterators
        constexpr difference_type operator-(const pointer& p) const noexcept
        { return idx() - p.idx(); }

        template <bool ISCONST2>
        constexpr bool operator==(const Iterator<POSITION, ISCONST2>& q) const
                noexcept
        {
            return POSITION::operator==(static_cast<const POSITION&>(q));
        }
        template <bool ISCONST2>
        constexpr bool operator!=(const Iterator<POSITION, ISCONST2>& q) const
                noexcept
        {
            return POSITION::operator!=(static_cast<const POSITION&>(q));
        }
        template <bool ISCONST2>
        constexpr bool operator<(const Iterator<POSITION, ISCONST2>& q) const
                noexcept
        {
            return POSITION::operator<(static_cast<const POSITION&>(q));
        }
        template <bool ISCONST2>
        constexpr bool operator>(const Iterator<POSITION, ISCONST2>& q) const
                noexcept
        {
            return POSITION::operator>(static_cast<const POSITION&>(q));
        }
        template <bool ISCONST2>
        constexpr bool operator<=(const Iterator<POSITION, ISCONST2>& q) const
                noexcept
        {
            return POSITION::operator<=(static_cast<const POSITION&>(q));
        }
        template <bool ISCONST2>
        constexpr bool operator>=(const Iterator<POSITION, ISCONST2>& q) const
                noexcept
        {
            return POSITION::operator>=(static_cast<const POSITION&>(q));
        }

        friend std::ostream& operator<<(std::ostream& os, const pointer& p)
        {
            os << "iterator{ stor=" << p.stor() << ", pos=" << p.idx() << " }";
            return os;
        }

        /// convert SOA iterator into iterator for given field
        template <typename FIELD>
        constexpr auto
        for_field() const noexcept -> typename std::conditional<
                ISCONST,
                decltype(std::get<parent_type::template memberno<FIELD>()>(
                                 *std::declval<POSITION>().stor())
                                 .cbegin() +
                         std::declval<POSITION>().idx()),
                decltype(std::get<parent_type::template memberno<FIELD>()>(
                                 *std::declval<POSITION>().stor())
                                 .begin() +
                         std::declval<POSITION>().idx())>::type
        {
            return std::get<parent_type::template memberno<FIELD>()>(*stor())
                           .begin() +
                   idx();
        }
    };

    /// iterator + integer constant (or similar)
    template <typename POSITION, bool CONST, typename INC>
    constexpr typename std::enable_if<std::is_convertible<
              typename Iterator<POSITION, CONST>::difference_type, INC>::value,
    Iterator<POSITION, CONST> >::type operator+(
            const Iterator<POSITION, CONST>& pos, INC inc) noexcept
    {
        return Iterator<POSITION, CONST>(pos) +=
               typename Iterator<POSITION, CONST>::difference_type(inc);
    }

    /// integer constant (or similar) + iterator
    template <typename POSITION, bool CONST, typename INC>
    constexpr typename std::enable_if<std::is_convertible<
              typename Iterator<POSITION, CONST>::difference_type, INC>::value,
    Iterator<POSITION, CONST> >::type operator+(INC inc,
            const Iterator<POSITION, CONST>& pos) noexcept
    {
        return Iterator<POSITION, CONST>(pos) +=
               typename Iterator<POSITION, CONST>::difference_type(inc);
    }

    /// iterator - integer constant (or similar)
    template <typename POSITION, bool CONST, typename INC>
    constexpr typename std::enable_if<std::is_convertible<
              typename Iterator<POSITION, CONST>::difference_type, INC>::value,
    Iterator<POSITION, CONST> >::type operator-(
            const Iterator<POSITION, CONST>& pos, INC inc) noexcept
    {
        return Iterator<POSITION, CONST>(pos) -=
               typename Iterator<POSITION, CONST>::difference_type(inc);
    }

} // namespace SOA

#endif // SOAITERATOR_H

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
