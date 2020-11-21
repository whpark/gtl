//////////////////////////////////////////////////////////////////////
//
// string.h:
//
// PWH
// 2018.12.02.
// 2019.07.24. QL -> GTL, TQString -> TString
// 2020.11.12.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#ifndef GTL_HEADER__STRING
#define GTL_HEADER__STRING


#pragma warning(push)


#include <string>
#include <concepts>

#include "_lib_gtl.h"
#include "concepts.h"
#include "string/string_primitives.h"
#include "string/convert_codepage.h"
//#include "string/convert_codepage_kssm.h"
//#include "string/old_format.h"

namespace gtl {
#pragma pack(push, 8)

	//-----------------------------------------------------------------------------
	/// @brief pre-defines : class TString
	template < gtlc::string_elem tchar, class ttraits = std::char_traits<tchar>, class tallocator = std::allocator<tchar> >
	class TString;

	//------------------------------------------------------------------------------------------
	// TString : MFC::CString-like std::string wrapper
	//
	template < gtlc::string_elem tchar, class ttraits, class tallocator>
	class TString : public basic_string_t<tchar, ttraits, tallocator> {
	public:
		using base_t = basic_string_t<tchar, ttraits, tallocator>;
		using value_type = typename base_t::value_type;
		using size_type = typename base_t::size_type;
		using index_type = intptr_t;

		/// @brief Constructor
		/// @tparam tchar 
		using base_t::base_t;
		TString() = default;
		TString(TString const& B) = default;
		TString(TString&& B) = default;
		TString(basic_string_t<tchar> const& B) : basic_string_t<tchar>(B) {}


		/// @brief Constructor from other codepage (string)
		/// @param strOther : string
		template < gtlc::string_elem tchar_other > // requires (!std::is_same_v<tchar_other, tchar>)
		explicit TString(TString<tchar_other> const& strOther) : base_t(ToString<tchar_other, tchar>(strOther)) {
		}


		/// @brief Constructor from other codepage (psz)
		/// @param pszOther 
		template < gtlc::string_elem tchar_other > // requires (!std::is_same_v<tchar_other, tchar>)
		explicit TString(tchar_other const* pszOther) {
			*this = pszOther;
		}


		/// @brief Constructor from string_view<tchar_other>
		/// @tparam  
		/// @param B 
		//template < gtlc::string_elem tchar_other, template <typename> typename tstr_view >
		//	requires (std::is_convertible_v<tstr_view<tchar_other>, std::basic_string_view<tchar_other> >)
		//TString(tstr_view<tchar_other> B) : base_t(ToString<tchar_other, tchar>(B)) {
		//}
		template < gtlc::string_elem tchar_other >
		TString(std::basic_string_view<tchar_other> sv) : base_t(ToString<tchar_other, tchar>(sv)) {
		}

		//tchar* Attach(tchar* psz, size_type nBufferSize);
		//tchar* Detach();
		void Init(size_type nMinLen = 0) {
			base_t::resize(nMinLen, 0);
		}

	public:
		//// helper functions. do I need these?..
		//auto GetLength() const						{ return base_t::size(); }
		//auto IsEmpty() const						{ return base_t::empty(); }
		//void Clear()								{ base_t::clear(); }
		//[[deprecated]] void Empty()					{ base_t::clear(); }

		//tchar&		GetAt(int index)			{ return base_t::at(index); }
		//tchar const&	GetAt(int index) const		{ return base_t::at(index); }
		//void SetAt(int index, tchar ch)			{ base_t::at(index) = ch; }

		// returns psz
		operator tchar const*() const				{ return base_t::c_str(); }

		// operator []
		using base_t::operator[];

		// returns string_view
		operator std::basic_string_view<tchar>() const {
			return { this->data(), this->data() + this->size() };
		}

		//---------------------------------------------------------------------
		// oeprator =
		using base_t::operator = ;
		TString& operator = (TString const&) = default;
		TString& operator = (TString&&) = default;

		template < gtlc::string_elem tchar_other >
		TString& operator = (basic_string_t<tchar_other> const& str) {
			*this = ToString<tchar_other, tchar>(str);
			return *this;
		}
		template < gtlc::string_elem tchar_other >
		TString& operator = (tchar_other const* psz) {
			*this = ToString<tchar_other, tchar>(psz);
			return *this;
		}
		// From string_view
		template < gtlc::string_elem tchar_other >
		TString& operator = (std::basic_string_view<tchar_other> sv) {
			*this = ToString<tchar_other, tchar>(sv);
			return *this;
		}

