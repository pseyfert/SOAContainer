#ifndef SOAITERATOR_H
#define SOAITERATOR_H

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
        using parent_type = typename POSITION::parent_type;

        using POSITION::stor;
        using POSITION::idx;

    public:
        using iterator_category = std::random_access_iterator_tag;
        using reference = typename std::conditional<ISCONST,
              typename parent_type::const_reference,
              typename parent_type::reference>::type;
        using size_type = typename parent_type::size_type;
        using difference_type = typename parent_type::difference_type;
        using value_type = typename parent_type::value_type;
        using pointer = Iterator<POSITION, ISCONST>;
        using const_pointer = Iterator<POSITION, true>;

        // "magic" constructor to be used by SOA container classes only
        //
        // it's "magic" in the sense that it's kind of hard to get at the
        // POSITION type accidentally, since none of the SOA container classes
        // expose this type in their public interface, so there is little
        // danger of inadvertently creating an Iterator compatible with SOA
        // containers that is in a funky state because the user accidentally
        // passed funny values to the constructor.
        template <typename POS>
        constexpr explicit Iterator(POS&& pos,
            typename std::enable_if<std::is_base_of<POSITION,
            POS>::value>::type* = nullptr) noexcept :
            POSITION(std::forward<POS>(pos))
        {}

        // test for "nullness"
        constexpr operator bool() const noexcept
        { return stor() && (idx() < std::get<0>(*stor()).size()); }

        // deference/index/operator->
        constexpr reference operator*() const noexcept
        { return reference{ POSITION(*this) }; }
        constexpr const reference* const operator->() const noexcept
        { return reinterpret_cast<const reference* const>(this); }
        reference* operator->() noexcept
        { return reinterpret_cast<reference*>(this); }
        constexpr reference operator[](size_type ofs) const noexcept
        { return *((*this) + ofs); }

        // pointers convert to const_pointers, but not vice-versa
        constexpr operator const_pointer() const noexcept
        { return const_pointer{ POSITION(*this) }; }

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

        template <typename PTR, typename INC>
        constexpr friend typename std::enable_if<
            std::is_same<typename std::remove_reference<typename
            std::remove_cv<PTR>::type>::type, pointer>::value &&
            std::is_convertible<INC, difference_type>::value,
        pointer>::type operator+(PTR&& p, INC inc) noexcept
        {
            return pointer(std::forward<PTR>(p)) +=
                static_cast<difference_type>(inc);
        }

        template <typename PTR, typename INC>
        constexpr friend typename std::enable_if<
            std::is_same<typename std::remove_reference<typename
            std::remove_cv<PTR>::type>::type, pointer>::value &&
            std::is_convertible<INC, difference_type>::value,
        pointer>::type operator-(PTR&& p, INC inc) noexcept
        {
            return pointer(std::forward<PTR>(p)) -=
                static_cast<difference_type>(inc);
        }

        template <typename PTR, typename INC>
        constexpr friend typename std::enable_if<
            std::is_same<typename std::remove_reference<typename
            std::remove_cv<PTR>::type>::type, pointer>::value &&
            std::is_convertible<INC, difference_type>::value,
        pointer>::type operator+(INC inc, PTR&& p) noexcept
        {
            return pointer(std::forward<PTR>(p)) +=
                static_cast<difference_type>(inc);
        }

        // distance between iterators
        template <typename PTR>
        constexpr typename std::enable_if<std::is_same<PTR, pointer>::value &&
            std::is_same<PTR, const_pointer>::value,
        difference_type>::type operator-(const PTR& p) const noexcept
        { return idx() - p.idx(); }

        // pointer equality
        friend constexpr bool operator==(
                const pointer& p, const pointer& q) noexcept
        { return p.stor() == q.stor() && p.idx() == q.idx(); }
        friend constexpr bool operator!=(
                const pointer& p, const pointer& q) noexcept
        { return !(p == q); }

        // pointer comparisons - ordering works only if they point into same
        // container instance
        friend constexpr bool operator<(
                const pointer& p, const pointer& q) noexcept
        {
            return (p.stor() < q.stor()) ||
                (!(q.stor() < p.stor()) && p.idx() < q.idx());
        }
        friend constexpr bool operator>(
                const pointer& p, const pointer& q) noexcept
        { return q < p; }
        friend constexpr bool operator<=(
                const pointer& p, const pointer& q) noexcept
        { return !(q < p); }
        friend constexpr bool operator>=(
                const pointer& p, const pointer& q) noexcept
        { return !(p < q); }
    };
} // namespace SOA

#endif // SOAITERATOR_H

// vim: sw=4:tw=78:ft=cpp:et
