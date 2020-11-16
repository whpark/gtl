#pragma once
//=========
//Automatically Generated File.
//
//        PWH.
//
//=========


#include "gtl/config_gtl.h"

#if (GTL_STRING_SUPPORT_CODEPAGE_KSSM)

#include <cstdint>
#include <map>


#include "gtl/_lib_gtl.h"

namespace gtl {
#pragma pack(push, 8)


	struct S_HANGEUL_CODE {
		wchar_t cUnicode;
		uint16_t cKSC;
		uint16_t c949;
		uint16_t cKSSM;
	};
	
	using T_HANGEUL_TABLE = std::array<S_HANGEUL_CODE, 11223>;

	GTL_DATA extern T_HANGEUL_TABLE const tblHangeulCode_g;
	GTL_DATA extern std::map<char16_t, uint16_t> const mapUTF16toKSSM_g;
	GTL_DATA extern std::map<uint16_t, char16_t> const mapKSSMtoUTF16_g;

#pragma pack(pop)
}	// namespace gtl


#endif // GTL_STRING_SUPPORT_CODEPAGE_KSSM
