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

/*export*/ namespace gtl::ui::prop {

	//=============================================================================================================================
	//! @brief widget appearance
	struct xWidgetProperty {
	public:
		using this_t = xWidgetProperty;

		string_t m_property, m_value;

		//constexpr xWidgetProperty(string_view_t prop, string_view_t value) : m_property(prop), m_value(value) {}
		//virtual ~xWidgetProperty() {}

		string_t GetStyle() const {
			return fmt::format(GText("{}:{};"), m_property, m_value);
		}

	};


	using position_t = gtl::ui::unit::position_t;

	/// @brief TextAlign
	/// @ex TextAlign(gtl::ui::prop::value::text_align::center);
	constexpr static xWidgetProperty TextAlign(string_t const& prop)	{ return xWidgetProperty{GText("text-align"s), prop}; }

	template < typename ... Args >
	constexpr static xWidgetProperty Background(Args&& ... args) {
		return xWidgetProperty{GText(""), ToString(std::forward<Args>(args)...)};
	}

	/// @brief Width
	static xWidgetProperty Width(position_t const& length) {
		return xWidgetProperty(GText("width"s), ToString(length));
	}
	/// @brief Width
	static xWidgetProperty Height(position_t const& length) {
		return xWidgetProperty(GText("height"s), ToString(length));
	}
	/// @brief Width
	static xWidgetProperty MaxWidth(position_t const& length) {
		return xWidgetProperty(GText("max-width"s), ToString(length));
	}
	/// @brief Width
	static xWidgetProperty MaxHeight(position_t const& length) {
		return xWidgetProperty(GText("max-height"s), ToString(length));
	}


	/// @brief 
	/// @tparam ...ANY_OF_PX_EM_REM_PERCENT_STRING 
	/// @param ...args 
	/// @return
	/// @ex Border(3_px, 3_px, u8"solid", u8"green");
	/// @ex Border(3_px, 3_px, u8"solid", color::AliceBlue);
	template < typename ... ANY_OF_PX_EM_REM_PERCENT_STRING >
	static xWidgetProperty Border(ANY_OF_PX_EM_REM_PERCENT_STRING&& ... args) {
		static_assert(sizeof... (args) > 0 and sizeof... (args) <= 4);
		return xWidgetProperty{ GText("border"s), ToString(args...) };
	}

	/// @brief 
	/// @tparam ...ANY_OF_PX_EM_REM_PERCENT 
	/// @param ...args 
	/// @return 
	/// @ex Margin(10_px);
	/// @ex Margin(0.1_em, 0.1_em);
	/// @ex Margin(0.1_rem, 0.1_rem);
	static xWidgetProperty Margin(position_t const& p1) { return xWidgetProperty{ GText("margin"s), ToString(p1) }; }
	static xWidgetProperty Margin(position_t const& p1, position_t const& p2) { return xWidgetProperty{ GText("margin"s), ToString(p1, p2) }; }
	static xWidgetProperty Margin(position_t const& p1, position_t const& p2, position_t const& p3, position_t const& p4) { return xWidgetProperty{ GText("margin"s), ToString(p1, p2, p3, p4) }; }

	/// @brief 
	/// @tparam ...ANY_OF_PX_EM_REM_PERCENT 
	/// @param ...args 
	/// @return 
	/// @ex Padding(10_px);
	/// @ex Padding(0.1_em, 0.1_em);
	/// @ex Padding(0.1_rem, 0.1_rem);
	static xWidgetProperty Padding(position_t const& p1) { return xWidgetProperty{ GText("padding"s), ToString(p1) }; }
	static xWidgetProperty Padding(position_t const& p1, position_t const& p2) { return xWidgetProperty{ GText("padding"s), ToString(p1, p2) }; }
	static xWidgetProperty Padding(position_t const& p1, position_t const& p2, position_t const& p3, position_t const& p4) { return xWidgetProperty{ GText("padding"s), ToString(p1, p2, p3, p4) }; }

};

