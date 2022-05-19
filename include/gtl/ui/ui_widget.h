//module;
//
//#include "gtl/gtl.h"
//#include "gtl.ui.h"
//
//export module gtl.ui:widget;
//import :predefine;
//import :widget_property;
//
//using namespace std::literals;
//using namespace gtl::literals;

#pragma once

#include "ui_predefine.h"
#include "ui_prop.h"

namespace gtl::ui {

	//=============================================================================================================================
	//! @brief widget data
	class xWidgetData {
	public:
		using this_t = xWidgetData;
		using rWidgetData = std::shared_ptr<xWidgetData>;

	public:
		//enum class eALIGN : uint8_t {
		//	none,
		//	left_top,		center_top,		right_top,		stretch_top,
		//	left_center,	center_center,	right_center,	stretch_center,
		//	left_bottom,	center_bottom,	right_bottom,	stretch_bottom,
		//	left_stretch,	center_stretch,	right_stretch,	stretch_stretch,
		//};


	public:
		//=====================================================================
		// attributes
		//! @brief unique id
		//string_t m_id{MakeUniqueID()};
		id_t m_id{};	// Unique ID by framework.
		//! @brief name
		string_t m_name;
		//! @brief widget type (label, button, panel...)
		string_t m_tag;
		//! @brief widget class
		string_t m_class;
		//! @brief text to display as title
		string_t m_title;
		//! @brief value. value of item. ex, text value of text box
		string_t m_value;
		//! @brief text.
		string_t m_text;

		//=====================================================================
		// additional attributes
		std::vector<string_t> m_attributes;
		//! @brief CSS Style
		string_t m_style;
		std::vector<gtl::ui::prop::xWidgetProperty> m_properties;

		//=====================================================================
		// child items
		rWidgetData m_template;
		using item_t = string_t;//std::variant<void*, int64_t, double, string_t>;
		std::vector<item_t> m_items;

		//=====================================================================
		// embeded components
		std::deque<rWidgetData> m_components;

		//=====================================================================
		// Callback
		using event_handler_t = std::function<bool(rWidgetData rself, string_view_t evt, string_view_t content)>;
		event_handler_t fn_OnEvent;
		event_handler_t fn_OnClick, fn_OnRClick, fn_OnDblClick, fn_OnRDblClick;
		event_handler_t fn_Validate;
		event_handler_t fn_OnTextChanged;
		event_handler_t fn_OnItemSelect;

	#if 0
	protected:
		static inline std::atomic<int64_t> id{};
		static string_t MakeUniqueID() {
			return fmt::format(GText("uid{}"), ++id);
		}
	public:
		static void InitUniqueID() { id = 0; }
	#endif
	};
	static_assert(std::is_aggregate_v<xWidgetData>);
	using rWidgetData = std::shared_ptr<xWidgetData>;


	//=============================================================================================================================
	//! @brief widget
	class xWidget : public xWidgetData {
	public:
		using base_t = xWidgetData;
		using this_t = xWidget;

		using rWidget = std::shared_ptr<xWidget>;

		//public:
		//	// Constructor
		//	xWidget() = default;
		//	xWidget(xWidget const&) = default;
		//	xWidget& operator = (xWidget const&) = default;
		//	xWidget(xWidget&&) = default;
		//	xWidget& operator = (xWidget&&) = default;

		//	xWidget(base_t const& data) : base_t{data} {
		//	}

		xWidget& AddComponent(xWidget&& w) {
			m_components.emplace_back(std::make_shared<xWidget>(std::move(w)));
			return *this;
		}
		xWidget& AddComponent(xWidget const& w) {
			m_components.emplace_back(std::make_shared<xWidget>(w));
			return *this;
		}
		xWidget& AddComponent(std::shared_ptr<xWidget> r) {
			m_components.push_back(r);
			return *this;
		}

		rWidgetData FindComponent(id_t id/* 'parent:1st:2nd:*/) {
			return FindComponent(id, m_components);
		}

		static rWidgetData FindComponent(id_t id, std::deque<std::shared_ptr<xWidgetData>>& components) {
			//auto pos = strID.find(':');
			//string_view_t idChild;
			//id_t id{gtl::tsztoi<id_t>(strID)};
			//if (pos != strID.npos) {
			//	idChild = strID.substr(pos);
			//}

			for (auto& r : components) {
				if (r->m_id == id)
					return r;
				if (auto c = FindComponent(id, r->m_components); c)
					return c;
			}
			return {};
		}