		//---------------------------------------------------------------------
		// operator +=
		using base_t::operator +=;
		TString& operator += (const TString& B) {
			base_t::operator += (B);
			return *this;
		}
		template < gtlc::string_elem tchar_other >
		TString& operator += (const basic_string_t<tchar_other>& B) {
			if constexpr (std::is_same_v<tchar, tchar_other>) {
				base_t::operator += (B);
			} else {
				operator += ( ((const TString<tchar_other>&)B).ToString<tchar>() );
			}
			return *this;
		}
		template < gtlc::string_elem tchar_other >
		TString& operator += (const tchar_other* psz) {
			operator += (basic_string_t<tchar_other>(psz));
			return *this;
		}
		template < gtlc::string_elem tchar_other >
		TString& operator += (tchar_other ch) {
			operator += (basic_string_t<tchar_other>(1, ch));
			return *this;
		}

		//---------------------------------------------------------------------
		// operator +
		// operand 둘 중 하나는 TString 이라야 함.
		friend TString operator + (const TString& A, const TString<char>& B)				{ return TString(A) += B; }
		friend TString operator + (const TString& A, const basic_string_t<char>& B)			{ return TString(A) += B; }
		friend TString operator + (const TString& A, const char B)							{ return TString(A) += B; }
		friend TString operator + (const TString& A, const char* B)							{ return TString(A) += B; }
		friend TString operator + (const TString& A, const TString<wchar_t>& B)				{ return TString(A) += B; }
		friend TString operator + (const TString& A, const basic_string_t<wchar_t>& B)		{ return TString(A) += B; }
		friend TString operator + (const TString& A, const wchar_t B)						{ return TString(A) += B; }
		friend TString operator + (const TString& A, const wchar_t* B)						{ return TString(A) += B; }
		friend TString operator + (const basic_string_t<char>& A,		const TString& B)	{ return TString(A) += B; }
		friend TString operator + (const char A,						const TString& B)	{ return TString(A) += B; }
		friend TString operator + (const char* A,						const TString& B)	{ return TString(A) += B; }
		friend TString operator + (const basic_string_t<wchar_t>& A,	const TString& B)	{ return TString(A) += B; }
		friend TString operator + (const wchar_t A,						const TString& B)	{ return TString(A) += B; }
		friend TString operator + (const wchar_t* A,					const TString& B)	{ return TString(A) += B; }

		//---------------------------------------------------------------------
		// Compare
		int Compare(const base_t& strB)			const { return tszcmp(base_t::c_str(), strB.c_str()); }
		int CompareNoCase(const base_t& strB)	const { return tszicmp(base_t::c_str(), strB.c_str()); }
		int Compare(const tchar* psz)			const { return tszcmp(base_t::c_str(), psz); }
		int CompareNoCase(const tchar* psz)	const { return tszicmp(base_t::c_str(), psz); }

		template < typename tchar_other >
		int Compare(const tchar_other* psz)		const { return tszcmp(base_t::c_str(), TString<tchar>(psz)); }
		template < typename tchar_other >
		int CompareNoCase(const tchar_other* psz)	const { return tszicmp(base_t::c_str(), TString<tchar>(psz)); }

		//---------------------------------------------------------------------
		// operator ==, !=, <, >, <=, >=, 필요 없음. basic_string 으로 대체

		//---------------------------------------------------------------------
		// ToNumber
		template < gtlc::arithmetic T_NUMBER >
		T_NUMBER ToNumber(tchar const** pszEnd = nullptr, int radix = 0) {
			return tszto_number<tchar, T_NUMBER>(*this, pszEnd, radix);
		}

		//---------------------------------------------------------------------
		// SubString
		TString Mid(index_type iBegin) const {
			return TString(base_t::substr(iBegin, base_t::size()-iBegin));
		}
		TString Mid(index_type iBegin, index_type nCount) const {
			return TString(base_t::substr(iBegin, nCount));
		}
		TString Left(index_type nCount) const {
			return TString(base_t::substr(0, nCount));
		}
		TString Right(index_type nCount) const {
			index_type iBegin = ((index_type)base_t::size() < nCount) ? 0 : (index_type)base_t::size()-nCount;
			return TString(base_t::substr(iBegin, nCount));
		}

