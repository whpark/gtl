#pragma once

#include "gtl/dxf/group.h"

using namespace std::literals;
using namespace gtl::literals;

namespace gtl::dxf {

	//template < typename tag, typename TUserDefined, auto Count_or_OffsetToCountMember, eGROUP_CODE ... eGroupCodes >

	template < eGROUP_CODE code, typename TUserDefined = void >
	using gcv = TGroupVariable<detail::tagGVSimple, TUserDefined, code>;

	template < eGROUP_CODE code, typename TUserDefined = void >
	using gcv_t = gcv<code, TUserDefined>::value_t;

	template < typename TUserDefined, eGROUP_CODE ... codes >
	using mgcv = TGroupVariable<detail::tagGVStruct, TUserDefined, codes...>;

	template < eGROUP_CODE ... codes >
	using sgcv = TGroupVariable<detail::tagGVString, string_t, codes...>;

	template < eGROUP_CODE code, auto Count_or_OffsetToCountMember >
	using lgcv = TListVariable<TGroupVariable<detail::tagGVSimple, void, code>, Count_or_OffsetToCountMember>;

	using gcv_extrusion = mgcv<point_t, 210, 220, 230>;

};
