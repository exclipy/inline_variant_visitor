#include <boost/variant.hpp>
#include "inline_variant.hpp"
#include <iostream>

int main()
{
    boost::variant< int, std::string > u("hello world");
    std::cout << u; // output: hello world

    int result = match(u,
        [](int i) -> int { return i; },
        [](const std::string & str) -> int { return str.length(); });
    std::cout << result; // output: 11 (i.e., length of "hello world")
}
