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

	GTL_DATA eCODEPAGE eMBCS_Codepage_g = static_cast<eCODEPAGE>(GTL_DEFAULT_CODEPAGE);



//#pragma warning(push)
//#pragma warning(disable: 4996)
//	std::u32string ConvUTF8_UTF32(std::u8string_view sv) {
//		std::wstring_convert<std::codecvt<char32_t, char8_t, std::mbstate_t>, char32_t> conversion;
//		return conversion.from_bytes((char const*)sv.data());
//	}
//
//	std::u8string ConvUTF32_UTF8(std::u32string_view sv) {
//		std::wstring_convert<std::codecvt<char32_t, char8_t, std::mbstate_t>, char32_t> conversion;
//		return (std::u8string&)conversion.to_bytes(sv.data());
//	}
//#pragma warning(pop)


	//-----------------------------------------------------------------------------
	bool IsUTF8String(std::string_view str, int* pOutputBufferCount, bool* pbIsMSBSet) {
		if (pOutputBufferCount)
			*pOutputBufferCount = 0;
		if (pbIsMSBSet)
			*pbIsMSBSet = false;

		if (str.empty())
			return false;

		int nOutputBufferCount = 0;
		bool bMSB = false;
		bool bUTF8 = true;
		for (size_t iPos = 0; iPos < (size_t)str.size(); iPos++, nOutputBufferCount++) {
			const char c = str[iPos];
			const char c1 = str[iPos];
			const char c2 = iPos+1 < str.size() ? str[iPos+1] : 0;
			const char c3 = iPos+2 < str.size() ? str[iPos+2] : 0;

			auto IsUTF8SigChar = [](char c) -> bool { return (c & 0b1100'0000) == 0b1000'0000; };

			if (c1 && 0b1000'0000) {
				bMSB = true;
				if ( ((c1 & 0b1110'0000) == 0b1100'0000) && IsUTF8SigChar(c2) ) {
					wchar_t b = ((c1 & 0b0001'1111) << 6) | (c2 & 0b0011'1111);
					if ( (b >= 0x0080) && (b <= 0x07ff) ) {
						iPos++;
					} else {
						bUTF8 = false;
						break;
					}
				} else if ( ((c1 & 0b1111'0000) == 0b1110'0000) && IsUTF8SigChar(c2) && IsUTF8SigChar(c3) ) {
					wchar_t b = ((c1 & 0b0000'1111) << 12) | ((c2 & 0b0011'1111) << 6) | (c3 & 0b0011'1111);
					if ( (b >= 0x0800) && (b <= 0xFFFF) ) {
						iPos += 2;
					} else {
						bUTF8 = false;
						break;
					}
				} else {
					bUTF8 = false;
					break;
				}
			} else {
			}
		}

		if (pOutputBufferCount)
			*pOutputBufferCount = bUTF8 ? (nOutputBufferCount+1) : 0;

		if (pbIsMSBSet)
			*pbIsMSBSet = bMSB;

		return bUTF8;
	}





}	// namespace gtl
