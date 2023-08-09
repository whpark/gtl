/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EnvironmentVariable.h : Declaration of the xEnvironmentVariable
//
// PWH.
//
// 2023-08-08 from mocha
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "_lib_gtl_win_util.h"
#include "gtl/_default.h"
#include "winreg/WinReg.hpp"

namespace gtl::win_util {

	// Environment Setting
	class GTL__WINUTIL_CLASS xEnvironmentVariable {
	public:
		using this_t = xEnvironmentVariable;
		using key_t = std::wstring;
		using string_t = std::wstring;
		enum class eSCOPE {
			LOCAL_MACHINE,
			CURRENT_USER,
		};

	protected:
		//bool m_bBroadcasted;
		winreg::RegKey m_reg;
	public:
		xEnvironmentVariable(eSCOPE scope = eSCOPE::LOCAL_MACHINE);
		~xEnvironmentVariable();

		bool Open(eSCOPE scope = eSCOPE::LOCAL_MACHINE);

		explicit operator bool() const noexcept { return (bool)m_reg; }
		bool IsValid() const noexcept { return (bool)m_reg; }

		bool Broadcast(std::chrono::milliseconds timeout = std::chrono::milliseconds(3'000));

		// Operation
		std::optional<std::wstring> Get(key_t const& key);
		std::unordered_map<std::wstring, std::wstring> GetAll();
		bool Set(key_t const& key, string_t const& value);
		bool Delete(key_t const& svVarName);
	};

};	// namespace gtl::win_util