		TString SpanIncluding(const tchar* pszCharSet) const {
			TString str;
			index_type n {0};
			for (const tchar* sz = *this; sz && *sz; sz++) {
				if (tszsearch(pszCharSet, *sz))
					n++;
			}
			if (n) {
				str.reserve(n);
				for (const tchar* sz = *this; sz && *sz; sz++) {
					if (tszsearch(pszCharSet, *sz))
						str += *sz;
				}
			}
			return str;
		}
		TString SpanExcluding(const tchar* pszCharSet) const {
			TString str;
			index_type n {0};
			for (const tchar* sz = *this; sz && *sz; sz++) {
				if (!tszsearch(pszCharSet, (int)*sz))
					n++;
			}
			if (n) {
				str.reserve(n);
				for (const tchar* sz = *this; sz && *sz; sz++) {
					if (!tszsearch(pszCharSet, *sz))
						str += *sz;
				}
			}
			return str;
		}

		void MakeUpper()												{ std::transform(base_t::begin(), base_t::end(), std::toupper); /*for (auto& ch : *this) ch = toupper(ch);*/ }
		void MakeLower()												{ std::transform(base_t::begin(), base_t::end(), std::tolower); /*for (auto& ch : *this) ch = tolower(ch);*/ }
		void MakeReverse()												{ std::reverse(base_t::begin(), base_t::end()); }

		void Trim()														{ TrimLeft(); TrimRight(); }
		void TrimRight()												{ gtl::TrimRight(*this); }
		void TrimRight(tchar chTarget)								{ tchar szTrim[] = { chTarget, 0, }; gtl::TrimRight(*this, szTrim); }
		void TrimRight(const tchar* pszTargets)						{ gtl::TrimRight(*this, pszTargets); }
		void TrimLeft()													{ gtl::TrimLeft(*this); }
		void TrimLeft(tchar chTarget)									{ tchar szTrim[] = { chTarget, 0, }; gtl::TrimLeft(*this, szTrim); }
		void TrimLeft(const tchar* pszTargets)						{ gtl::TrimLeft(*this, pszTargets); }


		index_type Replace(tchar chOld, tchar chNew) {
			index_type nReplaced = 0;
			for (auto& ch : *this) {
				if (ch == chOld) {
					ch = chNew;
					nReplaced++;
				}
			}
			return nReplaced;
			// or
			//std::replace(base_t::begin(), base_t::end(), chOld, chNew);
		}
		// no recursive Replace.
		index_type Replace(const tchar* pszOld, const tchar* pszNew) {
			if (!pszOld || !tszlen(pszOld))
				return 0;
			index_type nLenOld = tszlen(pszOld);
			index_type nLenNew = pszNew ? tszlen(pszNew) : 0;
			index_type nToReplace = 0;

			const tchar* pszStart = base_t::c_str();
			const tchar* pszEnd = pszStart + base_t::size();
			const tchar* pszEndSearch = pszEnd-nLenOld+1;

			// 먼저 몇 개 바꿔야 되는지 찾고
			for (const tchar* psz = pszStart; psz < pszEndSearch; psz++) {
				if (tszncmp(psz, pszOld, nLenOld) == 0) {
					psz += nLenOld-1;
					nToReplace++;
				}
			}
			if (!nToReplace)
				return 0;

			index_type nLeft = nToReplace;
			size_type nLen = base_t::size() + (nLenNew - nLenOld) * nToReplace;
			if(nLen) {
				TString strReplaced;
				strReplaced.resize(nLen);
				tchar* pszReplaced = strReplaced.data();
				const tchar* psz = pszStart;
				for (; nLeft && psz < pszEndSearch; psz++) {
					if (tszncmp(psz, pszOld, nLenOld) == 0) {
						if (nLenNew) {
							memcpy(pszReplaced, pszNew, nLenNew*sizeof(tchar));	// funcion tszncpy() appends NULL. we don't need NULL to be copied into dest.
							pszReplaced += nLenNew;
						}
						psz += nLenOld-1;
						nLeft--;
					} else {
						*pszReplaced++ = *psz;
					}
				}
				// copy rest... (including NULL terminating char)
				memcpy(pszReplaced, psz, (pszEnd - psz + 1)*sizeof(tchar));
				*this = std::move(strReplaced);
			} else
				base_t::clear();
			return nToReplace;
		}
		index_type Remove(tchar chRemove) {
			index_type nOrg = base_t::size();
			std::array<tchar, 2> szRemove {chRemove, 0};
			*this = SpanExcluding(szRemove.data());
			index_type nNew = base_t::size();
			return nOrg-nNew;
			//tchar* pos1 = base_t::data();
			//tchar* pos2 = base_t::data();
			//index_type nReplaced = 0;
			//while (*pos2) {
			//	if (*pos2 == chRemove) {
			//		*pos2++;
			//		nReplaced++;
			//	} else {
			//		if (nReplaced)
			//			*pos1++ = *pos2++;
			//	}
			//}
			//return nReplaced;
		}
		index_type Insert(index_type index, tchar ch) {
			return base_t::insert(index, ch).size();
		}
		index_type Insert(index_type index, const tchar* psz) {
			return base_t::insert(index, psz).size();
		}
		index_type Delete(index_type index, index_type nCount = 1) {
			return base_t::erase(index, nCount).size();
		}

