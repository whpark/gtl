#pragma once

//////////////////////////////////////////////////////////////////////
//
// group.h : DXF group code / group value
//
// PWH
// 2026-09-02 ported from biscuit.dxf (module :group) to gtl.dxf (header)
//
//////////////////////////////////////////////////////////////////////

#include <array>
#include <compare>
#include <cstdint>
#include <deque>
#include <iterator>
#include <limits>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include "fmt/format.h"
#include "fmt/ranges.h"
#include "fmt/std.h"

#include "gtl/_default.h"
#include "gtl/concepts.h"
#include "gtl/misc.h"
#include "gtl/reflection_struct.h"
#include "gtl/coord.h"
#include "gtl/unit.h"

#include "gtl/dxf/_lib_gtl_dxf.h"
#include "gtl/dxf/macro.h"

namespace gtl::dxf {
#pragma pack(push, 8)

	using namespace std::literals;
	using namespace gtl::literals;

	//=============================================================================================================================
	using binary_t = std::vector<std::uint8_t>;

	// NOTE : gtl::shape::string_t is std::wstring, but DXF group values are kept as read from the file.
	//        they are converted to gtl::shape::string_t only when a drawing is built. (see gtl/dxf/dxf.cpp)
	using string_t = std::string;
	using string_view_t = std::string_view;
	using group_value_t = std::variant<bool, std::int16_t, std::int32_t, std::int64_t, double, string_t, binary_t>;
	enum eGROUP_VALUE_TYPE : std::int8_t { none = -1, boolean = 0, i16, i32, i64, dbl, str, binary };

	using point_t = gtl::xPoint3d;

	template < typename T >
	consteval size_t GetGroupValueIndex() {
		// NOTE : gtl::rad_t / gtl::deg_t have an implicit (and non-constexpr) conversion to double,
		//        so they must be checked BEFORE std::is_constructible_v<group_value_t, T>.
		if constexpr (std::is_same_v<T, gtl::rad_t> or std::is_same_v<T, gtl::deg_t>) {
			return GetGroupValueIndex<double>();
		} else if constexpr (std::is_same_v<T, gtl::color_bgra_t>) {
			return GetGroupValueIndex<int32_t>();
		} else if constexpr (std::is_constructible_v<group_value_t, T>) {
			return group_value_t{T{}}.index();
		} else if constexpr (std::is_enum_v<T>) {
			return GetGroupValueIndex<std::underlying_type_t<T>>();
		} else {
			static_assert(false);
		}
	}
	template < typename T >
		requires (::gtl::concepts::is_one_of<T, bool, std::int16_t, std::int32_t, std::int64_t, double, string_t, binary_t>)
	std::partial_ordering operator <=> (group_value_t const& l, T const& r) noexcept {
		using U = std::remove_cvref_t<T>;
		using R = std::partial_ordering;//std::compare_three_way_result_t<U, U>;
		constexpr auto index = GetGroupValueIndex<U>();
		if (R r = l.index() <=> index; r != 0)
			return r;
		R res{R::unordered};
		std::visit([&](auto const& v) { if constexpr (std::is_same_v<std::remove_cvref_t<decltype(v)>, U>) res = (v <=> r); } , l);
		return res;
	}
	template < typename T >
		requires (::gtl::concepts::is_one_of<T, bool, std::int16_t, std::int32_t, std::int64_t, double, string_t, binary_t>)
	std::partial_ordering operator <=> (T const& r, group_value_t const& l) noexcept { return 0 <=> (l <=> r); }

	template < typename T >
		requires (::gtl::concepts::is_one_of<T, bool, std::int16_t, std::int32_t, std::int64_t, double, string_t, binary_t>)
	bool operator == (group_value_t const& l, T const& r) noexcept { return (l <=> r) == 0; }
	template < typename T >
		requires (::gtl::concepts::is_one_of<T, bool, std::int16_t, std::int32_t, std::int64_t, double, string_t, binary_t>)
	bool operator == (T const& r, group_value_t const& l) noexcept { return (l <=> r) == 0; }
	template < typename T >
		requires (::gtl::concepts::is_one_of<T, bool, std::int16_t, std::int32_t, std::int64_t, double, string_t, binary_t>)
	bool operator != (group_value_t const& l, T const& r) noexcept { return (l <=> r) != 0; }
	template < typename T >
		requires (::gtl::concepts::is_one_of<T, bool, std::int16_t, std::int32_t, std::int64_t, double, string_t, binary_t>)
	bool operator != (T const& r, group_value_t const& l) noexcept { return (l <=> r) != 0; }

