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


/*export*/ namespace gtl::ui::unit {

	template < typename T, typename CLASS, xStringLiteral TUNIT_NAME >
	struct TPosLength {
		using value_type = T;
		using this_t = TPosLength;
		static inline auto const unit_name = TUNIT_NAME.str;

		value_type value{};

		//operator value_type& () { return value; }
		//operator value_type () const { return value; }

		[[ nodiscard ]] auto operator <=> (this_t const& ) const = default;
	};

	struct px_t :		public TPosLength<float, px_t,		GText("px")> {};
	struct em_t :		public TPosLength<float, em_t,		GText("em")> {};
	struct rem_t :		public TPosLength<float, rem_t,		GText("rem")> {};
	struct vw_t :		public TPosLength<float, rem_t,		GText("vw")> {};
	struct vh_t :		public TPosLength<float, rem_t,		GText("vh")> {};
	struct vmin_t :		public TPosLength<float, rem_t,		GText("vmin")> {};
	struct vmax_t :		public TPosLength<float, rem_t,		GText("vmax")> {};
	struct percent_t :	public TPosLength<float, percent_t,	GText("%")> {};
	struct dummy_t :	public TPosLength<float, rem_t,		GText("")> {};

	using position_t = std::variant<dummy_t, px_t, em_t, rem_t, vw_t, vh_t, vmin_t, vmax_t, percent_t>;

	template < typename TUNIT >
	struct TFormatter : public fmt::formatter<typename std::remove_cvref_t<TUNIT>::value_type, gtl::ui::char_type> {
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
};

//---------------------------------------------------------------------------------------------------------------------------------

/*export*/ template <> struct fmt::formatter<gtl::ui::unit::dummy_t, gtl::ui::char_type> : gtl::ui::unit::TFormatter<gtl::ui::unit::dummy_t> {};
/*export*/ template <> struct fmt::formatter<gtl::ui::unit::px_t, gtl::ui::char_type> : gtl::ui::unit::TFormatter<gtl::ui::unit::px_t> {};
/*export*/ template <> struct fmt::formatter<gtl::ui::unit::em_t, gtl::ui::char_type> : gtl::ui::unit::TFormatter<gtl::ui::unit::em_t> {};
/*export*/ template <> struct fmt::formatter<gtl::ui::unit::rem_t, gtl::ui::char_type> : gtl::ui::unit::TFormatter<gtl::ui::unit::rem_t> {};
/*export*/ template <> struct fmt::formatter<gtl::ui::unit::vw_t, gtl::ui::char_type> : gtl::ui::unit::TFormatter<gtl::ui::unit::vw_t> {};
/*export*/ template <> struct fmt::formatter<gtl::ui::unit::vh_t, gtl::ui::char_type> : gtl::ui::unit::TFormatter<gtl::ui::unit::vh_t> {};
/*export*/ template <> struct fmt::formatter<gtl::ui::unit::vmin_t, gtl::ui::char_type> : gtl::ui::unit::TFormatter<gtl::ui::unit::vmin_t> {};
/*export*/ template <> struct fmt::formatter<gtl::ui::unit::vmax_t, gtl::ui::char_type> : gtl::ui::unit::TFormatter<gtl::ui::unit::vmax_t> {};
/*export*/ template <> struct fmt::formatter<gtl::ui::unit::percent_t, gtl::ui::char_type> : gtl::ui::unit::TFormatter<gtl::ui::unit::percent_t> {};

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


/*export*/ namespace gtl::ui::inline literals {
	constexpr unit::px_t		operator "" _px (long double v)		{ return unit::px_t{(float)v}; }
	constexpr unit::px_t		operator "" _px (uint64_t v)		{ return unit::px_t{(float)v}; }

	constexpr unit::em_t		operator "" _em (long double v)		{ return unit::em_t{(float)v}; }
	constexpr unit::em_t		operator "" _em (uint64_t v)		{ return unit::em_t{(float)v}; }

	constexpr unit::rem_t		operator "" _rem (long double v)	{ return unit::rem_t{(float)v}; }
	constexpr unit::rem_t		operator "" _rem (uint64_t v)		{ return unit::rem_t{(float)v}; }

	constexpr unit::vw_t		operator "" _vw (long double v)		{ return unit::vw_t{(float)v}; }
	constexpr unit::vw_t		operator "" _vw (uint64_t v)		{ return unit::vw_t{(float)v}; }

	constexpr unit::vh_t		operator "" _vh (long double v)		{ return unit::vh_t{(float)v}; }
	constexpr unit::vh_t		operator "" _vh (uint64_t v)		{ return unit::vh_t{(float)v}; }

	constexpr unit::vmin_t		operator "" _vmin (long double v)	{ return unit::vmin_t{(float)v}; }
	constexpr unit::vmin_t		operator "" _vmin (uint64_t v)		{ return unit::vmin_t{(float)v}; }

	constexpr unit::vmax_t		operator "" _vmax (long double v)	{ return unit::vmax_t{(float)v}; }
	constexpr unit::vmax_t		operator "" _vmax (uint64_t v)		{ return unit::vmax_t{(float)v}; }

	constexpr unit::percent_t	operator "" _pct (long double v)	{ return unit::percent_t{(float)v}; }
	constexpr unit::percent_t	operator "" _pct (uint64_t v)		{ return unit::percent_t{(float)v}; }

}