		[[nodiscard]] index_type Find(tchar ch, index_type nStart = 0) const {
			return base_t::find(ch, nStart);
		}
		[[nodiscard]] index_type Find(const tchar* psz, index_type nStart = 0) const {
			return base_t::find(psz, nStart);
		}
		[[nodiscard]] index_type ReverseFind(tchar ch, index_type nStart = base_t::npos) const {
			return base_t::rfind(ch, nStart);
		}
		[[nodiscard]] index_type ReverseFind(const tchar* psz, index_type nStart = base_t::npos) const {
			return base_t::rfind(psz, nStart);
		}
		[[nodiscard]] index_type FindOneOf(const tchar* pszCharSet, index_type nStart = 0) const {
			const tchar* head = base_t::c_str();
			const tchar* pos = tszsearch_oneof(head+nStart, pszCharSet);
			if (pos)
				return (pos - head)+nStart;
			return base_t::npos;
		}

		//// simple formatting
		//template < typename ... Args >
		//TString& Format(tchar const* pszFormat, Args&& ... args) {
		//	return *this = gtl::old_printf::Format(pszFormat, std::forward<Args>(args)...);
		//}

		//// to do : to be tested
		//void FormatV(const tchar* pszFormat, va_list argList) {
		//	return *this = gtl::FormatV(pszFormat, argList);
		//}


	//	bool LoadString(UINT nID);

		[[nodiscard]] tchar* GetBuffer(size_type nNewLen) {
			if (nNewLen > base_t::size()) {
				//if (bKeepOriginal && (nNewLen > base_t::capacity()))
				//	base_t::reserve(nNewLen);
				//else
				base_t::resize(nNewLen);
			}
			return base_t::data();
		}
		size_type ReleaseBuffer() {	// reserve(), data() 함수를 사용해서 버퍼를 가져온 다음, 버퍼에 데이터를 보낸 다음, 크기 다시 설정. capacity()에 따라서 마지막 문자가 잘릴 수 있음.
			const tchar* p = *this;
			const auto nCapacity = base_t::capacity();
			if (!p || !nCapacity)
				return 0;
			//auto& _My_data = this->_Get_data();
			size_type nNewLen = 0;
			if constexpr (std::is_same_v<tchar, wchar_t> || std::is_same_v<tchar, char16_t>) {
				nNewLen = wcsnlen(p, nCapacity);
			} else if constexpr (std::is_same_v<tchar, char> || std::is_same_v<tchar, char8_t>) {
				nNewLen = strnlen(p, nCapacity);
			} else {
				for (const tchar* pos = p; *pos && (nNewLen < nCapacity); pos++, nNewLen++)
					;
			}

			//#ifdef _DEBUG
			//	if (nNewLen == nCapacity)
			//		throw std::overflow_error(GTL__FUNCSIG "String Length Overflowed.");
			//#endif
			base_t::resize(nNewLen, 0);
			return base_t::length();
		}

	};	// TString


	using CString = TString<char16_t>;
	using CStringA = TString<char>;				// MBCS
	using CStringW = TString<wchar_t>;			// Unicode (WideChar)
	using CStringU8 = TString<char8_t>;			// Unicode UTF-8
	using CStringU16 = TString<char16_t>;		// Unicode UTF-16
	using CStringU32 = TString<char32_t>;		// Unicode UTF-32
#if (GTL_STRING_SUPPORT_CODEPAGE_KSSM)
	using CStringKSSM = TString<uint16_t>;		// KSSM (codepage 1361)
#endif


#pragma pack(pop)
};	// namespace gtl;


#pragma warning(pop)



#endif	// GTL_HEADER__STRING
