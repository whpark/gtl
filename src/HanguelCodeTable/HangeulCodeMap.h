#pragma once
//=========
//Automatically Generated File.
//
//        PWH.
//
//=========


#include "gtl/_config.h"

#if (GTL_STRING_SUPPORT_CODEPAGE_KSSM) && !(GTL_STRING_SUPPORT_CODEPAGE_KSSM_LIBICONV)

#include <cstdint>
#include <map>


#include "gtl/_lib_gtl.h"

namespace gtl::charset::KSSM {
#pragma pack(push, 8)


	GTL_DATA extern std::map<char16_t, uint16_t> const mapUTF16toKSSM_g;
	GTL_DATA extern std::map<uint16_t, char16_t> const mapKSSMtoUTF16_g;

#pragma pack(pop)
}	// namespace gtl::charset::KSSM


#endif // GTL_STRING_SUPPORT_CODEPAGE_KSSM && !(GTL_STRING_SUPPORT_CODEPAGE_KSSM_LIBICONV)