	//=============================================================================================================================
	// group
	struct eGROUP_CODE {
		using enum_value_t = std::int16_t;
		enum : enum_value_t {
			entity = 0,
			table_entry = 2,
			variable = 9,
			subclass = 100,
			control = 102,

			extended = 1000,
			xdata = 1002,
			last = 1072,
		};
		enum_value_t eCode;

		constexpr eGROUP_CODE() = default;
		constexpr eGROUP_CODE(eGROUP_CODE const&) = default;
		constexpr eGROUP_CODE(eGROUP_CODE&&) = default;
		constexpr eGROUP_CODE& operator = (eGROUP_CODE const&) = default;
		constexpr eGROUP_CODE& operator = (eGROUP_CODE&&) = default;
		constexpr eGROUP_CODE(enum_value_t eValueType) : eCode(eValueType) {}

		//constexpr operator enum_value_t() const { return value; }
		eGROUP_CODE& operator = (enum_value_t eValueType) { eCode = eValueType; return *this; }
		constexpr auto operator <=> (eGROUP_CODE const&) const = default;
		constexpr auto operator <=> (enum_value_t const& eValueType) const { return eCode <=> eValueType; }
	};
	constexpr auto operator "" _g(unsigned long long v) { return eGROUP_CODE{ (std::int16_t)v }; }

	//-----------------------------------------------------------------------------------------------------------------------------
	struct alignas(32) sGroup : public eGROUP_CODE {	// alignas(32) - hoping for better cache performance (don't know if it works)
	public:
		using this_t = sGroup;
	public:

		//eGROUP_CODE eCode{};
		group_value_t value;

		constexpr sGroup() = default;
		constexpr sGroup(sGroup const&) = default;
		constexpr sGroup(sGroup&&) = default;
		constexpr sGroup& operator = (sGroup const&) = default;
		constexpr sGroup& operator = (sGroup&&) = default;

		constexpr sGroup(eGROUP_CODE code, group_value_t v) : eGROUP_CODE(code), value(std::move(v)) {}

		//constexpr bool operator == (sGroup const&) const = default;
		//constexpr bool operator != (sGroup const&) const = default;
		GTL__DXF_DEFINE_SPACESHIP_OPERATOR(this_t);

		//template < std::convertible_to<group_value_t> T >
		template < typename T >
		std::optional<T> GetValue() const {
			constexpr static size_t index = GetGroupValueIndex<T>();
			if (value.index() != index)
				return std::nullopt;
			return (T&)(std::get<index>(value));
		}
		std::optional<int> GetInt() const {
			std::optional<int> result;
			std::visit([&result]<typename T>(T const& v) {
				if constexpr (std::is_same_v<T, std::int64_t>) {
					//static_assert(false);
					if (v >= std::numeric_limits<int>::min() and v <= std::numeric_limits<int>::max()) {
						result = (int)v;
					}
				}
				else if constexpr (std::is_integral_v<T>)
					result = (int)v;
			}, value);
			return result;
		}

		template < typename T >
		bool GetValue(T& value) const {
			if constexpr ((std::is_enum_v<T> or std::is_integral_v<T>) and sizeof(T) > sizeof(bool) and sizeof(T) < sizeof(int64)) {
				if (auto v = GetInt()) {
					value = (T)*v;
					return true;
				}
			}
			else if (auto v = GetValue<T>()) {
				value = *v;
				return true;
			}
			return false;
		}

