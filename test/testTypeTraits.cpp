/**
 * @brief Construct a new TEST object
 *
 */

#include <catch2/catch_all.hpp>
#include <variant>
#include <base/type_traits.h>

using namespace mpl;

template <typename, typename Type = void>
struct multi_argument_operation
{
    using type = Type;
};

TEST_CASE("Choice", "[type_traits]")
{

    STATIC_REQUIRE(std::is_base_of_v<choice_t<0>, choice_t<1>>);
    STATIC_REQUIRE(!std::is_base_of_v<choice_t<1>, choice_t<0>>);
}

TEST_CASE("SizeOf", "[type_traits]")
{
    STATIC_REQUIRE(size_of_v<void> == 0u);
    STATIC_REQUIRE(size_of_v<char> == sizeof(char));
    STATIC_REQUIRE(size_of_v<int[]> == 0u);
    STATIC_REQUIRE(size_of_v<int[3]> == sizeof(int[3]));
}

TEST_CASE("TypeList", "[type_traits]")
{

    using type = type_list<int, char>;
    using other = type_list<double>;

    using list_0 = type_list<char, int, float>;

    STATIC_REQUIRE(type::size == 2u);
    STATIC_REQUIRE(other::size == 1u);
    // type_list_size
    // static_assert(type_list_size(list_0{}) == 3, "three element list");
    STATIC_REQUIRE(list_0::size == 3u);

    STATIC_REQUIRE(std::is_same_v<decltype(type{} + other{}), type_list<int, char, double>>);
    STATIC_REQUIRE(std::is_same_v<type_list_cat_t<type, other, type, other>, type_list<int, char, double, int, char, double>>);
    STATIC_REQUIRE(std::is_same_v<type_list_cat_t<type, other>, type_list<int, char, double>>);
    STATIC_REQUIRE(std::is_same_v<type_list_cat_t<type, type>, type_list<int, char, int, char>>);
    STATIC_REQUIRE(std::is_same_v<type_list_unique_t<type_list_cat_t<type, type>>, type_list<int, char>>);

    STATIC_REQUIRE(type_list_contains_v<type, int>);
    STATIC_REQUIRE(type_list_contains_v<type, char>);
    STATIC_REQUIRE(!type_list_contains_v<type, double>);

    // type_list_has
    STATIC_REQUIRE(type_list_contains_v<list_0, char>);           // contains char
    STATIC_REQUIRE(type_list_contains_v<list_0, int>);            // contains int
    STATIC_REQUIRE(type_list_contains_v<list_0, float>);          // contains float
    STATIC_REQUIRE(!type_list_contains_v<list_0, double>);        // no double
    STATIC_REQUIRE(!type_list_contains_v<list_0, unsigned char>); // no unsiogned char

    STATIC_REQUIRE(std::is_same_v<type_list_element_t<0u, type>, int>);
    STATIC_REQUIRE(std::is_same_v<type_list_element_t<1u, type>, char>);
    STATIC_REQUIRE(std::is_same_v<type_list_element_t<0u, other>, double>);

    // type_list_type
    STATIC_REQUIRE(std::is_same_v<type_list_element_t<0u, list_0>, char>); // "char at index 0");
    STATIC_REQUIRE(std::is_same_v<type_list_element_t<1u, list_0>, int>); // "int at index 1");
    STATIC_REQUIRE(std::is_same_v<type_list_element_t<2u, list_0>, float>); //"float at index 2");
    // TODO: STATIC_REQUIRE(std::is_same_v<type_list_type<3, list_0>::type, double> == ???, "??");

    STATIC_REQUIRE(type_list_index_v<int, type> == 0u);
    STATIC_REQUIRE(type_list_index_v<char, type> == 1u);
    STATIC_REQUIRE(type_list_index_v<double, other> == 0u);

    // type_list_index
    STATIC_REQUIRE(type_list_index_v<char, list_0> == 0);          // "char at index 0"
    STATIC_REQUIRE(type_list_index_v<int, list_0> == 1);           // "char at index 1"
    STATIC_REQUIRE(type_list_index_v<float, list_0> == 2);         // "char at index 2"
    STATIC_REQUIRE(type_list_index_v<double, list_0> == 3);        // "not present at index == sizeof"
    STATIC_REQUIRE(type_list_index_v<unsigned char, list_0> == 3); // "not present at index == sizeof");

    STATIC_REQUIRE(std::is_same_v<type_list_diff_t<type_list<int, char, double>, type_list<float, bool>>, type_list<int, char, double>>);
    STATIC_REQUIRE(std::is_same_v<type_list_diff_t<type_list<int, char, double>, type_list<int, char, double>>, type_list<>>);
    STATIC_REQUIRE(std::is_same_v<type_list_diff_t<type_list<int, char, double>, type_list<int, char>>, type_list<double>>);
    STATIC_REQUIRE(std::is_same_v<type_list_diff_t<type_list<int, char, double>, type_list<char, double>>, type_list<int>>);
    STATIC_REQUIRE(std::is_same_v<type_list_diff_t<type_list<int, char, double>, type_list<char>>, type_list<int, double>>);

    STATIC_REQUIRE(std::is_same_v<type_list_transform_t<type_list<int, char>, type_identity>, type_list<int, char>>);
    STATIC_REQUIRE(std::is_same_v<type_list_transform_t<type_list<int, char>, std::add_const>, type_list<const int, const char>>);
    STATIC_REQUIRE(std::is_same_v<type_list_transform_t<type_list<int, char>, multi_argument_operation>, type_list<void, void>>);

    // type_list_first
    STATIC_REQUIRE(std::is_same_v<typename type_list_first<list_0>::type, char>); // "first type should be char")

    // type_list_push_front
    using states_variant_list = typename type_list_push_front<list_0, std::monostate>::result;
    STATIC_REQUIRE(std::is_same_v<type_list_element_t<0u, states_variant_list>, std::monostate>); // "char at index 0");

    // type_list_rename
    using states_variant = typename type_list_rename<states_variant_list, std::variant>::result;
    
    STATIC_REQUIRE(std::is_same_v<states_variant, std::variant<std::monostate, char, int, float>>); // "states_variant is std::variant");

}

TEST_CASE("ValueList", "[type_traits]"){
    using value = value_list<0, 2>;
    using other = value_list<1>;

    STATIC_REQUIRE(value::size == 2u);
    STATIC_REQUIRE(other::size == 1u);

    STATIC_REQUIRE(std::is_same_v<decltype(value{} + other{}), value_list<0, 2, 1>>);
    STATIC_REQUIRE(std::is_same_v<value_list_cat_t<value, other, value, other>, value_list<0, 2, 1, 0, 2, 1>>);
    STATIC_REQUIRE(std::is_same_v<value_list_cat_t<value, other>, value_list<0, 2, 1>>);
    STATIC_REQUIRE(std::is_same_v<value_list_cat_t<value, value>, value_list<0, 2, 0, 2>>);

    STATIC_REQUIRE(value_list_element_v<0u, value> == 0);
    STATIC_REQUIRE(value_list_element_v<1u, value> == 2);
    STATIC_REQUIRE(value_list_element_v<0u, other> == 1);
}