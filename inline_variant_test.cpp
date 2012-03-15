#include <boost/variant.hpp>
#include "inline_variant.hpp"

typedef boost::variant<int, char> IntChar;

int main(int, char**) {
    IntChar v('a');
    // v = 3;
    int ret = boost::apply_visitor(make_visitor(
        [](int x) {
            std::cout << "int " << x << std::endl;
            return 1;
        },
        [](char x) {
            std::cout << "char " << x << std::endl;
            return 2;
        }), v);
    std::cout << ret << std::endl;

    return 0;
}
