//=================================================================
// by 하얀악마
#include "pch.h"

#include "windows.h"

#include "gtl/win_util/EnvironmentVariable.h"

namespace gtl::win_util {

	xEnvironmentVariable::xEnvironmentVariable(eSCOPE scope) {
		Open(scope);
	}
	xEnvironmentVariable::~xEnvironmentVariable() {
	}

	bool xEnvironmentVariable::Open(eSCOPE scope) {
		try {
			if (scope == eSCOPE::LOCAL_MACHINE)
				m_reg.Open(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Control\\Session Manager\\Environment");
			else
				m_reg.Open(HKEY_CURRENT_USER, L"Environment");
		}
		catch (...) {
			return false;
		}
		return true;
	}

	bool xEnvironmentVariable::Broadcast(std::chrono::milliseconds timeout) {
		if (!m_reg)
			return {};
		DWORD_PTR result;
		LPCTSTR strBuf = L"Environment";
		SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)strBuf, SMTO_ABORTIFHUNG, timeout.count(), &result);
		return true;
	}

	std::optional<std::wstring> xEnvironmentVariable::Get(key_t const& key) {
		if (!m_reg)
			return {};
		if (auto r = m_reg.TryGetStringValue(key)) {
			return r.GetValue();
		}
		return {};
	}

	std::unordered_map<std::wstring, std::wstring> xEnvironmentVariable::GetAll() {
		std::unordered_map<std::wstring, std::wstring> r;
		if (!m_reg)
			return {};
		for (auto&& [key, type] : m_reg.EnumValues()) {
			if (type == REG_SZ)
				r[key] = m_reg.GetStringValue(key);
		}
		return r;
	}

	bool xEnvironmentVariable::Set(key_t const& key, std::wstring const& value) {
		if (!m_reg)
			return false;
		auto r = m_reg.TrySetStringValue(key, value);
		return !!r;
	}

	bool xEnvironmentVariable::Delete(key_t const& key) {
		if (!m_reg)
			return false;
		auto r = m_reg.TryDeleteKey(key, KEY_READ | KEY_WRITE | KEY_WOW64_64KEY);
		return !!r;
	}

}	// namespace gtl::qt
