#include <tuple>
#include <vector>
#include <iostream>
#include "include/SOATypelist.h"
#include "include/SOAContainer.h"
#include "include/PrintableNullSkin.h"

/* library code  */
/** @brief class from which a field tag derives
 *
 * @tparam T    type of the field
 */
template <typename T>
class FieldBase {
    public:
        /// the field tag itself
        using self_type = FieldBase;
        /// tell SOAContainer/View that we wrap a type
        using wrap_tag = struct {};
        static_assert(!SOATypelist::is_wrapped<T>::value,
                "It is an error to nest FieldBase<BASE, FieldBase<T> >!");
        /// type we wrap
        using type = T;
        /// support rebinding to the underlying base (tuple/...)
        template <typename BASE, typename PARENT>
        class rebind : public PARENT {
            protected:
                /// read-write access to field
                type& _get()
                { return static_cast<BASE&>(*this).template get<self_type>(); }
                /// read-only access to field
                type const& _get() const
                { return static_cast<const BASE&>(*this).template get<self_type>(); }
        };
    protected:
        /// stub: read-write access to field (never called)
        type& _get();
        /// stub: read-only access to field (never called)
        type const& _get() const;
};

/** @brief base class of all skins
 *
 * @tparam BASE         underlying tuple/whatever
 * @tparam FIELDS       fields to which grant access
 */
template <typename BASE, typename... FIELDS>
struct SkinBase : FIELDS::template rebind<BASE, FIELDS>..., BASE
                  // order above important for empty base class optimisation
{
    // make sure that no user puts data in a field...
    static_assert(SOAUtils::ALL<std::is_empty,
            typename FIELDS::template rebind<BASE, FIELDS>...>::value,
            "Fields may not contain data or virtual methods!");
    /// inform the framework that we're a skin
    using skin_tag = struct {};
    /// we're the base class underlying classes that inherit from us
    using base_type = SkinBase;
    /// typelist listing all fields
    using fields_typelist = SOATypelist::typelist<FIELDS...>;
    /// forward to *underlying constructors operators
    using BASE::BASE;
    /// forward to *underlying assignment operators
    using BASE::operator=;
};

/// little helper to detect the base type of a skin (saves typing)
template <typename T>
using detect_skin_base = typename T::base_type;

/* user code */
struct f_x : FieldBase<float> // this may be packaged up as a macro...
{
    float& x() { return this->_get(); }
    float const& x() const { return this->_get(); }
};
struct f_y : FieldBase<float>
{
    float& y() { return this->_get(); }
    float const& y() const { return this->_get(); }
};
struct f_n : FieldBase<int>
{
    int& n() { return this->_get(); }
    int const& n() const { return this->_get(); }
};

template <typename BASE>
struct Skin : SkinBase<PrintableNullSkin<BASE>, f_x, f_y, f_n>
{
    using B = detect_skin_base<Skin>;
    using B::B;
    using B::operator=;
};

/* small test driver... */
int main()
{
    SOAContainer<std::vector, NullSkin, float, float, int> s;
    std::tuple<float, float, int> t(3.14, 2.79, 42);
    std::cout << "naive proxy has size " <<
        sizeof(s.begin()) << std::endl;
    SOAContainer<std::vector, Skin> c;
    std::cout << "proxy has size " <<
        sizeof(c.begin()) << std::endl;
    c.push_back(t);
    std::cout << c[0] << std::endl;
    struct Foo { int i; };
    SkinBase<Foo, f_x, f_y, f_n> sb;
    std::cout << "SkinBase<Foo, f_x, f_y, f_n> has size " <<
        sizeof(sb) << std::endl;
    return 0;
}

// vim: sw=4:tw=78:ft=cpp:et
