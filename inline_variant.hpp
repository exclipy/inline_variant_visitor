#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/fusion/include/at_key.hpp>
#include <boost/fusion/include/has_key.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/fusion/include/map.hpp>
#include <boost/fusion/algorithm/transformation/transform.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/equal_to.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/set.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/remove_reference.hpp>

#include "function_signature.hpp"

namespace detail {

// A metafunction class for getting the argument type from a unary function or functor type
struct function_arg_extractor
{
    // FunctionType is either a function type like void(int const&), or a functor - eg. a class with void operator(int)
    // Sets type to the argument type with the constness and referenceness stripped (eg. int)
    template <typename FunctionType>
    struct apply
    {
    private:
        typedef typename boost::remove_const< typename boost::remove_reference<FunctionType>::type >::type bare_type;
        typedef typename signature_of<bare_type>::type normalized_function_type;
        typedef typename boost::function_types::function_arity<normalized_function_type>::type arity;
        typedef typename boost::function_types::parameter_types<normalized_function_type>::type parameter_types;
        typedef typename boost::function_types::result_type<normalized_function_type>::type result_type;

        BOOST_STATIC_ASSERT_MSG((arity::value == 1), "make_visitor called with a non-unary function");

        typedef typename boost::mpl::front<parameter_types>::type parameter_type;
    public:
        typedef typename boost::remove_const< typename boost::remove_reference<parameter_type>::type >::type type;
    };
};

// A private helper fusion metafunction to construct a fusion map of functions
struct pair_maker
{
    template<typename Sig>
    struct result;

    template <typename FunctionType>
    struct result<pair_maker(FunctionType)>
    {
        typedef typename function_arg_extractor::apply<FunctionType>::type arg_type;
        typedef boost::fusion::pair<arg_type, FunctionType> type;
    };

    template <typename FunctionType>
    typename result<pair_maker(FunctionType)>::type operator()(FunctionType a) const
    {
        return boost::fusion::make_pair< typename result<pair_maker(FunctionType)>::arg_type >(a);
    }
};

// A functor template suitable for passing into apply_visitor.  The constructor accepts the list of handler functions,
// which are then exposed through a set of operator()s
template <typename ReturnType, typename... FunctionTypes >
struct generic_visitor : boost::static_visitor<ReturnType>
{
private:
    // Compute the function_map type
    typedef boost::mpl::vector<FunctionTypes...> function_types;
    typedef typename boost::mpl::transform<function_types, function_arg_extractor>::type variant_types;
    // Check that the argument types are unique
    typedef typename boost::mpl::fold<
        variant_types,
        boost::mpl::set0<>,
        boost::mpl::insert<boost::mpl::_1,boost::mpl::_2>
    >::type variant_types_set;
    BOOST_STATIC_ASSERT_MSG((boost::mpl::size<variant_types_set>::type::value == boost::mpl::size<variant_types>::value),
            "make_visitor called with non-unique argument types for handler functions");
    typedef typename boost::mpl::transform<
        variant_types,
        function_types,
        boost::fusion::result_of::make_pair<boost::mpl::_1, boost::mpl::_2>
    >::type pair_list;
    typedef typename boost::fusion::result_of::as_map<pair_list>::type function_map;

    // Maps from argument type to the runtime function object that can deal with it
    function_map fmap;

public:
    generic_visitor(FunctionTypes... functions)
    :
        fmap(boost::fusion::as_map(boost::fusion::transform(boost::fusion::make_vector(functions...), pair_maker())))
    {
    }

    template <typename T>
    ReturnType operator()(T object) const {
        typedef typename boost::remove_const< typename boost::remove_reference<T>::type >::type bare_type;
        BOOST_STATIC_ASSERT_MSG((boost::fusion::result_of::has_key<function_map, T>::value),
                "make_visitor called without specifying handlers for all required types");
        return boost::fusion::at_key<T>(fmap)(object);
    }
};

// A metafunction class for getting the return type of a function
struct function_return_extractor
{
    template <typename FunctionType>
    struct apply : boost::function_types::result_type<typename signature_of<FunctionType>::type>
    {
    };
};

// Check that all functions return result_type
// A metafunction class that asserts the two arguments are the same and returns the first one
struct check_same
{
    template <typename Type1, typename Type2>
    struct apply
    {
    private:
        BOOST_STATIC_ASSERT_MSG((boost::is_same<Type1, Type2>::type::value),
                "make_visitor called with functions of differing return types");
    public:
        typedef Type1 type;
    };
};

// A metafunction for getting the required generic_visitor type for the set of FunctionTypes
template <typename... FunctionTypes>
struct get_generic_visitor_type
{
private:
    typedef boost::mpl::vector<FunctionTypes...> function_types;
    BOOST_STATIC_ASSERT_MSG((boost::mpl::size<function_types>::value > 0),
            "make_visitor called with no functions");
    typedef typename boost::mpl::transform<function_types, function_return_extractor>::type return_types;

    // Set result_type to the return type of the first function
    typedef typename boost::mpl::front<return_types>::type result_type;

    typedef typename boost::mpl::fold<return_types, result_type, check_same>::type dummy;
public:
    typedef generic_visitor<result_type, FunctionTypes...> type;
};

}

template <typename... FunctionTypes>
auto make_visitor(FunctionTypes... functions) -> typename detail::get_generic_visitor_type<FunctionTypes...>::type {
    return typename detail::get_generic_visitor_type<FunctionTypes...>::type(functions...);
}
