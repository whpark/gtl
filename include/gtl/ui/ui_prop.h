//module;
//
//#include "gtl/gtl.h"
//#include "gtl.ui.h"
//
//export module gtl.ui:widget_property;
//import :predefine;
//import :prop.value.etc;
//import :prop.value.color;
//
//using namespace std::literals;
//using namespace gtl::literals;
//using namespace gtl::ui::unit::literals;


#pragma once

#include "ui_predefine.h"
#include "ui_prop_value_etc.h"
#include "ui_prop_value_color.h"
#include "ui_unit.h"

/*export*/ namespace gtl::ui::inline prop {

	inline namespace name {
		static inline constexpr string_t const text_align		{ GText("text-align"s) };
		static inline constexpr string_t const background		{ GText("background"s) };
		static inline constexpr string_t const width			{ GText("width"s) };
		static inline constexpr string_t const height			{ GText("height"s) };
		static inline constexpr string_t const max_width		{ GText("max-width"s) };
		static inline constexpr string_t const min_width		{ GText("min-width"s) };
		static inline constexpr string_t const max_height		{ GText("max-height"s) };
		static inline constexpr string_t const min_height		{ GText("min-height"s) };
		static inline constexpr string_t const border			{ GText("border"s) };
		static inline constexpr string_t const margin			{ GText("margin"s) };
		static inline constexpr string_t const padding			{ GText("padding"s) };
	}

	//==============================================================================================================================
	//! @brief widget appearance
	struct GTL__CLASS xWidgetProperty {
	public:
		using this_t = xWidgetProperty;

		string_t m_name, m_value;

		//constexpr xWidgetProperty(string_view_t prop, string_view_t value) : m_property(prop), m_value(value) {}
		//virtual ~xWidgetProperty() {}

		string_t GetStyle() const {
			return fmt::format(GText("{}:{};"), m_name, m_value);
		}

		auto operator <=> (xWidgetProperty const&) const = default;
		bool operator == (string_t const& name) const {
			return m_name == name;
		}

	};

	template < typename ... Args >
	xWidgetProperty MakeProperty(string_t name, Args&& ... value) {
		xWidgetProperty prop;
		prop.m_name = std::move(name);
		if constexpr ((sizeof ... (Args) == 1) and std::is_convertible_v<Args, string_t>) {
			prop.m_value = std::move(value);
		}
		else {
			prop.m_value = ToString(std::forward<Args>(args)...);
		}
		return prop;
	}

	//==============================================================================================================================

	/// @brief TextAlign
	/// @ex TextAlign(gtl::ui::prop::value::text_align::center);
	constexpr inline xWidgetProperty TextAlign(string_t const& prop) {
		return xWidgetProperty{gtl::ui::prop::name::text_align, prop};
	}

	template < typename ... Args >
	constexpr static xWidgetProperty Background(Args&& ... args) {
		return MakeProperty(gtl::ui::prop::name::background, std::forward<Args>(args)...);
	}

	/// @brief Width
	inline xWidgetProperty Width(position_t const& length) {
		return MakeProperty(gtl::ui::prop::name::width, length);
	}
	/// @brief height
	inline xWidgetProperty Height(position_t const& length) {
		return MakeProperty(gtl::ui::prop::name::height, length);
	}
	/// @brief max-width
	inline xWidgetProperty MaxWidth(position_t const& length) {
		return MakeProperty(gtl::ui::prop::name::max_width, length);
	}
	/// @brief max-height
	inline xWidgetProperty MaxHeight(position_t const& length) {
		return MakeProperty(gtl::ui::prop::name::max_height, length);
	}
	/// @brief min-width
	inline xWidgetProperty MinWidth(position_t const& length) {
		return MakeProperty(gtl::ui::prop::name::min_width, length);
	}
	/// @brief min-height
	inline xWidgetProperty MinHeight(position_t const& length) {
		return MakeProperty(gtl::ui::prop::name::min_height, length);
	}


	/// @brief 
	/// @tparam ...ANY_OF_PX_EM_REM_PERCENT_STRING 
	/// @param ...args 
	/// @return
	/// @ex Border(3_px, 3_px, u8"solid", u8"green");
	/// @ex Border(3_px, 3_px, u8"solid", color::AliceBlue);
	template < typename ... ANY_OF_PX_EM_REM_PERCENT_STRING >
	xWidgetProperty Border(ANY_OF_PX_EM_REM_PERCENT_STRING&& ... args) {
		static_assert(sizeof... (args) > 0 and sizeof... (args) <= 4);
		return MakeProperty(gtl::ui::prop::name::border, args...);
	}

	/// @brief 
	/// @tparam ...ANY_OF_PX_EM_REM_PERCENT 
	/// @param ...args 
	/// @return 
	/// @ex Margin(10_px);
	/// @ex Margin(0.1_em, 0.1_em);
	/// @ex Margin(0.1_rem, 0.1_rem);
	inline xWidgetProperty Margin(position_t const& p1) { return MakeProperty(gtl::ui::prop::name::margin, p1); }
	inline xWidgetProperty Margin(position_t const& p1, position_t const& p2) { return MakeProperty(gtl::ui::prop::name::margin, p1, p2); }
	inline xWidgetProperty Margin(position_t const& p1, position_t const& p2, position_t const& p3, position_t const& p4) {
		return MakeProperty(gtl::ui::prop::name::margin, p1, p2, p3, p4);
	}

	/// @brief 
	/// @tparam ...ANY_OF_PX_EM_REM_PERCENT 
	/// @param ...args 
	/// @return 
	/// @ex Padding(10_px);
	/// @ex Padding(0.1_em, 0.1_em);
	/// @ex Padding(0.1_rem, 0.1_rem);
	inline xWidgetProperty Padding(position_t const& p1) { return MakeProperty(gtl::ui::prop::name::padding, p1); }
	inline xWidgetProperty Padding(position_t const& p1, position_t const& p2) { return MakeProperty(gtl::ui::prop::name::padding, p1, p2); }
	inline xWidgetProperty Padding(position_t const& p1, position_t const& p2, position_t const& p3, position_t const& p4) {
		return MakeProperty(gtl::ui::prop::name::padding, p1, p2, p3, p4);
	}

};

