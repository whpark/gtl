#pragma once

#include "gtl/gtl.h"
#include "_lib_gtl_wx.h"

namespace gtl::wx {
#pragma pack(push, 8)

	template <typename tvalue_>
	std::remove_cvref_t<tvalue_> GetRegValue(wxRegKey& reg, std::string const& key, tvalue_&& valueDefault) {
		if (!reg.HasValue(key)) {
			return std::forward<tvalue_>(valueDefault);
		}

		//wxString str;
		//if (!reg.QueryValue(key, str))
		//	return std::forward<tvalue_>(valueDefault);

		using tvalue = std::remove_cvref_t<tvalue_>;
		if constexpr (std::is_integral_v<tvalue>) {
			long v{};
			if (!reg.QueryValue(key, &v))
				return valueDefault;
			return (tvalue)v;
		}
		else if constexpr (std::is_floating_point_v<tvalue>) {
			wxString str;
			if (!reg.QueryValue(key, str))
				return valueDefault;
			double v{};
			if (!str.ToDouble(&v))
				return valueDefault;
			return (tvalue)v;
		}
		else if constexpr (std::is_same_v<std::string, tvalue>) {
			wxString str;
			if (!reg.QueryValue(key, str))
				return valueDefault;
			return str.ToStdString();
		}
		else if constexpr (std::is_same_v<std::wstring, tvalue>) {
			wxString str;
			if (!reg.QueryValue(key, str))
				return valueDefault;
			return str.ToStdWstring();
		}
		else if constexpr (std::is_same_v<wxString, tvalue>) {
			wxString str;
			if (!reg.QueryValue(key, str))
				return valueDefault;
			return str;
		}
		else {
			static_assert(gtlc::dependent_false_v);
		}

		return valueDefault;
	};

	GTL__WX_API bool SaveWindowPosition(wxRegKey& regParent, std::string const& strWindowName, wxWindow* pWindow);
	GTL__WX_API bool LoadWindowPosition(wxRegKey& regParent, std::string const& strWindowName, wxWindow* pWindow);

#pragma pack(pop)
}
