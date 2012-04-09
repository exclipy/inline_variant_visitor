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

struct int_function
{
    int_function(int * counter) : copy_count_(counter) {}
    int_function(int_function && other) {}
    int_function(int_function const& other) : copy_count_(other.copy_count_)
    {
        ++(*copy_count_);
    }

    int * copy_count_;
    void operator()(int x) const {}
};

BOOST_AUTO_TEST_CASE(copyable_functions) {
    IntChar v(123);
    int counter = 0;
    match(v, int_function(&counter), [](char){});
    BOOST_CHECK_EQUAL(counter, 0);
}