		constexpr static eGROUP_VALUE_TYPE GET_VALUE_TYPE_ENUM(eGROUP_CODE code) {
			//static_assert(std::is_constant_evaluated());
			using enum eGROUP_VALUE_TYPE;

			auto InRange = [](auto&& code, std::int16_t min, std::int16_t max) {
				return code >= min and code <= max;
			};

																//================================================================================================
																// 2026
																// source - https://help.autodesk.com/view/OARX/2026/ENU/?guid=GUID-2553CF98-44F6-4828-82DD-FE3BC7448113
																//------------------------------------------------------------------------------------------------
			if (false);											//
			else if (InRange(code,    0,    9)) return str;	 	//	    0-   9	| String (with the introduction of extended symbol names in AutoCAD 2000,
																//				| the 255-character limit has been increased to 2049 single-byte characters
																//				| not including the newline at the end of the line)
																//				| see the "Storage of String Values" section for more information
			else if (InRange(code,   10,   17)) return dbl;		//	   10-  17	| Double precision 3D point value
			else if (InRange(code,   20,   27)) return dbl;		//	   20-  27	| Double precision 3D point value
			else if (InRange(code,   30,   37)) return dbl;		//	   30-  37	| Double precision 3D point value
			else if (InRange(code,   38,   59)) return dbl;		//	   38-  59	| Double-precision floating-point value
			else if (InRange(code,   60,   79)) return i16;		//	   60-  79	| 16-bit integer value
			else if (InRange(code,   90,   99)) return i32;		//	   90-  99	| 32-bit integer value
			else if (InRange(code,  100,  102)) return str;		//	  100- 102	| String (255-character maximum, less for Unicode strings)
			else if (InRange(code,  105,  105)) return str;		//		   105	| String representing hexadecimal (hex) handle value
			else if (InRange(code,  110,  119)) return dbl;		//	  110- 119	| Double precision floating-point value
			else if (InRange(code,  120,  129)) return dbl;		//	  120- 129	| Double precision floating-point value
			else if (InRange(code,  130,  139)) return dbl;		//	  130- 139	| Double precision floating-point value
			else if (InRange(code,  140,  149)) return dbl;		//	  140- 149	| Double precision scalar floating-point value
			else if (InRange(code,  160,  169)) return i64;		//	  160- 169	| 64-bit integer value
			else if (InRange(code,  170,  179)) return i16;		//	  170- 179	| 16-bit integer value
			else if (InRange(code,  210,  239)) return dbl;		//	  210- 239	| Double-precision floating-point value
			else if (InRange(code,  270,  279)) return i16;		//	  270- 279	| 16-bit integer value
			else if (InRange(code,  280,  289)) return i16;		//	  280- 289	| 16-bit integer value
			else if (InRange(code,  290,  299)) return boolean;	//	  290- 299	| Boolean flag value
			else if (InRange(code,  300,  309)) return str;		//	  300- 309	| Arbitrary text string;
			else if (InRange(code,  310,  319)) return binary;	//	  310- 319	| String representing hex value of binary chunk
			else if (InRange(code,  320,  329)) return str;		//	  320- 329	| String representing hex handle value
			else if (InRange(code,  330,  369)) return str;		//	  330- 369	| String representing hex object IDs
			else if (InRange(code,  370,  379)) return i16;		//	  370- 379	| 16-bit integer value
			else if (InRange(code,  380,  389)) return i16;		//	  380- 389	| 16-bit integer value
			else if (InRange(code,  390,  399)) return str;		//	  390- 399	| String representing hex handle value
			else if (InRange(code,  400,  409)) return i16;		//	  400- 409	| 16-bit integer value
			else if (InRange(code,  410,  419)) return str;		//	  410- 419	| String;
			else if (InRange(code,  420,  429)) return i32;		//	  420- 429	| 32-bit integer value
			else if (InRange(code,  430,  439)) return str;		//	  430- 439	| String;
			else if (InRange(code,  440,  449)) return i32;		//	  440- 449	| 32-bit integer value
			else if (InRange(code,  450,  459)) return i32;		//	  450- 459	| Long ???
			else if (InRange(code,  460,  469)) return dbl;		//	  460- 469	| Double-precision floating-point value
			else if (InRange(code,  470,  479)) return str;		//	  470- 479	| String;
			else if (InRange(code,  480,  481)) return str;		//	  480- 481	| String representing a hex handle value
			else if (InRange(code,  999,  999)) return str;		//		   999	| Comment (string);
			else if (InRange(code, 1000, 1003)) return str;		//	 1000-1003	| String (same limits as indicated with 0-9 code range)
			else if (InRange(code, 1004, 1004)) return binary;	//		  1004	| String representing a hex value of binary chunk
			else if (InRange(code, 1005, 1005)) return str;		//		  1005	| String (same limits as indicated with 0-9 code range)
			else if (InRange(code, 1010, 1013)) return dbl;		//	 1010-1013	| Double-precision floating-point value
			else if (InRange(code, 1020, 1023)) return dbl;		//	 1020-1023	| Double-precision floating-point value
			else if (InRange(code, 1030, 1033)) return dbl;		//	 1030-1033	| Double-precision floating-point value
			else if (InRange(code, 1040, 1042)) return dbl;		//	 1040-1042	| Double-precision floating-point value
			else if (InRange(code, 1070, 1070)) return i16;		//		  1070	| 16-bit integer value
			else if (InRange(code, 1071, 1071)) return i32;		//		  1071	| 32-bit integer value
			else return none;
		}

