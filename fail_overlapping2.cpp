#include <boost/variant.hpp>

#include "inline_variant.hpp"

typedef boost::variant<int, char> IntChar;

int main(int, char**) {
    IntChar v(123);
    int ret = boost::apply_visitor(make_visitor(
        [](int x) { return 0; },
        [](int const& x) { return 1; },
        [](char x) { return 2; }),
        v);
    return 0;
}