	public:
		xWidget& SetCallback_OnClick(event_handler_t func)				{ fn_OnClick = func;		return *this; }
		xWidget& SetCallback_OnDblClick(event_handler_t func)			{ fn_OnDblClick = func;		return *this; }
		xWidget& SetCallback_OnRClick(event_handler_t func)				{ fn_OnRClick = func;		return *this; }
		xWidget& SetCallback_OnRDblClick(event_handler_t func)			{ fn_OnRDblClick = func;	return *this; }
		xWidget& SetCallback_OnTextChanged(event_handler_t func)		{ fn_OnTextChanged = func;	return *this; }
		xWidget& SetCallback_OnItemSelected(event_handler_t func)		{ fn_OnItemSelect = func;	return *this; }
		xWidget& SetCallback_OnEvent(event_handler_t func)				{ fn_OnEvent = func;		return *this; }

		string_t MakeHTML(id_t& id) {
			using namespace std::literals;

			string_t str;
			//std::basic_stringstream<gtl::ui::char_type> ss;
			auto strTagName = m_tag.empty() ? GText("div"s) : m_tag;

			auto strOpen = fmt::format(GText("<{}{}{}>"s), strTagName, GetAttributeString(), GetStyleString());

			string_t strComponents;
			for (auto& rChild : m_components) {
				rChild->m_id = ++id;
				strComponents += ((xWidget*)rChild.get())->MakeHTML(id);
			}

			string_t text{m_text};
			if (m_template) {
				xWidget templ { *m_template};
				for (auto const& item : m_items) {
					//templ.m_value = std::get<string_t>(item);
					if (!text.empty())
						text += ' ';
					templ.m_value = item;
					templ.m_text = item;
					text += templ.MakeHTML(id);
				}
			}

			auto strClose = fmt::format(GText("</{}>\n"s), strTagName);

			return strOpen + TranslateHTMLChar<true>(m_title) + strComponents + text + strClose;
		}

		string_t GetAttributeString() {
			string_t strAttribute;

			strAttribute += fmt::format(GText(R"xx( id="{}")xx"), m_id);

			if (!m_value.empty())
				strAttribute += fmt::format(GText(R"xx( value="{}")xx"), /*TranslateHTMLChar<true>*/(m_value));

			size_t len{};
			for (auto const& attr : m_attributes) {
				len += attr.size()+1;
			}
			if (len) {
				strAttribute.reserve(strAttribute.size() + len);
				for (auto const& attr : m_attributes) {
					strAttribute += GText(' ');
					strAttribute += attr;
				}
			}

			return strAttribute;
		}

		string_t GetStyleString() {
			using namespace std::literals;

			string_t strStyles;
			size_t len{};
			for (auto const& prop : m_properties) {
				len += prop.m_name.size() + prop.m_value.size() + 3;
			}

			if (len) {
				static auto const attrH = GText(" style=\""s);
				static auto const attrT = GText("\""s);
				strStyles.reserve(len + attrH.size() + attrT.size());
				strStyles = attrH;
				for (auto const& prop : m_properties) {
					strStyles += GText(' ');
					strStyles += prop.GetStyle();
				}
				strStyles += attrT;
			}
			return strStyles;
		}

		[[nodiscard]] static rWidget GroupBoxVertical(string_t title, xWidgetData option = {}, std::initializer_list<rWidgetData> lst = {}) {
			if (option.m_tag.empty())
				option.m_tag = GText("fieldset");
			option.m_properties.push_back({ GText("display"), GText("flex") });
			option.m_properties.push_back({ GText("flex-direction"), GText("column") });
			//option.m_properties.push_back({ GText("align-items"s), GText("center"s) });
			auto r = std::make_shared<xWidget>(option);
			r->m_components = lst;
			r->m_components.push_front(std::make_shared<xWidgetData>(xWidgetData{.m_tag=GText("legend"), .m_title=title}));
			return r;
		}

		[[nodiscard]] static rWidget PanelVertical(xWidgetData option={}, std::initializer_list<rWidgetData> lst = {}) {
			using namespace std::literals;

			if (option.m_tag.empty())
				option.m_tag = GText("div"s);
			option.m_properties.push_back({ GText("display"), GText("flex") });
			option.m_properties.push_back({GText("flex-direction"), GText("column")});
			//option.m_properties.push_back({ GText("align-items"s), GText("center"s) });
			auto r = std::make_shared<xWidget>(option);
			r->m_components = lst;
			return r;
		}
		[[nodiscard]] static rWidget PanelHorizontal(xWidgetData option={}, std::initializer_list<rWidgetData> lst = {}) {
			if (option.m_tag.empty())
				option.m_tag = GText("div"s);
			option.m_properties.push_back({GText("display"), GText("flex")});
			option.m_properties.push_back({GText("flex-direction"), GText("row")});
			option.m_properties.push_back({ GText("align-items"), GText("center") });
			auto r = std::make_shared<xWidget>(option);
			r->m_components = lst;
			return r;
		}

