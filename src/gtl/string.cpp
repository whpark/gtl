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

#include "gtl/string.h"
//#include "gtl/archive.h"
#include "HangeulCodeTable.h"



namespace gtl {
	//template < typename tchar_t >
	//typename TString<tchar_t>::size_type TString<tchar_t>::RecalcLength() {	// data() 함수를 사용해서 버퍼를 가져온 다음, 버퍼에 데이터를 보낸 다음, 크기 다시 설정. capacity()에 따라서 마지막 문자가 잘릴 수 있음.
	//	const tchar_t* p = *this;
	//	const auto nCapacity = this->capacity();
	//	if (!p || !nCapacity)
	//		return 0;
	//	auto& _My_data = this->_Get_data();
	//	size_type nNewLen = 0;
	//	for (; *p && nNewLen < nCapacity; p++)
	//		nNewLen++;
	//#ifdef _DEBUG
	//	if (nNewLen == nCapacity)
	//		throw std::overflow_error(__FMSG "String Length Overflowed.");
	//#endif
	//	return _My_data._Mysize = nNewLen;
	//}

	std::string ConvUnicode_MBCS(const std::wstring_view& strSource, int eCodepage) {
		size_t nLen = WideCharToMultiByte(eCodepage, 0, strSource.data()/*not c_str()*/, (int)strSource.size(), nullptr, 0, nullptr, nullptr);
		std::string str;
		if (nLen > 0) {
			str.resize(nLen);
			WideCharToMultiByte(eCodepage, 0, strSource.data()/*not c_str()*/, (int)strSource.size(), str.data(), (int)nLen, nullptr, nullptr);
		}
		return str;
	}
	std::wstring ConvMBCS_Unicode(const std::string_view& strSource, int eCodepage) {
		size_t nLen = MultiByteToWideChar(eCodepage, 0, strSource.data()/*not c_str()*/, (int)strSource.size(), nullptr, 0);
		std::wstring str;
		if (nLen > 0) {
			str.resize(nLen);
			MultiByteToWideChar(eCodepage, 0, strSource.data()/*not c_str()*/, (int)strSource.size(), str.data(), (int)nLen);
		}
		return str;
	}
	std::u8string ConvUnicode_UTF8(const std::wstring_view& strSource, int eCodepage /*dummy*/) {
		size_t nLen = WideCharToMultiByte(CP_UTF8, 0, strSource.data()/*not c_str()*/, (int)strSource.size(), nullptr, 0, nullptr, nullptr);
		std::u8string str;
		if (nLen > 0) {
			str.resize(nLen);
			WideCharToMultiByte(CP_UTF8, 0, strSource.data()/*not c_str()*/, (int)strSource.size(), (char*)str.data(), (int)nLen, nullptr, nullptr);
		}
		return str;
	}

	//template < class GetCode >	// 잘못되었음!. Unicode 에서만 동작하고, KSSM은 Binary Search 안됨!
	//S_HANGEUL_CODE const* SearchHangeulTable(unsigned short c, GetCode get) {
	//	auto const& tbl = GetHangeulCodeTable();

	//	int iLeft = 0;
	//	int iRight = (int)tbl.size();
	//	int i = 0;

	//	// binary search
	//	for (i = (iLeft + iRight)/2; get(tbl[i]) != c; i = (iLeft + iRight)/2) {
	//		if (get(tbl[i]) < c) iLeft = i; else iRight = i;
	//		if ( (iRight - iLeft) == 1 ) {
	//			if (get(tbl[iLeft]) == c)
	//				return &tbl[iLeft];
	//			if (get(tbl[iRight]) == c)
	//				return &tbl[iRight];
	//			return nullptr;
	//		}
	//		if (iLeft == iRight) return nullptr;
	//	}
	//	return (get(tbl[i]) == c) ? tbl + i : nullptr;
	//}


	constexpr unsigned long BIT7_0 = 0b1111'1111;//BIT(0)|BIT(1)|BIT(2)|BIT(3)|BIT(4)|BIT(5)|BIT(6)|BIT(7);
	constexpr unsigned long BIT7_1 = 0b1111'1110;//BIT(1)|BIT(2)|BIT(3)|BIT(4)|BIT(5)|BIT(6)|BIT(7);
	constexpr unsigned long BIT7_2 = 0b1111'1100;//BIT(2)|BIT(3)|BIT(4)|BIT(5)|BIT(6)|BIT(7);
	constexpr unsigned long BIT7_3 = 0b1111'1000;//BIT(3)|BIT(4)|BIT(5)|BIT(6)|BIT(7);
	constexpr unsigned long BIT7_4 = 0b1111'0000;//BIT(4)|BIT(5)|BIT(6)|BIT(7);
	constexpr unsigned long BIT7_5 = 0b1110'0000;//BIT(5)|BIT(6)|BIT(7);
	constexpr unsigned long BIT7_6 = 0b1100'0000;//BIT(6)|BIT(7);
	constexpr unsigned long BIT7_7 = 0b1000'0000;//BIT(7);

	constexpr unsigned long BIT0_7 = 0b1111'1111;//BIT(7)|BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0);
	constexpr unsigned long BIT0_6 = 0b0111'1111;//BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0);
	constexpr unsigned long BIT0_5 = 0b0011'1111;//BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0);
	constexpr unsigned long BIT0_4 = 0b0001'1111;//BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0);
	constexpr unsigned long BIT0_3 = 0b0000'1111;//BIT(3)|BIT(2)|BIT(1)|BIT(0);
	constexpr unsigned long BIT0_2 = 0b0000'0111;//BIT(2)|BIT(1)|BIT(0);
	constexpr unsigned long BIT0_1 = 0b0000'0011;//BIT(1)|BIT(0);
	constexpr unsigned long BIT0_0 = 0b0000'0001;//BIT(0);

	#define IS_UTF_C(x) ( ((x) & BIT7_6) == BIT7_7 )

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

			if (c1 && BIT7_7) {
				bMSB = true;
				if ( ((c1&BIT7_5) == BIT7_6) && IS_UTF_C(c2) ) {
					wchar_t b = ((c1 & BIT0_4) << 6) | (c2 & BIT0_5);
					if ( (b >= 0x0080) && (b <= 0x07ff) ) {
						iPos++;
					} else {
						bUTF8 = false;
						break;
					}
				} else if ( ((c1&BIT7_4) == BIT7_5) && IS_UTF_C(c2) && IS_UTF_C(c3) ) {
					wchar_t b = ((c1 & BIT0_3) << 12) | ((c2 & BIT0_5) << 6) | (c3 & BIT0_5);
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
