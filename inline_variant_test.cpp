#include <boost/variant.hpp>
#include "inline_variant.hpp"

typedef boost::variant<int, char> IntChar;

struct Func1
{
	int operator()(int x) const {
        std::cout << "int " << x << std::endl;
        return 1;
	}
};

struct Func2
{
	int operator()(char x) const {
        std::cout << "char " << x << std::endl;
        return 2;
	}
};

int main() {
    IntChar v('a');
    //v = 3;
    int ret = boost::apply_visitor(make_visitor<int>(Func1(), Func2()), v);
    std::cout << ret << std::endl;
}
