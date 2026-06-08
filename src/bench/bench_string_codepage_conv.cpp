#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include "gtl/gtl.h"
#include "gtl/iconv_wrapper.h"
#include "gtl/string.h"
#include "gtl/string/convert_codepage.h"
#include "gtl/string/utf_char_view.h"

using namespace std::literals;
using namespace gtl::literals;

#define TEST_SZ 	"가나다라마바사아자차카타파하가나다라마바사아자차카타파하가나다라마바사아자차카타파하"\
					"asdfasdf가나다라마adrg바sfdgdh사아자차카타dd파하긎긣꿳fghs뎓뫓멙뻍😊❤✔"\
					"asdfasdfaskdfjaklsjgflak;sdfjaskl;dfjnvakls;dfnvja;slfvnlikasjf"


constexpr std::array<std::u8string_view, 3> TEST_STRING = { {
		TEXT_u8(TEST_SZ)
	} };

namespace gtl {
	template < typename tchar >
	[[nodiscard]] inline std::optional<std::basic_string<tchar>> CheckAndConvertEndian(std::basic_string_view<tchar> sv, eCODEPAGE eCodepage) {
		if (eCODEPAGE_OTHER_ENDIAN<tchar> != eCodepage) {
			[[likely]]
			return {};
		}
		std::basic_string<tchar> strOther;
		strOther.reserve(sv.size());
		for (auto c : sv) {
			strOther.push_back(GetByteSwap(c));
		}
		return strOther;
	}

	template < typename tchar, template < typename, typename ... tstr_args> typename tstr, typename ... tstr_args >
	inline bool CheckAndConvertEndian(tstr<tchar, tstr_args...>& str, eCODEPAGE eCodepage) {
		if (eCODEPAGE_OTHER_ENDIAN<tchar> != eCodepage) {
			[[likely]]
			return false;
		}
		for (auto& c : str) {
			ByteSwap(c);
		}
		return true;
	}
};

namespace uc = gtl::utf_const;


static std::u32string StringCodepageConv_U8toU32_CountAndConvert(std::size_t range) {

#pragma warning(push)
#pragma warning(disable: 4566)


	std::u8string_view svFrom { TEST_STRING[range] };
	std::u32string str;

		//if (svFrom.empty())
		//	return str;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		size_t nOutputLen = 0;
		auto const* pos = svFrom.data();
		auto const* const end = pos + svFrom.size();
		while (pos < end) {
			gtl::internal::UTFCharConverter<char32_t, char8_t, false, true, true>(pos, end, nOutputLen);
		}

		if (nOutputLen <= 0)
			return str;
		str.reserve(nOutputLen);

		pos = svFrom.data();
		while (pos < end) {
			gtl::internal::UTFCharConverter<char32_t, char8_t, true, false, false>(pos, end, str);
		}

		// check endian, Convert
		gtl::CheckAndConvertEndian(str, gtl::eCODEPAGE::DEFAULT);

	return str;


#pragma warning(pop)

}


static std::u32string StringCodepageConv_U8toU32_NoCountAndConvert(std::size_t range) {

#pragma warning(push)
#pragma warning(disable: 4566)

	std::u8string_view svFrom { TEST_STRING[range] };
	std::u32string str;

		//if (svFrom.empty())
		//	return str;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		size_t nOutputLen = svFrom.size();
		auto const* pos = svFrom.data();
		auto const* const end = pos + svFrom.size();
		//for (auto const* pos = svFrom.data(); pos < end; ) {
		//	gtl::internal::UTFCharConverter<char32_t, char8_t, false, true, true>(pos, end, nOutputLen);
		//}

		if (nOutputLen <= 0)
			return str;
		str.reserve(nOutputLen);

		pos = svFrom.data();
		while (pos < end) {
			gtl::internal::UTFCharConverter<char32_t, char8_t, true, true, true>(pos, end, str);
		}
		str.shrink_to_fit();

		// check endian, Convert
		gtl::CheckAndConvertEndian(str, gtl::eCODEPAGE::DEFAULT);

	return str;

#pragma warning(pop)

}

static std::u32string StringCodepageConv_U8toU32_NoCountAndConvertNoShrink(std::size_t range) {

#pragma warning(push)
#pragma warning(disable: 4566)

	std::u8string_view svFrom { TEST_STRING[range] };
	std::u32string str;

		//if (svFrom.empty())
		//	return str;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		size_t nOutputLen = svFrom.size();
		auto const* pos = svFrom.data();
		auto const* const end = pos + svFrom.size();
		//for (pos < end; ) {
		//	gtl::internal::UTFCharConverter<char32_t, char8_t, false, true, true>(pos, end, nOutputLen);
		//}

		if (nOutputLen <= 0)
			return str;
		str.reserve(nOutputLen);

		pos = svFrom.data();
		while (pos < end) {
			gtl::internal::UTFCharConverter<char32_t, char8_t, true, true, true>(pos, end, str);
		}

		// check endian, Convert
		gtl::CheckAndConvertEndian(str, gtl::eCODEPAGE::DEFAULT);

	return str;

#pragma warning(pop)

}