		[[nodiscard]] static rWidget Label(string_t title, gtl::ui::unit::position_t width={}, gtl::ui::unit::position_t height={}, xWidgetData option={}) {
			if (option.m_tag.empty())
				option.m_tag = GText("label"s);
			option.m_title = title;
			if (width.index())
				option.m_properties.push_back({GText("Width"), ToString(width)});
			if (height.index())
				option.m_properties.push_back({GText("Height"), ToString(height)});
			return std::make_shared<xWidget>(option);
		}
		[[nodiscard]] static rWidget Input(string_t text, gtl::ui::unit::position_t width={}, gtl::ui::unit::position_t height={}, xWidgetData option={}) {
			if (option.m_tag.empty())
				option.m_tag = GText("input"s);
			if (option.m_value.empty())
				option.m_value = text;
			if (width.index())
				option.m_properties.push_back({GText("Width"), ToString(width)});
			if (height.index())
				option.m_properties.push_back({GText("Height"), ToString(height)});
			if (!option.fn_OnTextChanged) {
				option.fn_OnTextChanged = [](rWidgetData rSelf, string_view_t evt, string_view_t text) -> bool {
					rSelf->m_value = text;
					return true;
				};
			}
			option.m_attributes.push_back(GText(R"xx(onchange="OnEditChange({id}, this.value)" )xx"));
			return std::make_shared<xWidget>(option);
		}
		[[nodiscard]] static rWidget Button(string_t title, gtl::ui::unit::position_t width={}, gtl::ui::unit::position_t height={}, xWidgetData option={}) {
			if (option.m_tag.empty())
				option.m_tag = GText("button"s);
			if (option.m_title.empty())
				option.m_title = title;
			option.m_attributes.emplace_back(GText(R"xx(onclick="OnButton({id})" )xx"));

			if (width.index())
				option.m_properties.push_back(gtl::ui::prop::MakeProperty(GText("Width"), width));
			if (height.index())
				option.m_properties.push_back({GText("Height"), ToString(height)});
			return std::make_shared<xWidget>(option);
		}
		[[nodiscard]] static rWidget Option(xWidgetData option = {}) {
			if (option.m_tag.empty())
				option.m_tag = GText("option"s);
			return std::make_shared<xWidget>(option);
		}
		[[nodiscard]] static rWidget Select(string_t title, std::vector<string_t> values, int size = 0, gtl::ui::unit::position_t width={}, gtl::ui::unit::position_t height={}, xWidgetData option={}) {
			if (option.m_tag.empty())
				option.m_tag = GText("select"s);
			if (option.m_title.empty())
				option.m_title = title;
			option.m_attributes.emplace_back(GText(R"xx(onclick="OnSelectChange({id})" )xx"));
			if (size)
				option.m_attributes.emplace_back(fmt::format(GText(R"xx(size={})xx"), size));
			option.m_template = Option();
			option.m_items = std::move(values);

			if (width.index())
				option.m_properties.push_back({ GText("Width"), ToString(width) });
			if (height.index())
				option.m_properties.push_back({ GText("Height"), ToString(height) });
			return std::make_shared<xWidget>(option);
		}

		[[nodiscard]] static rWidget Table(string_t name, gtl::ui::unit::position_t width={}, gtl::ui::unit::position_t height = {}) {
		}
		[[nodiscard]] static rWidget ListColumn(string_t name, rWidget itemTempl, gtl::ui::unit::position_t width={}, gtl::ui::unit::position_t height = {}) {
		}
		[[nodiscard]] static rWidget List(std::vector<xWidget> set, bool bColumnItems = true) {
		}
		[[nodiscard]] static rWidget TreeItem(std::vector<xWidget> set, bool bColumnItems = true) {
		}
		[[nodiscard]] static rWidget Tree(std::vector<xWidget> set, bool bColumnItems = true) {
		}
	};
	static_assert(std::is_aggregate_v<xWidget>);
	//using rWidget = std::shared_ptr<xWidget>;

};

