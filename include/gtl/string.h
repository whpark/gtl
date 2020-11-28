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
	class TString : public std::basic_string<tchar, ttraits, tallocator> {
	public:
		using base_t = std::basic_string<tchar, ttraits, tallocator>;
		using value_type = typename base_t::value_type;
		using size_type = typename base_t::size_type;
		using index_type = intptr_t;

		/// @brief Constructor
		/// @tparam tchar 
		using base_t::base_t;
		TString() = default;
		TString(TString const& B) = default;
		TString(TString&& B) = default;
		TString(std::basic_string<tchar> const& B) : std::basic_string<tchar>(B) {}


		/// @brief Constructor from other codepage (string)
		/// @param strOther : string
		template < gtlc::string_elem tchar_other >
		explicit TString(TString<tchar_other> const& strOther) : base_t(ToString<tchar_other, tchar>(strOther)) {
		}


		/// @brief Constructor from other codepage (psz)
		/// @param pszOther 
		template < gtlc::string_elem tchar_other >
		GTL_DEPR_SEC explicit TString(tchar_other const*& pszOther) {
#pragma warning(suppress:4996)
			*this = pszOther;
		}

		template < gtlc::string_buffer_fixed tstring_buf_other >
		requires (!std::is_same_v< std::remove_cvref_t<decltype(tstring_buf_other{}[0])>, std::remove_cvref_t<tchar>>)
		explicit TString(tstring_buf_other const& buf) {
			*this = buf;
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
			this->resize(nMinLen, 0);
		}

	public:

		// returns psz
		//operator tchar const*() const					{ return this->c_str(); }

		// returns string_view
		operator std::basic_string_view<tchar>() const {
			return { this->data(), this->data() + this->size() };
		}

		// operator []
		using base_t::operator[];

		//---------------------------------------------------------------------
		// oeprator =
		using base_t::operator = ;
		TString& operator = (TString const&) = default;
		TString& operator = (TString&&) = default;

		// operator = ( char??_t type conversion)
		template < gtlc::string_elem tchar_other >
		requires (!std::is_same_v<std::remove_cvref_t<tchar_other>, std::remove_cvref_t<tchar>>)
		TString& operator = (std::basic_string<tchar_other> const& str) {
			*this = ToString<tchar_other, tchar>(str);
			return *this;
		}
		template < gtlc::string_elem tchar_other >
		requires (!std::is_same_v<std::remove_cvref_t<tchar_other>, std::remove_cvref_t<tchar>>)
		GTL_DEPR_SEC TString& operator = (tchar_other const*& psz) {
			*this = ToString<tchar_other, tchar>(psz);
			return *this;
		}
		template < gtlc::string_elem tchar_other >
		TString& operator = (std::basic_string_view<tchar_other> sv) {
			*this = ToString<tchar_other, tchar>(sv);
			return *this;
		}
		template < gtlc::string_buffer_fixed tstring_buf_other >
		requires (!std::is_same_v< std::remove_cvref_t<decltype(tstring_buf_other{}[0]) >, std::remove_cvref_t<tchar>>)
		TString& operator = (tstring_buf_other const& buf) {
			std::basic_string_view sv{std::data(buf), tszlen(buf)};
			*this = ToString<std::remove_cvref_t<decltype(tstring_buf_other{}[0])>, tchar>(sv);
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
		TString& operator += (const std::basic_string<tchar_other>& B) {
			if constexpr (std::is_same_v<tchar, tchar_other>) {
				base_t::operator += (B);
			} else {
				operator += ( ((const TString<tchar_other>&)B).ToString<tchar>() );
			}
			return *this;
		}
		template < gtlc::string_elem tchar_other >
		TString& operator += (const tchar_other* psz) {
			operator += (std::basic_string<tchar_other>(psz));
			return *this;
		}
		template < gtlc::string_elem tchar_other >
		TString& operator += (tchar_other ch) {
			operator += (std::basic_string<tchar_other>(1, ch));
			return *this;
		}

		//---------------------------------------------------------------------
		// operator +
		// operand 둘 중 하나는 TString 이라야 함.
		friend TString operator + (const TString& A, const TString<char>& B)				{ return TString(A) += B; }
		friend TString operator + (const TString& A, const std::basic_string<char>& B)		{ return TString(A) += B; }
		friend TString operator + (const TString& A, const char B)							{ return TString(A) += B; }
		friend TString operator + (const TString& A, const char* B)							{ return TString(A) += B; }
		friend TString operator + (const TString& A, const TString<wchar_t>& B)				{ return TString(A) += B; }
		friend TString operator + (const TString& A, const std::basic_string<wchar_t>& B)	{ return TString(A) += B; }
		friend TString operator + (const TString& A, const wchar_t B)						{ return TString(A) += B; }
		friend TString operator + (const TString& A, const wchar_t* B)						{ return TString(A) += B; }
		friend TString operator + (const std::basic_string<char>& A,	const TString& B)	{ return TString(A) += B; }
		friend TString operator + (const char A,						const TString& B)	{ return TString(A) += B; }
		friend TString operator + (const char* A,						const TString& B)	{ return TString(A) += B; }
		friend TString operator + (const std::basic_string<wchar_t>& A,	const TString& B)	{ return TString(A) += B; }
		friend TString operator + (const wchar_t A,						const TString& B)	{ return TString(A) += B; }
		friend TString operator + (const wchar_t* A,					const TString& B)	{ return TString(A) += B; }

		//---------------------------------------------------------------------
		// Compare
		inline int Compare(const base_t& strB)						const { return tszcmp<tchar>((std::basic_string_view<tchar>)*this, (std::basic_string_view<tchar>)strB); }
		inline int CompareNoCase(const base_t& strB)				const { return tszicmp<tchar>((std::basic_string_view<tchar>)*this, (std::basic_string_view<tchar>)strB); }
		inline int Compare(std::basic_string_view<tchar> sv)		const { return tszcmp<tchar>((std::basic_string_view<tchar>)*this, sv); }
		inline int CompareNoCase(std::basic_string_view<tchar> sv)	const { return tszicmp<tchar>((std::basic_string_view<tchar>)*this, sv); }

		template < typename tchar_other >
		inline int Compare(std::basic_string_view<tchar_other> sv)	const { return tszcmp<tchar>((std::basic_string_view<tchar>)*this, TString<tchar>(sv)); }
		template < typename tchar_other >
		int CompareNoCase(std::basic_string_view<tchar_other> sv)	const { return tszicmp<tchar>((std::basic_string_view<tchar>)*this, TString<tchar>(sv)); }

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
			return this->substr(iBegin, this->size() - iBegin);
		}
		TString Mid(index_type iBegin, index_type nCount) const {
			return this->substr(iBegin, nCount);
		}
		TString Left(index_type nCount) const {
			return this->substr(0, nCount);
		}
		TString Right(index_type nCount) const {
			index_type iBegin = ((index_type)this->size() < nCount) ? 0 : (index_type)this->size()-nCount;
			return this->substr(iBegin, nCount);
		}

		[[nodiscard]] std::basic_string_view<tchar> MidView(index_type iBegin) const {
			return std::basic_string_view<tchar>{this}.substr(iBegin, this->size() - iBegin);
		}
		[[nodiscard]] std::basic_string_view<tchar> MidView(index_type iBegin, index_type nCount) const {
			return std::basic_string_view<tchar>{this}.substr(iBegin, nCount);
		}
		[[nodiscard]] std::basic_string_view<tchar> LeftView(index_type nCount) const {
			return std::basic_string_view<tchar>{this}.substr(0, nCount);
		}
		[[nodiscard]] std::basic_string_view<tchar> RightView(index_type nCount) const {
			index_type iBegin = ((index_type)this->size() < nCount) ? 0 : (index_type)this->size()-nCount;
			return std::basic_string_view<tchar>{this}.substr(iBegin, nCount);
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
			std::string st;
			return str;
		}

		void MakeUpper()												{ std::transform(this->begin(), this->end(), std::toupper); /*for (auto& ch : *this) ch = toupper(ch);*/ }
		void MakeLower()												{ std::transform(this->begin(), this->end(), std::tolower); /*for (auto& ch : *this) ch = tolower(ch);*/ }
		void MakeReverse()												{ std::reverse(this->begin(), this->end()); }

		[[nodiscard]] std::basic_string<tchar> GetUpper() const {
			std::basic_string<tchar> str(this->size(), 0);
			std::transform(this->begin(), this->end(), str.begin(), std::toupper);
			return str;
		}
		[[nodiscard]] std::basic_string<tchar> GetLower() const {
			std::basic_string<tchar> str(this->size(), 0);
			std::transform(this->begin(), this->end(), str.begin(), std::tolower);
			return str;
		}

		void Trim()														{ gtl::Trim(); }
		void TrimRight()												{ gtl::TrimRight(*this); }
		void TrimRight(tchar chTarget)									{ gtl::TrimRight(*this, chTarget); }
		void TrimRight(const tchar* pszTargets)							{ gtl::TrimRight(*this, pszTargets); }
		void TrimLeft()													{ gtl::TrimLeft(*this); }
		void TrimLeft(tchar chTarget)									{ gtl::TrimLeft(*this, chTarget); }
		void TrimLeft(const tchar* pszTargets)							{ gtl::TrimLeft(*this, pszTargets); }

		[[nodiscard]] std::basic_string_view<tchar> TrimView()								const { return gtl::TrimView((std::basic_string_view<tchar>)*this); }
		[[nodiscard]] std::basic_string_view<tchar> TrimRightView()							const { return gtl::TrimRightView((std::basic_string_view<tchar>)*this); }
		[[nodiscard]] std::basic_string_view<tchar> TrimRightView(tchar chTarget)			const { return gtl::TrimRightView((std::basic_string_view<tchar>)*this, chTarget); }
		[[nodiscard]] std::basic_string_view<tchar> TrimRightView(const tchar* pszTargets)	const { return gtl::TrimRightView((std::basic_string_view<tchar>)*this, pszTargets); }
		[[nodiscard]] std::basic_string_view<tchar> TrimLeftView()							const { return gtl::TrimLeftView((std::basic_string_view<tchar>)*this); }
		[[nodiscard]] std::basic_string_view<tchar> TrimLeftView(tchar chTarget)			const { return gtl::TrimLeftView((std::basic_string_view<tchar>)*this, chTarget); }
		[[nodiscard]] std::basic_string_view<tchar> TrimLeftView(const tchar* pszTargets)	const { return gtl::TrimLeftView((std::basic_string_view<tchar>)*this, pszTargets); }

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
			//std::replace(this->begin(), this->end(), chOld, chNew);
		}
		// no recursive Replace.
		index_type Replace(const tchar* pszOld, const tchar* pszNew) {
			if (!pszOld || !tszlen(pszOld))
				return 0;
			index_type nLenOld = tszlen(pszOld);
			index_type nLenNew = pszNew ? tszlen(pszNew) : 0;
			index_type nToReplace = 0;

			const tchar* pszStart = this->data();
			const tchar* pszEnd = pszStart + this->size();
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
			size_type nLen = this->size() + (nLenNew - nLenOld) * nToReplace;
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
				this->clear();
			return nToReplace;
		}
		index_type Remove(tchar chRemove) {
			auto nNewLen = tszrmchar(this->data(), this->size(), chRemove);
			if (nNewLen != this->size())
				this->resize(nNewLen);
			return this->size();
		}
		index_type Insert(index_type index, tchar ch) {
			return this->insert(index, ch).size();
		}
		index_type Insert(index_type index, const tchar* psz) {
			return this->insert(index, psz).size();
		}
		index_type Delete(index_type index, index_type nCount = 1) {
			return this->erase(index, nCount).size();
		}

		//[[nodiscard]] index_type Find(tchar ch, index_type nStart = 0) const {
		//	return this->find(ch, nStart);
		//}
		//[[nodiscard]] index_type Find(const tchar* psz, index_type nStart = 0) const {
		//	return this->find(psz, nStart);
		//}
		//[[nodiscard]] index_type ReverseFind(tchar ch, index_type nStart = base_t::npos) const {
		//	return this->rfind(ch, nStart);
		//}
		//[[nodiscard]] index_type ReverseFind(const tchar* psz, index_type nStart = base_t::npos) const {
		//	return this->rfind(psz, nStart);
		//}
		//[[nodiscard]] index_type FindOneOf(const tchar* pszCharSet, index_type nStart = 0) const {
		//	const tchar* head = this->data();
		//	const tchar* pos = tszsearch_oneof(head+nStart, pszCharSet);
		//	if (pos)
		//		return (pos - head)+nStart;
		//	return this->npos;
		//}

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
			if (nNewLen > this->size()) {
				//if (bKeepOriginal && (nNewLen > this->capacity()))
				//	this->reserve(nNewLen);
				//else
				this->resize(nNewLen);
			}
			return this->data();
		}
		size_type ReleaseBuffer() {	// reserve(), data() 함수를 사용해서 버퍼를 가져온 다음, 버퍼에 데이터를 보낸 다음, 크기 다시 설정. capacity()에 따라서 마지막 문자가 잘릴 수 있음.
			if (!this->data() || !this->capacity())
				return 0;
			size_type nNewLen = tszlen(this->data(), this->size());
			this->resize(nNewLen, 0);
			return this->size();
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