static std::u32string StringCodepageConv_U8toU32_coroutine(std::size_t range) {

#pragma warning(push)
#pragma warning(disable: 4566)

	std::u8string_view svFrom { TEST_STRING[range] };
	std::u32string str;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		str.reserve(svFrom.size());
		for (auto c : gtl::SeqUTF<char32_t, char8_t>(svFrom)) {
			str.push_back(c);
		}

		// check endian, Convert
		gtl::CheckAndConvertEndian(str, gtl::eCODEPAGE::DEFAULT);

	return str;

#pragma warning(pop)

}

// BENCHMARK("U8 to U32 count and convert") { return StringCodepageConv_U8toU32_CountAndConvert(0); };
// BENCHMARK("U8 to U32 no count and convert") { return StringCodepageConv_U8toU32_NoCountAndConvert(0); };
// BENCHMARK("U8 to U32 no count and no shrink") { return StringCodepageConv_U8toU32_NoCountAndConvertNoShrink(0); };
// BENCHMARK("U8 to U32 coroutine") { return StringCodepageConv_U8toU32_coroutine(0); };

#pragma optimize ("", off)
std::optional<std::wstring> StringCodepageConv_ICONV_MBCS_WIDE() {
	std::string str{TEST_SZ};
	return gtl::ToString_iconv<wchar_t, char, "", "CP949">(str);
}
#pragma optimize ("", on)

#pragma optimize ("", off)
std::wstring StringCodepageConv_WindowsAPI_MBCS_WIDE() {
	std::string str{TEST_SZ};
	auto n = MultiByteToWideChar((int)gtl::eCODEPAGE::KO_KR_949, 0, str.c_str(), (int)str.size(), nullptr, 0);
	if (n <= 0)
		return {};
	std::wstring strW;
	strW.resize(n);
	MultiByteToWideChar((int)gtl::eCODEPAGE::KO_KR_949, 0, str.c_str(), (int)str.size(), strW.data(), (int)strW.size());
	return strW;
}
#pragma optimize ("", on)

#pragma optimize ("", off)
std::optional<std::wstring> StringCodepageConv_ICONV_UTF8_WIDE() {
	std::u8string str{TEXT_u8(TEST_SZ)};
	return gtl::ToString_iconv<wchar_t>(str);
}
#pragma optimize ("", on)

#pragma optimize ("", off)
std::wstring StringCodepageConv_WindowsAPI_UTF8_WIDE() {
	std::u8string str{TEXT_u8(TEST_SZ)};
	auto n = MultiByteToWideChar((int)gtl::eCODEPAGE::KO_KR_949, 0, (char const*)str.c_str(), (int)str.size(), nullptr, 0);
	if (n <= 0)
		return {};
	std::wstring strW;
	strW.resize(n);
	MultiByteToWideChar((int)gtl::eCODEPAGE::KO_KR_949, 0, (char const*)str.c_str(), (int)str.size(), strW.data(), (int)strW.size());
	return strW;
}
#pragma optimize ("", on)

TEST_CASE("String codepage conversion benchmarks", "[benchmark][string]") {
	BENCHMARK("ICONV MBCS to wide") {
		return StringCodepageConv_ICONV_MBCS_WIDE();
	};

	BENCHMARK("Windows API MBCS to wide") {
		return StringCodepageConv_WindowsAPI_MBCS_WIDE();
	};

	BENCHMARK("ICONV UTF-8 to wide") {
		return StringCodepageConv_ICONV_UTF8_WIDE();
	};

	BENCHMARK("Windows API UTF-8 to wide") {
		return StringCodepageConv_WindowsAPI_UTF8_WIDE();
	};
}

//#pragma optimize ("", off)
//std::optional<std::wstring> StringCodepageConv_ICONV_R_MBCS_WIDE() {
//
//	gtl::Ticonv<wchar_t, char> conv(nullptr, "CP949");
//	std::u8string str{TEXT_u8(TEST_SZ)};
//	return conv.Convert(str);
//}
//#pragma optimize ("", on)
//
//BENCHMARK(StringCodepageConv_ICONV_R_MBCS_WIDE);

//=====================================================================================================

