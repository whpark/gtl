//module;
//
//#include "gtl/gtl.h"
//#include "gtl.ui.h"
//
//#define BOOST_JSON_STANDALONE
//#include "boost/json.hpp"
//
//
//export module gtl.ui:predefine;
//
//using namespace std::literals;
//using namespace gtl::literals;


#pragma once

#include "ui_predefine.h"
#include "gtl/misc.h"

//#define BOOST_JSON_STANDALONE
//#include "boost/json.hpp"


/*export*/ namespace gtl::ui::inline unit {

	template < typename T, xStringLiteral UNIT_NAME >
	struct TPosLength {
		using value_type = T;
		using this_t = TPosLength;
		static inline constexpr auto const unit_name = UNIT_NAME.str;

		value_type value{};

		//operator value_type& () { return value; }
		//operator value_type () const { return value; }

		[[ nodiscard ]] auto operator <=> (this_t const& ) const = default;
	};

	using px_t =		TPosLength<float, GText("px")>;
	using em_t =		TPosLength<float, GText("em")>;
	using rem_t =		TPosLength<float, GText("rem")>;
	using vw_t =		TPosLength<float, GText("vw")>;
	using vh_t =		TPosLength<float, GText("vh")>;
	using vmin_t =		TPosLength<float, GText("vmin")>;
	using vmax_t =		TPosLength<float, GText("vmax")>;
	using percent_t =	TPosLength<float, GText("%")>;
	using dummy_t =		TPosLength<float, GText("")>;

	using position_t = std::variant<dummy_t, px_t, em_t, rem_t, vw_t, vh_t, vmin_t, vmax_t, percent_t>;

	template < typename TUNIT >
	struct TFMTUnitFormatter : public fmt::formatter<typename std::remove_cvref_t<TUNIT>::value_type, gtl::ui::char_type> {
		using base_t = fmt::formatter<typename std::remove_cvref_t<TUNIT>::value_type, gtl::ui::char_type>;
		template < typename context >
		auto format(TUNIT const& v, context& ctx) {
			if constexpr (std::is_same_v<std::remove_cvref_t<TUNIT>, dummy_t>) {
				return ctx.out();
			} else {
				base_t::format(v.value, ctx);
				// todo: has bug. ex) format("{:<10}", 3_px) ==> "3         px"
				return fmt::format_to(ctx.out(), std::remove_cvref_t<TUNIT>::unit_name);
			}
		}
	};

	template < typename TUNIT >
	struct TSTDUnitFormatter : public std::formatter<typename std::remove_cvref_t<TUNIT>::value_type, gtl::ui::char_type> {
		using base_t = std::formatter<typename std::remove_cvref_t<TUNIT>::value_type, gtl::ui::char_type>;
		template < typename context >
		auto format(TUNIT const& v, context& ctx) {
			if constexpr (std::is_same_v<std::remove_cvref_t<TUNIT>, dummy_t>) {
				return ctx.out();
			} else {
				base_t::format(v.value, ctx);
				// todo: has bug. ex) format("{:<10}", 3_px) ==> "3         px"
				return std::format_to(ctx.out(), std::remove_cvref_t<TUNIT>::unit_name);
			}
		}
	};

	//---------------------------------------------------------------------------------------------------------------------------------

};	// namespace gtl::ui::unit

/*export*/ template <> struct fmt::formatter<gtl::ui::unit::dummy_t,	gtl::ui::char_type> : gtl::ui::unit::TFMTUnitFormatter<gtl::ui::unit::dummy_t> {};
/*export*/ template <> struct fmt::formatter<gtl::ui::unit::px_t,		gtl::ui::char_type> : gtl::ui::unit::TFMTUnitFormatter<gtl::ui::unit::px_t> {};
/*export*/ template <> struct fmt::formatter<gtl::ui::unit::em_t,		gtl::ui::char_type> : gtl::ui::unit::TFMTUnitFormatter<gtl::ui::unit::em_t> {};
/*export*/ template <> struct fmt::formatter<gtl::ui::unit::rem_t,		gtl::ui::char_type> : gtl::ui::unit::TFMTUnitFormatter<gtl::ui::unit::rem_t> {};
/*export*/ template <> struct fmt::formatter<gtl::ui::unit::vw_t,		gtl::ui::char_type> : gtl::ui::unit::TFMTUnitFormatter<gtl::ui::unit::vw_t> {};
/*export*/ template <> struct fmt::formatter<gtl::ui::unit::vh_t,		gtl::ui::char_type> : gtl::ui::unit::TFMTUnitFormatter<gtl::ui::unit::vh_t> {};
/*export*/ template <> struct fmt::formatter<gtl::ui::unit::vmin_t,		gtl::ui::char_type> : gtl::ui::unit::TFMTUnitFormatter<gtl::ui::unit::vmin_t> {};
/*export*/ template <> struct fmt::formatter<gtl::ui::unit::vmax_t,		gtl::ui::char_type> : gtl::ui::unit::TFMTUnitFormatter<gtl::ui::unit::vmax_t> {};
/*export*/ template <> struct fmt::formatter<gtl::ui::unit::percent_t,	gtl::ui::char_type> : gtl::ui::unit::TFMTUnitFormatter<gtl::ui::unit::percent_t> {};
/*export*/ template <> struct fmt::formatter<gtl::ui::unit::position_t, gtl::ui::char_type> : fmt::formatter<float, gtl::ui::char_type> {
	using base_t = fmt::formatter<float, gtl::ui::char_type>;
	template < typename context > auto format(gtl::ui::unit::position_t const& v, context& ctx) {
		//gtl::ui::string_t str;
		std::visit([&]<typename T>(T&& arg) {
			base_t::format((float)arg.value, ctx);
			fmt::format_to(ctx.out(), std::remove_cvref_t<T>::unit_name);
		}, v);
		return ctx.out();
	}
};