		eGROUP_VALUE_TYPE GetValueTypeEnumByCode() const { return GetValueTypeEnum(eCode); }
		static eGROUP_VALUE_TYPE GetValueTypeEnum(eGROUP_CODE eCode) {
			constinit static auto const s_tblGroupCodeToType = [] {
				std::array<eGROUP_VALUE_TYPE, eGROUP_CODE::last> tbl{};
				for (eGROUP_CODE code{}; code.eCode < (eGROUP_CODE::enum_value_t)tbl.size(); code.eCode++) {
					tbl[code.eCode] = GET_VALUE_TYPE_ENUM(code);
				}
				return tbl;
			}();

			if (eCode < 0 or eCode.eCode >= (eGROUP_CODE::enum_value_t)s_tblGroupCodeToType.size())
				return eGROUP_VALUE_TYPE::none;
			return s_tblGroupCodeToType[eCode.eCode];
		}
	};

	// fmt
	inline auto format_as(eGROUP_CODE eValueType) { return eValueType.eCode; }
	inline auto format_as(sGroup const& group) {
		return std::pair{ group.eCode, group.value };
	}

	using groups_t = std::vector<sGroup>;
	struct group_iter_t : public groups_t::const_iterator {
		using this_t = group_iter_t;
		using base_t = groups_t::const_iterator;
	public:
	#ifdef _DEBUG
		base_t const begin;
	#endif
		base_t const end;
		group_iter_t(base_t iter, base_t end) : base_t(iter),
		#ifdef _DEBUG
			begin{iter},
		#endif
			end(end)
		{ }
		template < typename tContainer >
		explicit group_iter_t(tContainer const& container) : base_t{container.begin()},
		#ifdef _DEBUG
			begin{container.begin()},
		#endif
			end{container.end()}
		{ }

	#ifdef _DEBUG
		auto Distance() const { return std::distance(begin, (base_t const&)*this); }
	#endif

		//using base_t::base_t;
		using base_t::operator*;
		using base_t::operator->;
		auto& operator ++()		{ base_t::operator++(); return *this; }
		auto operator ++(int)	{ auto temp = *this; ++(*this); return temp; }
		auto& operator --()		{ base_t::operator--(); return *this; }
		auto operator --(int)	{ auto temp = *this; --(*this); return temp; }

		explicit operator bool() const { return (base_t const&)*this != end; }

		bool NextNotNull() const {
			return ((base_t const&)*this) + 1 != end;
		}
	};

	//=============================================================================================================================
	namespace entities {
		template < typename TField, bool bCheckMarker = false >
		bool ReadFieldMembers(TField& field, group_iter_t& iter);
	}
	//=============================================================================================================================
	namespace detail {
		struct tagGVSimple {};
		struct tagGVStruct {};
		struct tagGVString {};
	}	// namespace detail

