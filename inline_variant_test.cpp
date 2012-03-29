#include <boost/variant.hpp>
#define BOOST_TEST_MODULE MyTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "inline_variant.hpp"

typedef boost::variant<int, char> IntChar;

int run_visitor(IntChar const& variant)
{
    int ret = match(variant,
        [](int x) -> int { return x; },
        [](char x) -> int { return static_cast<int>(x) + 3; });
    return ret;
}

BOOST_AUTO_TEST_CASE(visit_primitives) {
    IntChar v(123);
    BOOST_CHECK_EQUAL(run_visitor(v), 123);
    v = 'c';
    BOOST_CHECK_EQUAL(run_visitor(v), static_cast<int>('c')+3);
}

BOOST_AUTO_TEST_CASE(void_return) {
    IntChar v(123);
    int result = 0;
    match(v,
        [&result](int x) -> void { result = 1; },
        [&result](char x) -> void { result = 2; });
    BOOST_CHECK_EQUAL(result, 1);
}

struct copy_counter
{
    copy_counter(int * counter) : copy_count_(counter) {}
    copy_counter(copy_counter const& other) : copy_count_(other.copy_count_)
    {
        ++(*copy_count_);
    }

    int * copy_count_;
};

struct int_function : copy_counter
{
    int_function(int * counter) : copy_counter(counter) {}
    void operator()(int x) const {}
};

struct char_function : copy_counter
{
    char_function(int * counter) : copy_counter(counter) {}
    void operator()(char x) const {}
};

BOOST_AUTO_TEST_CASE(copyable_functions) {
    IntChar v(123);
    int counter1 = 0;
    int counter2 = 0;
    match(v, int_function(&counter1), char_function(&counter2));
    std::cout << counter1 << std::endl;
    std::cout << counter2 << std::endl;
}