/*export*/ template <> struct std::formatter<gtl::ui::unit::dummy_t,	gtl::ui::char_type> : gtl::ui::unit::TSTDUnitFormatter<gtl::ui::unit::dummy_t> {};
/*export*/ template <> struct std::formatter<gtl::ui::unit::px_t,		gtl::ui::char_type> : gtl::ui::unit::TSTDUnitFormatter<gtl::ui::unit::px_t> {};
/*export*/ template <> struct std::formatter<gtl::ui::unit::em_t,		gtl::ui::char_type> : gtl::ui::unit::TSTDUnitFormatter<gtl::ui::unit::em_t> {};
/*export*/ template <> struct std::formatter<gtl::ui::unit::rem_t,		gtl::ui::char_type> : gtl::ui::unit::TSTDUnitFormatter<gtl::ui::unit::rem_t> {};
/*export*/ template <> struct std::formatter<gtl::ui::unit::vw_t,		gtl::ui::char_type> : gtl::ui::unit::TSTDUnitFormatter<gtl::ui::unit::vw_t> {};
/*export*/ template <> struct std::formatter<gtl::ui::unit::vh_t,		gtl::ui::char_type> : gtl::ui::unit::TSTDUnitFormatter<gtl::ui::unit::vh_t> {};
/*export*/ template <> struct std::formatter<gtl::ui::unit::vmin_t,		gtl::ui::char_type> : gtl::ui::unit::TSTDUnitFormatter<gtl::ui::unit::vmin_t> {};
/*export*/ template <> struct std::formatter<gtl::ui::unit::vmax_t,		gtl::ui::char_type> : gtl::ui::unit::TSTDUnitFormatter<gtl::ui::unit::vmax_t> {};
/*export*/ template <> struct std::formatter<gtl::ui::unit::percent_t,	gtl::ui::char_type> : gtl::ui::unit::TSTDUnitFormatter<gtl::ui::unit::percent_t> {};
/*export*/ template <> struct std::formatter<gtl::ui::unit::position_t, gtl::ui::char_type> : std::formatter<float, gtl::ui::char_type> {
	using base_t = std::formatter<float, gtl::ui::char_type>;
	template < typename context > auto format(gtl::ui::unit::position_t const& v, context& ctx) {
		//gtl::ui::string_t str;
		std::visit([&]<typename T>(T&& arg) {
			base_t::format((float)arg.value, ctx);
			std::format_to(ctx.out(), std::remove_cvref_t<T>::unit_name);
		}, v);
		return ctx.out();
	}
};


/*export*/ namespace gtl::ui::inline literals {
	constexpr gtl::ui::unit::px_t		operator "" _px (long double v)			{ return gtl::ui::unit::px_t{(gtl::ui::unit::px_t::value_type)v}; }
	constexpr gtl::ui::unit::px_t		operator "" _px (uint64_t v)			{ return gtl::ui::unit::px_t{(gtl::ui::unit::px_t::value_type)v}; }

	constexpr gtl::ui::unit::em_t		operator "" _em (long double v)			{ return gtl::ui::unit::em_t{(gtl::ui::unit::em_t::value_type)v}; }
	constexpr gtl::ui::unit::em_t		operator "" _em (uint64_t v)			{ return gtl::ui::unit::em_t{(gtl::ui::unit::em_t::value_type)v}; }

	constexpr gtl::ui::unit::rem_t		operator "" _rem (long double v)		{ return gtl::ui::unit::rem_t{(gtl::ui::unit::rem_t::value_type)v}; }
	constexpr gtl::ui::unit::rem_t		operator "" _rem (uint64_t v)			{ return gtl::ui::unit::rem_t{(gtl::ui::unit::rem_t::value_type)v}; }

	constexpr gtl::ui::unit::vw_t		operator "" _vw (long double v)			{ return gtl::ui::unit::vw_t{(gtl::ui::unit::vw_t::value_type)v}; }
	constexpr gtl::ui::unit::vw_t		operator "" _vw (uint64_t v)			{ return gtl::ui::unit::vw_t{(gtl::ui::unit::vw_t::value_type)v}; }

	constexpr gtl::ui::unit::vh_t		operator "" _vh (long double v)			{ return gtl::ui::unit::vh_t{(gtl::ui::unit::vh_t::value_type)v}; }
	constexpr gtl::ui::unit::vh_t		operator "" _vh (uint64_t v)			{ return gtl::ui::unit::vh_t{(gtl::ui::unit::vh_t::value_type)v}; }

	constexpr gtl::ui::unit::vmin_t		operator "" _vmin (long double v)		{ return gtl::ui::unit::vmin_t{(gtl::ui::unit::vmin_t::value_type)v}; }
	constexpr gtl::ui::unit::vmin_t		operator "" _vmin (uint64_t v)			{ return gtl::ui::unit::vmin_t{(gtl::ui::unit::vmin_t::value_type)v}; }

	constexpr gtl::ui::unit::vmax_t		operator "" _vmax (long double v)		{ return gtl::ui::unit::vmax_t{(gtl::ui::unit::vmax_t::value_type)v}; }
	constexpr gtl::ui::unit::vmax_t		operator "" _vmax (uint64_t v)			{ return gtl::ui::unit::vmax_t{(gtl::ui::unit::vmax_t::value_type)v}; }

	constexpr gtl::ui::unit::percent_t	operator "" _percent (long double v)	{ return gtl::ui::unit::percent_t{(gtl::ui::unit::percent_t::value_type)v}; }
	constexpr gtl::ui::unit::percent_t	operator "" _percent (uint64_t v)		{ return gtl::ui::unit::percent_t{(gtl::ui::unit::percent_t::value_type)v}; }

}

