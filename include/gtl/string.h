﻿//////////////////////////////////////////////////////////////////////
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

#ifndef GTL__HEADER__STRING
#define GTL__HEADER__STRING


#pragma warning(push)


#include <string>
#include <concepts>

#include "_lib_gtl.h"
#include "_config.h"
#include "concepts.h"
#include "string/string_primitives.h"
#include "string/convert_codepage.h"
#include "string/string_misc.h"
#include "string/string_to_arithmetic.h"

#include "string/string_primitives.hpp"
#include "string/string_misc.hpp"

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
		TString(std::basic_string<tchar>&& B) : std::basic_string<tchar>(std::move(B)) {}
		TString(std::basic_string_view<tchar> B) : std::basic_string<tchar>(B) {}


		/// @brief Constructor from other codepage (string)
		/// @param strOther : string
		template < gtlc::string_elem tchar_other >
		explicit TString(TString<tchar_other> const& strOther) : base_t(gtl::ToString<tchar, tchar_other>(strOther)) {
		}


		///// @brief Constructor from other codepage (sz[])
		///// @param pszOther 
		//template < size_t N, gtlc::string_elem tchar_other > requires (!std::is_same_v<tchar, tchar_other>)
		//explicit TString(tchar_other const (&szOther)[N]) {
		//#pragma warning(suppress:4996)
		//	*this = szOther;
		//}

		/// @brief Constructor from other codepage (psz)
		/// @param pszOther 
		template < gtlc::string_elem tchar_other > requires (!std::is_same_v<tchar, tchar_other>)
		GTL__DEPR_SEC explicit TString(tchar_other const* pszOther) {
#pragma warning(suppress:4996)
			*this = pszOther;
		}

		/// @brief Constructor from other codepage
		/// @param 
		template < gtlc::contiguous_string_container tstring_buf, gtlc::string_elem tchar_other = std::remove_cvref_t<decltype(tstring_buf{}[0])> >
		requires (!std::is_same_v< tchar, tchar_other >)
		explicit TString(tstring_buf const& b) : base_t(gtl::ToString<tchar, tchar_other>(b)) {
		}


	public:
		// returns psz
	//#if GTL__USE_WINDOWS_API
		operator tchar const*() const					{ return this->c_str(); }
	//#endif

		// returns string_view
		operator std::basic_string_view<tchar>() const {
			return { this->data(), this->data() + this->size() };
		}

		//operator std::wstring_view() const requires (gtlc::is_same_utf<tchar, wchar_t>) {
		//	return { (wchar_t const*)this->data(), (wchar_t const*)this->data() + this->size() };
		//}
		//operator std::u16string_view() const requires (gtlc::is_same_utf<tchar, char16_t>) {
		//	return { (char16_t const*)this->data(), (char16_t const*)this->data() + this->size() };
		//}
		//operator std::u32string_view() const requires (gtlc::is_same_utf<tchar, char32_t>) {
		//	return { (char32_t const*)this->data(), (char32_t const*)this->data() + this->size() };
		//}

		//operator std::wstring& () requires (gtlc::is_same_utf<tchar, wchar_t>) {
		//	return reinterpret_cast<std::wstring&>(*this);
		//}
		//operator std::u16string& () requires (gtlc::is_same_utf<tchar, char16_t>) {
		//	return reinterpret_cast<std::u16string&>(*this);
		//}
		//operator std::u32string& () requires (gtlc::is_same_utf<tchar, char32_t>) {
		//	return reinterpret_cast<std::u32string&>(*this);
		//}

		// operator []
		using base_t::operator[];

		//---------------------------------------------------------------------
		/// @brief oeprator =
		using base_t::operator = ;
		TString& operator = (TString const&) = default;
		TString& operator = (TString&&) = default;
		TString& operator = (std::basic_string<tchar> const& b) { base_t::operator = (b); return *this; }
		TString& operator = (std::basic_string<tchar>&& b) { base_t::operator = (std::move(b)); return *this; }

		/// @brief operator = ( char??_t type conversion)
		template < gtlc::string_elem tchar_other >
		requires (!std::is_same_v<std::remove_cvref_t<tchar_other>, std::remove_cvref_t<tchar>>)
		TString& operator = (std::basic_string<tchar_other> const& str) {
			*this = gtl::ToString<tchar, tchar_other>(str);
			return *this;
		}
		//template < size_t N, gtlc::string_elem tchar_other > requires (!std::is_same_v<tchar, tchar_other>)
		//TString& operator = (tchar_other const (&szOther)[N]) {
		//#pragma warning(suppress:4996)
		//	*this = gtl::ToString<tchar, tchar_other>(std::basic_string_view<tchar_other>(szOther, tszlen(szOther, N)));
		//}
		template < gtlc::string_elem tchar_other >
		requires (!std::is_same_v<std::remove_cvref_t<tchar_other>, std::remove_cvref_t<tchar>>)
		GTL__DEPR_SEC TString& operator = (tchar_other const* const& psz) {
			*this = gtl::ToString<tchar, tchar_other>(psz);
			return *this;
		}
		template < gtlc::string_elem tchar_other >
		TString& operator = (std::basic_string_view<tchar_other> sv) {
			*this = gtl::ToString<tchar, tchar_other>(sv);
			return *this;
		}
		template < gtlc::contiguous_string_container tstring_buf, gtlc::string_elem tchar_other = std::remove_cvref_t<decltype(tstring_buf{}[0])> >
		requires (!std::is_same_v< tchar, tchar_other> )
		TString& operator = (tstring_buf const& buf) {
			std::basic_string_view sv{std::data(buf), tszlen(buf)};
			*this = gtl::ToString<tchar, tchar_other>(sv);
			return *this;
		}


		//---------------------------------------------------------------------
		/// @brief Convert Codepage
		template < typename tchar_other >
		std::basic_string<tchar_other> to_string(S_CODEPAGE_OPTION codepage = {}) const {
			return gtl::ToString<tchar_other, tchar>(*this, codepage);
		}
		template < typename tchar_other, bool bCOUNT_FIRST = true >
		TString<tchar_other> ToString(S_CODEPAGE_OPTION codepage = {}) const {
			return (TString<tchar_other>&)gtl::ToString<tchar_other, tchar>(*this, codepage);
		}

		//---------------------------------------------------------------------
		/// @brief operator += ...
		using base_t::operator +=;
		TString& operator += (TString const& b) {
			((std::basic_string<tchar>&)*this) += b;
			return *this;
		}
		template < gtlc::contiguous_string_container tstring_buf, gtlc::string_elem tchar_other = std::remove_cvref_t<decltype(tstring_buf{}[0])> >
		//template < gtlc::tchar_string_like tstring_buf, gtlc::string_elem tchar_other = std::remove_cvref_t<decltype(tstring_buf{}[0])> >
			requires (!std::is_same_v<tchar, tchar_other>)
		inline TString& operator += (tstring_buf const& b) {
			((std::basic_string<tchar>&)*this) += gtl::ToString<tchar, tchar_other>(/*std::basic_string_view<tchar_other>(*/b);
			return *this;
		}
		template < gtlc::string_elem tchar_other >// requires (!std::is_same_v<tchar, tchar_other>)
		GTL__DEPR_SEC inline TString& operator += (tchar_other const* const& psz) {
			operator += (std::basic_string_view<tchar_other>(psz));
			return *this;
		}
		template < gtlc::string_elem tchar_other >// requires (!std::is_same_v<tchar, tchar_other>)
		inline TString& operator += (tchar_other const c) {
			operator += (std::basic_string_view<tchar_other>(&c, &c+1));
			return *this;
		}

		//---------------------------------------------------------------------
		/// @brief operator + ... (which needs codepage conversion)
		template < gtlc::string_elem tchar_other > requires (!std::is_same_v<tchar, tchar_other>)
		friend inline [[nodiscard]] TString operator + (TString const& a, TString<tchar_other> const& b) {
			return (std::basic_string<tchar> const&)a + gtl::ToString<tchar, tchar_other>(b);
		}
		template < gtlc::contiguous_string_container tstring_buf, gtlc::string_elem tchar_other = std::remove_cvref_t<decltype(tstring_buf{}[0])> >
		requires (!std::is_same_v<tchar, tchar_other>)
		friend inline [[nodiscard]] TString operator + (TString const& a, tstring_buf const& b) {
			return (std::basic_string<tchar> const&)a + gtl::ToString<tchar, tchar_other>(b);
		}
		template < gtlc::contiguous_string_container tstring_buf, gtlc::string_elem tchar_other = std::remove_cvref_t<decltype(tstring_buf{}[0])> >
		requires (!std::is_same_v<tchar, tchar_other>)
		friend inline [[nodiscard]] TString&& operator + (TString&& a, tstring_buf const& b) {
			a += gtl::ToString<tchar, tchar_other>(b);
			return std::move(a);
		}

		///// @brief operator + (move) ...
		//friend inline [[nodiscard]] TString&& operator + (TString&& a, TString const& b) {
		//	a += b;
		//	return std::move(a);
		//}
		//template < gtlc::string_elem tchar_other >
		//friend inline [[nodiscard]] TString&& operator + (TString&& a, TString<tchar_other> const& b) {
		//	a += b.ToString<tchar>();
		//	return std::move(a);
		//}
		//template < gtlc::string_elem tchar_other >
		//friend [[nodiscard]] TString&& operator + (TString&& a, std::basic_string<tchar_other> const& b) {
		//	if constexpr (std::is_same_v<tchar, tchar_other>) {
		//		a += b;
		//	} else {
		//		a += gtl::ToString<tchar, tchar_other>(b);
		//	}
		//	return std::move(a);
		//}
		//template < gtlc::string_elem tchar_other >
		//friend inline [[nodiscard]] TString&& operator + (TString&& a, std::basic_string_view<tchar_other> b)	{
		//	if constexpr (std::is_same_v<tchar, tchar_other>) {
		//		a += b;
		//	} else {
		//		a += gtl::ToString<tchar, tchar_other>(b);
		//	}
		//	return std::move(a);
		//}
		//template < gtlc::string_elem tchar_other >
		//friend inline [[nodiscard]] TString&& operator + (TString&& a, tchar_other const b) {
		//	a += std::basic_string_view<tchar_other>(b);
		//	return std::move(a);
		//}
		//template < gtlc::string_elem tchar_other >
		//GTL__DEPR_SEC friend inline [[nodiscard]] TString&& operator + (TString&& a, tchar_other const* const& b) {
		//	a += std::basic_string_view<tchar_other>(b);
		//	return std::move(a);
		//}

	protected:
		template < gtlc::string_elem tchar1, gtlc::string_elem tchar2, typename tcontainer >
		requires ( (std::is_same_v<tchar1, std::remove_cvref_t<decltype(tcontainer{}[0])>>) )
		static TString<tchar1> Add(tcontainer const& a, TString<tchar2> const& b) {
			if constexpr (std::is_same_v<tchar1, tchar2>) {
				std::basic_string<tchar1> str;
				static auto m = str.max_size();
				auto const size_a = std::size(a);
				if ( (size_a >= m) or (b.size() >= m-size_a) )
					throw std::invalid_argument(GTL__FUNCSIG "too long a string.");
				auto n = size_a + b.size();
				str.reserve(n);
				str.append(a);
				str.append(b);
				return str;
			}
			else {
				return Add<tchar1, tchar1, tcontainer>(a, gtl::ToString<tchar1, tchar2>(b));
			}
		}

	public:
		//---------------------------------------------------------------------
		/// @brief operator ... +
		// TString<tchar_other> 로 하면 동작 안함. friend 함수의 인자에 하나라도 class와 동일한 인자가 있어야 하나?....
		template < gtlc::contiguous_string_container tstring_buf, gtlc::string_elem tchar_other = std::remove_cvref_t<decltype(tstring_buf{}[0])> >
		requires (!std::is_base_of_v<TString<tchar_other>, tstring_buf>)
		friend inline [[nodiscard]] TString<tchar_other> operator + (tstring_buf const& a, TString const& b) {
			return TString::Add<tchar_other, tchar>(a, b);
		}
		//template < typename tchar_other >
		//requires (!std::is_same_v<tchar, tchar_other>)
		//friend inline [[nodiscard]] TString<tchar> operator + (TString<tchar_other> const& a, TString const& b) {
		//	return TString::Add<tchar_other, tchar>(a, b);
		//}
		template < gtlc::string_elem tchar_other >// requires (!std::is_same_v<tchar, tchar_other>)
		GTL__DEPR_SEC friend inline [[nodiscard]] TString<tchar_other> operator + (tchar_other const* const& a, TString<tchar> const& b) {
			return TString::Add<tchar_other, tchar>(std::basic_string_view<tchar_other>(a), b);
		}
		template < gtlc::string_elem tchar_other >// requires (!std::is_same_v<tchar, tchar_other>)
		friend inline [[nodiscard]] TString<tchar_other> operator + (tchar_other const a, TString const& b) {
			return TString::Add<tchar_other, tchar>(std::basic_string_view<tchar_other>{&a, &a+1}, b);
		}

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
		template < typename tchar_other >
		inline int Compare(std::basic_string<tchar_other> const& str)	const { return tszcmp<tchar>((std::basic_string_view<tchar>)*this, TString<tchar>(str)); }
		template < typename tchar_other >
		int CompareNoCase(std::basic_string<tchar_other> const& str)	const { return tszicmp<tchar>((std::basic_string_view<tchar>)*this, TString<tchar>(str)); }

		//---------------------------------------------------------------------
		// operator ==, !=, <, >, <=, >=, 필요 없음. basic_string 으로 대체

		//---------------------------------------------------------------------
		// ToNumber
		template < gtlc::arithmetic T_NUMBER >
		T_NUMBER ToNumber(tchar const** pszEnd = nullptr, int radix = 0) const {
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
			return std::basic_string_view<tchar>{*this}.substr(iBegin, this->size() - iBegin);
		}
		[[nodiscard]] std::basic_string_view<tchar> MidView(index_type iBegin, index_type nCount) const {
			return std::basic_string_view<tchar>{*this}.substr(iBegin, nCount);
		}
		[[nodiscard]] std::basic_string_view<tchar> LeftView(index_type nCount) const {
			return std::basic_string_view<tchar>{*this}.substr(0, nCount);
		}
		[[nodiscard]] std::basic_string_view<tchar> RightView(index_type nCount) const {
			index_type iBegin = ((index_type)this->size() < nCount) ? 0 : (index_type)this->size()-nCount;
			return std::basic_string_view<tchar>{*this}.substr(iBegin, nCount);
		}

		[[nodiscard]] TString SpanIncluding(std::basic_string_view<tchar> svCharSet) const {
			TString str;
			index_type n {0};
			for (tchar c : *this) {
				if (svCharSet.find(c) != this->npos)
					n++;
			}
			if (n) {
				str.reserve(n);
				for (tchar c : *this) {
					if (svCharSet.find(c) != this->npos)
						str += c;
				}
			}
			return str;
		}
		[[nodiscard]] TString SpanExcluding(std::basic_string_view<tchar> svCharSet) const {
			TString str;
			index_type n {0};
			for (tchar c : *this) {
				if (svCharSet.find(c) == this->npos)
					n++;
			}
			if (n) {
				str.reserve(n);
				for (tchar c : *this) {
					if (svCharSet.find(c) == this->npos)
						str += c;
				}
			}
			return str;
		}

		void MakeUpper()				{ for (auto& ch : *this) ch = std::toupper(ch); }
		void MakeLower()				{ for (auto& ch : *this) ch = std::tolower(ch); }
		void MakeReverse()				{ std::reverse(this->begin(), this->end()); }

		[[nodiscard]] TString GetUpper() const {
			auto l = this->size();
			std::basic_string<tchar> str(l, 0);
			for (size_t i = 0; i < l; i++)
				str[i] = ToUpper(this->at(i));
			//static std::locale loc {fmt::format("en_US.UTF-16", (int)eCODEPAGE_DEFAULT<tchar>)};
			//std::transform(this->begin(), this->end(), str.begin(), [&](tchar c) -> tchar { return std::toupper<tchar>(c, loc); });
			return (TString&)str;
		}
		[[nodiscard]] TString GetLower() const {
			auto l = this->size();
			std::basic_string<tchar> str(l, 0);
			for (size_t i = 0; i < l; i++)
				str[i] = ToLower(this->at(i));
			//static std::locale loc {fmt::format("en_US.UTF-16", (int)eCODEPAGE_DEFAULT<tchar>)};
			//std::transform(this->begin(), this->end(), str.begin(), [&](tchar c) -> tchar { return std::tolower<tchar>(c, loc); });
			return (TString&)str;
		}
		[[nodiscard]] TString GetReverse() const {
			std::basic_string<tchar> str;
			str.resize(this->size());
			std::reverse_copy(this->begin(), this->end(), str.begin());
			return (TString&)str;
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
		TString GetReplaced(tchar chOld, tchar chNew) const {
			std::basic_string<tchar> str;
			str.resize(this->size());
			std::replace_copy(this->begin(), this->end(), str.begin(), chOld, chNew);
			return str;
		}

		/// @brief no recursive Replace.
		int Replace(std::basic_string_view<tchar> svOld, std::basic_string_view<tchar> svNew) {
			int nReplaced {};
			if (auto r = GetReplacedOpt(svOld, svNew, &nReplaced); r) {
				*this = std::move(*r);
			}
			return nReplaced;
		}

		/// @brief no recursive Replace.
		TString GetReplaced(std::basic_string_view<tchar> svOld, std::basic_string_view<tchar> svNew, int* pnReplaced = nullptr) const {
			auto r = GetReplacedOpt(svOld, svNew, pnReplaced);
			return r.value_or(*this);
		}


		/// @brief no recursive Replace.
		/// @return replaced str. if no replace occur, returns empty
		std::optional<TString> GetReplacedOpt(std::basic_string_view<tchar> svOld, std::basic_string_view<tchar> svNew, int* pnReplaced = nullptr) const {
			if (svOld.empty())
				return {};
			index_type nLenOld = svOld.size();
			index_type nLenNew = svNew.size();
			int nToReplace {};
			if (pnReplaced)
				*pnReplaced = 0;

			tchar const* pszStart = this->data();
			tchar const* pszEnd = pszStart + this->size();
			tchar const* pszEndSearch = pszEnd-nLenOld+1;

			// 먼저 몇 개 바꿔야 되는지 찾고
			for (tchar const* psz = pszStart; psz < pszEndSearch; psz++) {
				if (tszncmp({psz, pszEnd}, svOld, nLenOld) == 0) {
					psz += nLenOld-1;
					nToReplace++;
				}
			}
			if (!nToReplace)
				return {};

			if (pnReplaced)
				*pnReplaced = nToReplace;

			index_type nLeft = nToReplace;
			size_type nLen = this->size() + (nLenNew - nLenOld) * nToReplace;
			TString strReplaced;
			if (nLen) {
				strReplaced.resize(nLen);
				tchar* pszReplaced = strReplaced.data();
				const tchar* psz = pszStart;
				for (; nLeft && psz < pszEndSearch; psz++) {
					if (tszncmp({psz, pszEnd}, svOld, nLenOld) == 0) {
						if (nLenNew) {
							memcpy(pszReplaced, svNew.data(), nLenNew*sizeof(tchar));	// funcion tszncpy() appends NULL. don't need NULL to be copied into dest.
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
			}
			return strReplaced;
		}

	public:
		index_type Remove(tchar chRemove) {
			auto nNewLen = tszrmchar(this->data(), this->data()+this->size(), chRemove);
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


	public:
		/// @brief fmt::format
		template < typename S, typename ... Args >
		TString& Format(S const& format_str, Args&& ... args) {
			return *this = fmt::format<S, Args...>(format_str, std::forward<Args>(args)...);
		}


	public:
		/// @brief Split string into...
		[[nodiscard]] std::vector<TString> Split(std::basic_string_view<tchar> svDelimiters) const {
			return gtl::internal::TSplit<tchar, TString>(*this, [svDelimiters](tchar c) -> bool { return svDelimiters.find(c) != svDelimiters.npos; });
		}
		[[nodiscard]] inline std::vector<TString> Split(tchar cDelimiter) const {
			return gtl::internal::TSplit<tchar, TString>(*this, [cDelimiter](tchar c) -> bool { return cDelimiter == c; });
		}
		[[nodiscard]] std::vector<std::basic_string_view<tchar>> SplitView(std::basic_string_view<tchar> svDelimiters) const {
			return gtl::internal::TSplit<tchar, std::basic_string_view<tchar>>(*this, [svDelimiters](tchar c) -> bool { return svDelimiters.find(c) != svDelimiters.npos; });
		}
		[[nodiscard]] inline std::vector<std::basic_string_view<tchar>> SplitView(tchar cDelimiter) const {
			return gtl::internal::TSplit<tchar, std::basic_string_view<tchar>>(*this, [cDelimiter](tchar c) -> bool { return cDelimiter == c; });
		}


		/// @brief Manual Buffer Manage
		[[nodiscard]] tchar* GetBuffer(size_type nNewLen) {
			this->resize(nNewLen);
			return this->data();
		}
		/// @brief Manual Buffer Manage
		size_type ReleaseBuffer() {	// reserve(), data() 함수를 사용해서 버퍼를 가져온 다음, 버퍼에 데이터를 보낸 다음, 크기 다시 설정.
			if (!this->data() || !this->size())
				return 0;
			size_type nNewLen = tszlen(this->data(), this->size());
			this->resize(nNewLen, 0);
			return this->size();
		}


	};	// TString


	using xString = TString<wchar_t>;
	using xStringA = TString<char>;				// MBCS
	using xStringW = TString<wchar_t>;			// Unicode (WideChar)
	using xStringU8 = TString<char8_t>;			// Unicode UTF-8
	using xStringU16 = TString<char16_t>;		// Unicode UTF-16
	using xStringU32 = TString<char32_t>;		// Unicode UTF-32
#if (GTL__STRING_SUPPORT_CODEPAGE_KSSM)
	using xStringKSSM = TString<charKSSM_t>;		// KSSM (codepage 1361)
#endif

	namespace internal {
		// fmt::format adapter
		template <typename tchar, typename... targs>
		using tformat_string = fmt::basic_format_string<tchar, std::type_identity_t<targs>...>;
	};
		
	template < typename tchar, typename ... targs>
	constexpr auto TFormat(fmt::basic_string_view<tchar> fmt, targs&& ... args) -> std::basic_string<tchar> {
		if constexpr (concepts::is_one_of<tchar, char, wchar_t>) {
			return fmt::format(fmt::runtime(fmt), std::forward<targs>(args)...);
		}
		else {
			return fmt::format(fmt, std::forward<targs>(args)...);
		}
		////return fmt::format(fmt, std::forward<targs>(args)...);
		//return fmt::vformat(fmt, fmt::make_format_args<fmt::buffer_context<tchar>>(args...));
	}
	template < typename ... targs> constexpr [[nodiscard]] std::basic_string<char> Format(gtl::internal::tformat_string<char, targs...> const& fmt, targs&& ... args)			{ return TFormat<char>(fmt, std::forward<targs>(args)...); }
	template < typename ... targs> constexpr [[nodiscard]] std::basic_string<wchar_t> Format(gtl::internal::tformat_string<wchar_t, targs...> const& fmt, targs&& ... args)		{ return TFormat<wchar_t>(fmt, std::forward<targs>(args)...); }
	template < typename ... targs> constexpr [[nodiscard]] std::basic_string<char8_t> Format(gtl::internal::tformat_string<char8_t, targs...> const& fmt, targs&& ... args)		{ return TFormat<char8_t>(fmt, std::forward<targs>(args)...); }
	template < typename ... targs> constexpr [[nodiscard]] std::basic_string<char16_t> Format(gtl::internal::tformat_string<char16_t, targs...> const& fmt, targs&& ... args)	{ return TFormat<char16_t>(fmt, std::forward<targs>(args)...); }
	template < typename ... targs> constexpr [[nodiscard]] std::basic_string<char32_t> Format(gtl::internal::tformat_string<char32_t, targs...> const& fmt, targs&& ... args)	{ return TFormat<char32_t>(fmt, std::forward<targs>(args)...); }

	template < typename tchar, typename toutput, typename ... targs>
	constexpr auto TFormatTo(toutput& out, fmt::basic_string_view<tchar> fmt, targs&& ... args) {
		//return fmt::vformat_to(out, fmt, fmt::make_format_args<fmt::buffer_context<tchar>>(std::forward<targs>(args)...));
		return fmt::format_to(out, fmt, std::forward<targs>(args)...);
	}
	template < typename toutput, typename ... targs> constexpr decltype(auto) FormatTo(toutput& out, gtl::internal::tformat_string<char, targs...> const& fmt, targs&& ... args)		{ return TFormatTo<char>(out, fmt, std::forward<targs>(args)...); }
	template < typename toutput, typename ... targs> constexpr decltype(auto) FormatTo(toutput& out, gtl::internal::tformat_string<wchar_t, targs...> const& fmt, targs&& ... args)		{ return TFormatTo<wchar_t>(out, fmt, std::forward<targs>(args)...); }
	template < typename toutput, typename ... targs> constexpr decltype(auto) FormatTo(toutput& out, gtl::internal::tformat_string<char8_t, targs...> const& fmt, targs&& ... args)		{ return TFormatTo<char8_t>(out, fmt, std::forward<targs>(args)...); }
	template < typename toutput, typename ... targs> constexpr decltype(auto) FormatTo(toutput& out, gtl::internal::tformat_string<char16_t, targs...> const& fmt, targs&& ... args)	{ return TFormatTo<char16_t>(out, fmt, std::forward<targs>(args)...); }
	template < typename toutput, typename ... targs> constexpr decltype(auto) FormatTo(toutput& out, gtl::internal::tformat_string<char32_t, targs...> const& fmt, targs&& ... args)	{ return TFormatTo<char32_t>(out, fmt, std::forward<targs>(args)...); }


	template < typename tchar_to, gtl::xStringLiteral literal, typename ... targs >
	inline static std::basic_string<tchar_to> FormatToTString(targs&& ... args) {
		fmt::basic_format_string<tchar_to, targs...>{gtl::TStringLiteral<tchar_to, literal>{}.value};	// Compiltime Validation only
		static constexpr gtl::TStringLiteral<tchar_to, literal> tfmt{};
		if constexpr (gtlc::is_one_of<tchar_to, char, wchar_t>) {
			return fmt::format(fmt::runtime(tfmt.value), std::forward<targs>(args)...);
		}
		else {
			return fmt::format(tfmt.value, std::forward<targs>(args)...);
		}
	}

	//inline fmt::basic_runtime<char> RuntimeFormatString(std::string_view s) { return {{s}}; }
	//inline fmt::basic_runtime<wchar_t> RuntimeFormatString(std::wstring_view s) { return {{s}}; }
	//inline fmt::basic_runtime<char8_t> RuntimeFormatString(std::u8string_view s) { return {{s}}; }
	//inline fmt::basic_runtime<char16_t> RuntimeFormatString(std::u16string_view s) { return {{s}}; }
	//inline fmt::basic_runtime<char32_t> RuntimeFormatString(std::u32string_view s) { return {{s}}; }
	template < gtlc::string_elem tchar >
	constexpr std::basic_string_view<tchar> GetDefaultFormatString() {
		if constexpr (std::is_same_v<tchar, char>) return "{}";
		else if constexpr (std::is_same_v<tchar, wchar_t>) return L"{}";
		else if constexpr (std::is_same_v<tchar, char16_t>) return u"{}";
		else if constexpr (std::is_same_v<tchar, char32_t>) return U"{}";
		else if constexpr (std::is_same_v<tchar, char8_t>) return u8"{}";
		else static_assert(gtlc::dependent_false_v, "invalid type");
	}

#pragma pack(pop)
};	// namespace gtl;


#pragma warning(pop)



#endif	// GTL__HEADER__STRING
