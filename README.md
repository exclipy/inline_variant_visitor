While Boost Variant is a wonderful library that fills a genuine hole in the C++ language, it is not
as nice to use as it could be.  The "correct" way to unwrap a variant is to apply a static visitor,
but this method carries a high syntactic burden because it needs the user to define a class just so
they can "switch" on the type of the object stored in the variant.

For example, here is the "motivating example" from the Boost Variant documentation:

    #include <boost/variant.hpp>
    #include <iostream>

    class my_visitor : public boost::static_visitor<int>
    {
    public:
        int operator()(int i) const
        {
            return i;
        }
        
        int operator()(const std::string & str) const
        {
            return str.length();
        }
    };

    int main()
    {
        boost::variant< int, std::string > u("hello world");
        std::cout << u; // output: hello world

        int result = boost::apply_visitor( my_visitor(), u );
        std::cout << result; // output: 11 (i.e., length of "hello world")
    }

Besides the obvious weight of the class definition, notice that the logic you want to run is now
removed from the original context in which you wanted to run it.

To make it better, we turn to C++11 lambdas.  Here is the code rewritten to use the library
presented here:

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

The magic part is the call to the `match` function, which accepts the variant followed by the
handler functions.  The functions can be specified in any order.  A compiler error will be thrown if
the functions do not match the types of the variant, if the return types are not all the same, or if
a non-unary function is supplied.

This is a C++11 only library because it's quite pointless without lambda functions.  I've tested it
with gcc 4.7.  It doesn't work on gcc 4.6, which can't handle the variadic templates.
