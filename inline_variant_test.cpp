#include <boost/variant.hpp>
#define BOOST_TEST_MODULE MyTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "inline_variant.hpp"

typedef boost::variant<int, char> IntChar;

int run_visitor(const IntChar& variant)
{
    int ret = match(variant,
        [](int x) { return static_cast<int>(x); },
        [](char x) { return static_cast<int>(x) + 3; });
    return ret;
}

BOOST_AUTO_TEST_CASE(visit_primitives) {
    IntChar v(123);
    BOOST_CHECK_EQUAL(run_visitor(v), 123);
    v = 'c';
    BOOST_CHECK_EQUAL(run_visitor(v), static_cast<int>('c')+3);
}
