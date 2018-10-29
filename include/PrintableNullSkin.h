/** @file PrintableNullSkin.h
 *
 * @author Manuel Schiller <Manuel.Schiller@glasgow.ac.uk>
 * @date 2016-11-16
 *
 * @brief make underlying tuple printable for whoever needs it
 *
 * For copyright and license information, see the end of the file.
 */

#ifndef PRINTABLENULLSKIN_H
#define PRINTABLENULLSKIN_H

#include <sstream>

/// namespace to encapsulate SOA stuff
namespace SOA {
    /// hide implementation details
    namespace impl {
        template <std::size_t LEN> struct printer;
        /// print a tuple, recursion anchor
        template <>
        struct printer<std::size_t(0)> {
            template <typename T>
            void operator()(std::ostringstream&, const T&) const
            { }
        };
        /// print a tuple, specialisation for size 1
        template <>
        struct printer<std::size_t(1)> {
            template <typename T>
            void operator()(std::ostringstream& os, const T& t) const
            {
                printer<0>()(os, t);
                os << t.template get<0>();
            }
        };
        /// print a tuple, LEN is tuple size
        template <std::size_t LEN>
        struct printer {
            template <typename T>
            void operator()(std::ostringstream& os, const T& t) const
            {
                printer<LEN - 1>()(os, t);
                os << ", " << t.template get<LEN - 1>();
            }
        };
    }
    
    /// make tuple convertible to string
    template <typename T>
    struct PrintableNullSkin : T
    {
        /// constructor - forward to underlying proxy
        using T::T;
        /// assignment operator - forward to underlying proxy
        using T::operator=;
    
        /// conversion to string
        operator std::string() const {
            std::ostringstream os;
            impl::printer<T::parent_type::fields_typelist::size()>()(
                    os, *this);
            return os.str();
        }
    };
    
    //// operator<< on ostream for a PrintableNullSkin<T>
    template <typename T>
    std::ostream& operator<<(std::ostream& os,
            const PrintableNullSkin<T>& printable)
    { return os << "{" << std::string(printable) << "}"; }
} // namespace SOA

#endif // PRINTABLENULLSKIN_H

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
 */

// vim: sw=4:tw=78:ft=cpp:et
