#ifndef DUMP_X3_SUPPORT_HPP
#define DUMP_X3_SUPPORT_HPP

#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include <type_traits>

namespace dump {
    // As advised by Agustín Bergé
    // https://github.com/K-ballo

    /*
     * Usage:
     * <code>
     * template <typename T,
     *           typename Enable = typename std::enable_if<is_variant<T>::value>::type>
     * void operator()(T const& ast) {
     *     boost::apply_visitor(*this, ast);
     * };
     * </code>
     */

    template <typename ...Ts> struct pack {};

    void _is_variant(...);

    template <typename ...Ts>
    pack<Ts...> _is_variant(boost::spirit::x3::variant<Ts...> const*);

    template <typename T, typename R = decltype(_is_variant(static_cast<T*>(nullptr)))>
    struct is_variant : std::false_type {};

    template <typename T, typename ...Ts>
    struct is_variant<T, pack<Ts...>> : std::true_type {};

}

#endif //DUMP_X3_SUPPORT_HPP
