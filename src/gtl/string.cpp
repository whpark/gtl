#include "pch.h"

//////////////////////////////////////////////////////////////////////
//
// string.cpp
//
// PWH
// 2018.12.02.
// 2019.07.24. QL -> GTL
// 2020.11.12.
//
//////////////////////////////////////////////////////////////////////

#include "gtl/string/string_primitives.h"
#include "gtl/string/convert_codepage.h"
#include "gtl/string/convert_utf.h"
#include "gtl/string/convert_codepage_kssm.h"
#include "gtl/string/HangeulCodeMap.h"
#include "gtl/string/old_format.h"
#include "gtl/string.h"
//#include "gtl/archive.h"


namespace gtl {

	GTL__DATA eCODEPAGE eMBCS_Codepage_g = static_cast<eCODEPAGE>(GTL__DEFAULT_CODEPAGE);

//	//-----------------------------------------------------------------------------
//	bool IsUTF8String(std::string_view sv, size_t* pOutputBufferCount, bool* pbIsMSBSet) {
//#if 1
//		if (pOutputBufferCount)
//			*pOutputBufferCount = 0;
//		if (pbIsMSBSet)
//			*pbIsMSBSet = false;
//
//		size_t nOutputLen = 0;
//		auto const* pos = sv.data();
//		auto const* const end = sv.data() + sv.size();
//		while (pos < end) {
//			if (!gtl::internal::UTFCharConverter<char32_t, char8_t, false, true, false>((char8_t const*&)pos, (char8_t const*)end, nOutputLen))
//				return false;
//		}
//
//		if (pOutputBufferCount)
//			*pOutputBufferCount = nOutputLen;
//		if (pbIsMSBSet && (nOutputLen != sv.size()))
//			*pbIsMSBSet = true;
//
//		return true;
//
//#else
//		if (pOutputBufferCount)
//			*pOutputBufferCount = 0;
//		if (pbIsMSBSet)
//			*pbIsMSBSet = false;
//
//		if (sv.empty())
//			return false;
//
//		int nOutputBufferCount = 0;
//		bool bMSB = false;
//		bool bUTF8 = true;
//		for (size_t iPos = 0; iPos < (size_t)sv.size(); iPos++, nOutputBufferCount++) {
//			const char c = sv[iPos];
//			const char c1 = sv[iPos];
//			const char c2 = iPos+1 < sv.size() ? sv[iPos+1] : 0;
//			const char c3 = iPos+2 < sv.size() ? sv[iPos+2] : 0;
//
//			auto IsUTF8CharSub = [](char c) -> bool { return (c & 0b1100'0000) == 0b1000'0000; };
//
//			if (c1 && 0b1000'0000) {
//				bMSB = true;
//				if ( ((c1 & 0b1110'0000) == 0b1100'0000) && IsUTF8CharSub(c2) ) {
//					wchar_t b = ((c1 & 0b0001'1111) << 6) | (c2 & 0b0011'1111);
//					if ( (b >= 0x0080) && (b <= 0x07ff) ) {
//						iPos++;
//					} else {
//						bUTF8 = false;
//						break;
//					}
//				} else if ( ((c1 & 0b1111'0000) == 0b1110'0000) && IsUTF8CharSub(c2) && IsUTF8CharSub(c3) ) {
//					wchar_t b = ((c1 & 0b0000'1111) << 12) | ((c2 & 0b0011'1111) << 6) | (c3 & 0b0011'1111);
//					if ( (b >= 0x0800) && (b <= 0xFFFF) ) {
//						iPos += 2;
//					} else {
//						bUTF8 = false;
//						break;
//					}
//				} else {
//					bUTF8 = false;
//					break;
//				}
//			} else {
//			}
//		}
//
//		if (pOutputBufferCount)
//			*pOutputBufferCount = bUTF8 ? (nOutputBufferCount+1) : 0;
//
//		if (pbIsMSBSet)
//			*pbIsMSBSet = bMSB;
//
//		return bUTF8;
//#endif
//	}





}	// namespace gtl