	//----------------------------------------------------------------
	template < typename tag, typename TUserDefined, eGROUP_CODE ... eGroupCodes >
	struct TGroupVariable {
	public:
		using this_t = TGroupVariable;
		constexpr inline static eGROUP_CODE const eCodes[]{eGroupCodes...};
		constexpr inline static eGROUP_CODE const eCode{eCodes[0]};
		constexpr inline static eGROUP_VALUE_TYPE const eValueType = sGroup::GET_VALUE_TYPE_ENUM(eCodes[0]);

		using implicit_value_type =
			std::conditional_t<eValueType == eGROUP_VALUE_TYPE::boolean, bool,
			std::conditional_t<eValueType == eGROUP_VALUE_TYPE::i16, int16,
			std::conditional_t<eValueType == eGROUP_VALUE_TYPE::i32, int32,
			std::conditional_t<eValueType == eGROUP_VALUE_TYPE::i64, int64,
			std::conditional_t<eValueType == eGROUP_VALUE_TYPE::dbl, double,
			std::conditional_t<eValueType == eGROUP_VALUE_TYPE::str, string_t,
			std::conditional_t<eValueType == eGROUP_VALUE_TYPE::binary, binary_t, void>>>>>>>;

		using value_t = std::conditional_t<std::is_same_v<TUserDefined, void>, implicit_value_type, TUserDefined>;

		static_assert(!std::is_same_v<tag, detail::tagGVSimple> or sizeof(value_t) == sizeof(implicit_value_type));
		//static_assert(!std::is_same_v<tag, detail::tagGVStruct> or gtl::CountStructMember<TUserDefined>() == sizeof ... (eGroupCodes));
		static_assert(!std::is_same_v<tag, detail::tagGVString> or 2 == sizeof ... (eGroupCodes));
		static_assert(gtl::concepts::is_one_of<tag, detail::tagGVSimple, detail::tagGVStruct, detail::tagGVString>);

	public:
		value_t value{};

		constexpr static bool IsGroupCodeMatch(eGROUP_CODE group) {
			for (auto const& eCode : eCodes) {
				if (eCode == group.eCode)
					return true;
			}
			return false;
		}
		constexpr static bool IsFirstGroupCodeMatch(eGROUP_CODE group) {
			return eCodes[0] == group.eCode;
		}
		template < typename TField >
		bool SetFromGroup(TField& field, group_iter_t& iter) {
			auto const& group = *iter;
			bool bFound{};
			if constexpr (std::is_same_v<tag, detail::tagGVSimple>) {
				bFound = group.GetValue(value);
			}
			else if constexpr (std::is_same_v<tag, detail::tagGVStruct>) {
				constexpr auto N = gtl::CountStructMember<TUserDefined>();
				static_assert(N >= sizeof ... (eGroupCodes));
				bFound = gtl::ForEachIntSeq<N>([&]<int I>{
					if (eCodes[I] != group.eCode)
						return false;
					group.GetValue(gtl::GetStructMember<I>(value));
					return true;
				});
			}
			else if constexpr (std::is_same_v<tag, detail::tagGVString>) {
				if (group.eCode == eCodes[0]) {
					group.GetValue(value);
					bFound = true;
				} else if (group.eCode == eCodes[1]) {
					if (string_t str; group.GetValue(str))
						value += str;
					bFound = true;
				}
			}
			else {
				static_assert(false);
			}
			if (bFound)
				iter++;
			return bFound;
		}

	public:
		//GTL__DXF_DEFINE_SPACESHIP_OPERATOR(this_t);
		//auto operator <=> (this_t const&) const = default;
		bool operator == (this_t const& r) const {
			if constexpr (requires (value_t v) { v.value(); }) {
				return value.value() == r.value.value();
			}
			else {
				return value == r.value;
			}
		}
		auto operator <=> (this_t const& r) const {
			if constexpr (requires (value_t v) { v.value(); }) {
				return value.value() <=> r.value.value();
			}
			else {
				return value <=> r.value;
			}
		}

