#include <tuple>
#include <vector>
#include <iostream>
#include "include/SOATypelist.h"
#include "include/SOAContainer.h"
#include "include/PrintableNullSkin.h"

template <typename BASE, typename T>
struct FieldBase {
    using wrap_tag = struct {};
    static_assert(!SOATypelist::is_wrapped<T>::value,
	"It is an error to nest FieldBase<BASE, FieldBase<T> >!");
    using type = T;
    type& operator()()
    { return static_cast<BASE&>(*this).template get<FieldBase<BASE, type> >(); }
    type const& operator()() const
    { return static_cast<const BASE&>(*this).template get<FieldBase<BASE, type> >(); }
};


template <typename BASE, template <typename> class... FIELDS>
struct SkinBase : BASE //, FIELDS<BASE>...
{
    using skin_tag = struct {};
    using fields_typelist = SOATypelist::typelist<FIELDS<BASE>... >;
    using BASE::BASE;
    using BASE::operator=;
};

template <typename BASE>
struct f_x : FieldBase<BASE, float>
{
    float& x() { return this->operator(); }
    float const& x() const { return this->operator(); }
};
template <typename BASE>
struct f_y : FieldBase<BASE, float>
{
    float& y() { return this->operator(); }
    float const& y() const { return this->operator(); }
};
template <typename BASE>
struct f_n : FieldBase<BASE, int>
{
    int& n() { return this->operator(); }
    int const& n() const { return this->operator(); }
};

template <typename T, template <typename> class... FIELDS>
SkinBase<T, FIELDS...> _detect_skin_base(const SkinBase<T, FIELDS...>&);
template <typename T>
using detect_skin_base = decltype(_detect_skin_base(std::declval<const T&>()));

template <typename BASE>
struct Skin : SkinBase<PrintableNullSkin<BASE>, f_x, f_y, f_n>
{
    using B = detect_skin_base<Skin>;
    using B::B;
    using B::operator=;
};

int main()
{
    std::tuple<float, float, int> t(3.14, 2.79, 42);
    SOAContainer<std::vector, Skin> c;
    c.push_back(t);
    std::cout << c[0] << std::endl;
    struct Foo {};
    SkinBase<Foo, f_x, f_y, f_n> sb;
    std::cout << "SkinBase<Foo, f_x, f_y, f_n> has size " <<
	sizeof(sb) << std::endl;
    return 0;
}


