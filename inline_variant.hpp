#include <typeinfo>
#include <iostream>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/equal_to.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/static_assert.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/move/move.hpp>
#include <boost/fusion/include/map.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/fusion/include/has_key.hpp>
#include <boost/fusion/include/at_key.hpp>
#include <boost/fusion/algorithm/transformation/transform.hpp>
#include <utility>

#include "function_signature.hpp"

struct function_arg_extractor
{
	template <typename FunctionType>
	struct apply
	{
	private:
		typedef typename boost::remove_const< typename boost::remove_reference<FunctionType>::type >::type bare_type;
		typedef typename signature_of<bare_type>::type normalized_function_type;
		typedef typename boost::function_types::function_arity<normalized_function_type>::type arity;
		typedef typename boost::function_types::parameter_types<normalized_function_type>::type parameter_types;
		typedef typename boost::function_types::result_type<normalized_function_type>::type result_type;

		BOOST_STATIC_ASSERT((arity::value == 1));

		typedef typename boost::mpl::front<parameter_types>::type parameter_type;
	public:
		typedef typename boost::remove_const< typename boost::remove_reference<parameter_type>::type >::type type;
	};
};

template <typename FunctionTypes>
struct get_functions_args : boost::mpl::transform<FunctionTypes, function_arg_extractor>
{
};

template <typename FunctionType>
struct get_function_return
{
private:
	typedef typename boost::remove_const< typename boost::remove_reference<FunctionType>::type >::type bare_type;
	typedef typename signature_of<bare_type>::type normalized_function_type;
public:
	typedef typename boost::function_types::result_type<normalized_function_type>::type type;
};

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

template <typename ReturnType, typename... FunctionTypes >
struct generic_visitor : boost::static_visitor<ReturnType>
{
private:
	typedef boost::mpl::vector<FunctionTypes...> function_types;
	typedef typename get_functions_args<function_types>::type variant_types;
	typedef typename boost::mpl::transform<
		variant_types,
		function_types,
		boost::fusion::result_of::make_pair<boost::mpl::_1, boost::mpl::_2> >::type pair_list;
	typedef typename boost::fusion::result_of::as_map<pair_list>::type function_map;

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
		BOOST_STATIC_ASSERT((boost::mpl::contains<variant_types, bare_type>::type::value ));
		BOOST_STATIC_ASSERT((boost::fusion::result_of::has_key<function_map, T>::value));
		return boost::fusion::at_key<T>(fmap)(object);
	}
};

template <typename... FunctionTypes>
struct get_generic_visitor_type
{
private:
	typedef boost::mpl::vector<FunctionTypes...> function_types;
	BOOST_STATIC_ASSERT((boost::mpl::size<function_types>::value > 0));
	typedef typename get_function_return<typename boost::mpl::front<function_types>::type>::type result_type;
public:
	typedef generic_visitor<result_type, FunctionTypes...> type;
};

template <typename... FunctionTypes>
auto make_visitor(FunctionTypes... functions) -> typename get_generic_visitor_type<FunctionTypes...>::type {
	return typename get_generic_visitor_type<FunctionTypes...>::type(functions...);
}