		std::partial_ordering Compare(sGroup const& group) const requires (std::size(eCodes) == 1) {
			if (std::partial_ordering r = eCodes[0] <=> group.eCode; r != 0)
				return r;
			return value <=> group.value;
		}

		size_t CountStoredVars() const {
			return 0;
		}

		this_t& operator = (this_t const&) = default;
		this_t& operator = (value_t const& v) { value = v; return *this; }
		this_t& operator = (value_t&& v) { value = std::move(v); return *this; }

		value_t& operator() () { return value; }
		value_t const& operator() () const { return value; }
		operator value_t& () { return value; }
		operator value_t const& () const { return value; }

	};

	template < typename TStruct, auto Count_or_OffsetToCountMember >
	struct TListVariable {
	public:
		using this_t = TListVariable;
		using value_t = std::vector<TStruct>;

	public:
		value_t value;

		constexpr static bool IsGroupCodeMatch(eGROUP_CODE group) {
			if constexpr (requires (TStruct t) { t.IsGroupCodeMatch; }) {
				return TStruct::IsGroupCodeMatch(group);
			}
			else if constexpr (requires (TStruct t) { t.marker; }) {
				return eGROUP_CODE::subclass == group;
			}
			else {
				// todo ....... for all members
				constexpr static size_t nMemberSize = gtl::CountStructMember<TStruct>();
				TStruct field;
				bool bFound = gtl::ForEachIntSeq<nMemberSize>(
					[&]<int I>{
					using member_t = gtl::struct_member_t<I, TStruct>;
					if constexpr (requires (member_t m) { m.IsGroupCodeMatch; }) {			// member with group code
						return member_t::IsGroupCodeMatch(group.eCode);
					}
					else if constexpr (requires (member_t m) { m.marker; m.marker.str; }) {
						return group == eGROUP_CODE::subclass;
					}
					return false;
				});
				return bFound;
			}
		}
		constexpr static bool IsFirstGroupCodeMatch(eGROUP_CODE eCode) {
			if constexpr (requires (TStruct t) { t.IsFirstGroupCodeMatch; }) {
				return TStruct::IsFirstGroupCodeMatch(eCode);
			}
			else if constexpr (requires (TStruct t) { t.marker; }) {
				return eGROUP_CODE::subclass == eCode;
			}
			else if constexpr (gtl::CountStructMember<TStruct>() > 1) {
				using member_t = gtl::struct_member_t<0, TStruct>;
				return member_t::IsFirstGroupCodeMatch(eCode);
			}
			else {
				static_assert(false);
				return false;
			}
		}
		template < typename TField >
		bool SetFromGroup(TField& field, group_iter_t& iter) {
			// check if the variable count
			// if first, push back a new element
			if (IsFirstGroupCodeMatch(iter->eCode)) {
				if constexpr (Count_or_OffsetToCountMember) {
					if constexpr (std::is_integral_v<decltype(Count_or_OffsetToCountMember)>) {
						if (Count_or_OffsetToCountMember <= value.size())
							return false;
					}
					else if constexpr (std::is_member_pointer_v<decltype(Count_or_OffsetToCountMember)>) {
						if ((field.*Count_or_OffsetToCountMember).value <= value.size())
							return false;
					}
					else {
						static_assert(false, "Count_or_OffsetToCountMember must be integral or member pointer type");
					}
				}
				value.emplace_back();
			}
			if constexpr (requires (TStruct t) { t.SetFromGroup; }) {
				if (!value.back().SetFromGroup(field, iter))
					return false;
			}
			else {
				if (value.empty()) {
				#ifdef _DEBUG
					auto distance = iter.Distance();
				#endif
					return false;
				}
				if (!entities::ReadFieldMembers(value.back(), iter))
					return false;
			}
			return true;
		}

	public:
		GTL__DXF_DEFINE_SPACESHIP_OPERATOR(this_t);

		size_t CountStoredVars() const {
			return value.size();
		}

		value_t& operator() () { return value; }
		value_t const& operator() () const { return value; }
		operator value_t& () { return value; }
		operator value_t const& () const { return value; }

	};

#pragma pack(pop)
}	// namespace gtl::dxf
