/** @file PrintableNullSkin.h
 *
 * @author Manuel Schiller <Manuel.Schiller@glasgow.ac.uk>
 * @date 2016-11-16
 * 
 * @brief make underlying tuple printable for whoever needs it
 */

#ifndef PRINTABLENULLSKIN_H
#define PRINTABLENULLSKIN_H

#include <sstream>

/// hide implementation details
namespace {
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
class PrintableNullSkin : public T
{
    public:
        /// constructor - forward to underlying proxy
        template <typename... ARGS>
        PrintableNullSkin(ARGS&&... args)
	    // ordinarily, we would like to have the following noexcept
	    // specification here:
            //
            // noexcept(noexcept(T(std::forward<ARGS>(args)...)))
            //
	    // however, gcc 4.9 and clang 3.5 insist that T's
	    // constructor is protected and refuse the code in the exception
	    // specification (despite the fact that it's perfectly legal to
	    // call that very constructor in the initializer list below
	    // because T is a friend of T)
            : T(std::forward<ARGS>(args)...) { }

        /// assignment operator - forward to underlying proxy
        template <typename ARG>
        PrintableNullSkin<T>& operator=(const ARG& arg) noexcept(noexcept(
                    static_cast<T*>(nullptr)->operator=(arg)))
        { T::operator=(arg); return *this; }

        /// move assignment operator - forward to underlying proxy
        template <typename ARG>
        PrintableNullSkin<T>& operator=(ARG&& arg) noexcept(noexcept(
		    static_cast<T*>(nullptr)->operator=(std::move(arg))))
        { T::operator=(std::move(arg)); return *this; }

	/// conversion to string
	operator std::string() const {
	    std::ostringstream os;
	    printer<T::parent_type::fields_typelist::size()>()(os, *this);
	    return os.str();
	}
};

//// operator<< on ostream for a PrintableNullSkin<T>
template <typename T>
std::ostream& operator<<(std::ostream& os,
	const PrintableNullSkin<T>& printable)
{ return os << "{" << std::string(printable) << "}"; }

#endif // PRINTABLENULLSKIN_H

// vim: sw=4:tw=78:ft=cpp:et
