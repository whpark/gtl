//////////////////////////////////////////////////////////////////////
//
// reflection.h: reflection for streaming, dynamic creating (json/archive...)
//
// PWH.
// 2020.01.03.
// 2021.01.04. member_wise.h -> reflection.h
// 2021.01.17. json proxy
//
//////////////////////////////////////////////////////////////////////

module;

#include <tuple>

#include "gtl/_config.h"
#include "gtl/_macro.h"

export module gtl:reflection;

import :concepts;
import :misc;
import :string;
import :json_proxy;

export namespace gtl {

	// REFL : Reflection. (Member Wise..., variables only. not for function ptr.)

	//================================================================================================================================
	// reflection CRTP version.
	//

	////================================================================================================================================
	//// reflection CRTP version.
	////

	///// @brief CRTP reflection class. (not good. better to use MACROS)
	///// @tparam 
	//template < typename THIS_CLASS, typename tjson = gtl::njson<nlohmann::json> >
	//class IReflection {
	//public:
	//	using reflection_base_t = THIS_CLASS;
	//	using this_t = THIS_CLASS;

	//	GTL__REFLECTION_VIRTUAL_BASE(tjson)

	////public:
	////	auto operator <=>(IReflection const&) const = default;
	//};



	///// @brief CRTP reflection class (derived). (not good. better to use MACROS)
	///// @tparam 
	//template < typename THIS_CLASS, typename BASE_CLASS >
	//class IReflectionDerived : public BASE_CLASS {
	//public:
	//	using this_t = THIS_CLASS;
	//	using base_t = BASE_CLASS;

	//	GTL__REFLECTION_VIRTUAL_DERIVED()

	//public:
	//	auto operator <=>(IReflectionDerived const&) const = default;
	//};


}	// namespace gtl
