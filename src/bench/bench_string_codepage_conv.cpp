#include "benchmark/benchmark.h"

#include "gtl/gtl.h"
#include "gtl/string/utf_char_view.h"

using namespace std::literals;
using namespace gtl::literals;



constexpr std::array<std::u8string_view, 3> TEST_STRING = { {
		u8"가나다라마바사아자차카타파하가나다라마바사아자차카타파하가나다라마바사아자차카타파하"sv,
		u8"asdfasdf가나다라마adrg바sfdgdh사아자차카타dd파하긎긣꿳fghs뎓뫓멙뻍😊❤✔"sv,
		u8"asdfasdfaskdfjaklsjgflak;sdfjaskl;dfjnvakls;dfnvja;slfvnlikasjf"sv
	} };

namespace gtl {
	template < typename tchar >
	inline [[nodiscard]] std::optional<std::basic_string<tchar>> CheckAndConvertEndian(std::basic_string_view<tchar> sv, eCODEPAGE eCodepage) {
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


static void StringCodepageConv_U8toU32_CountAndConvert(benchmark::State& state) {

#pragma warning(push)
#pragma warning(disable: 4566)


	std::u8string_view svFrom { TEST_STRING[state.range(0)] };
	for (auto _ : state) {
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
			continue;
		str.reserve(nOutputLen);

		pos = svFrom.data();
		while (pos < end) {
			gtl::internal::UTFCharConverter<char32_t, char8_t, true, false, false>(pos, end, str);
		}

		// check endian, Convert
		gtl::CheckAndConvertEndian(str, gtl::eCODEPAGE::DEFAULT);

	}


#pragma warning(pop)

}


static void StringCodepageConv_U8toU32_NoCountAndConvert(benchmark::State& state) {

#pragma warning(push)
#pragma warning(disable: 4566)

	std::u8string_view svFrom { TEST_STRING[state.range(0)] };
	for (auto _ : state) {
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
			continue;
		str.reserve(nOutputLen);

		pos = svFrom.data();
		while (pos < end) {
			gtl::internal::UTFCharConverter<char32_t, char8_t, true, true, true>(pos, end, str);
		}
		str.shrink_to_fit();

		// check endian, Convert
		gtl::CheckAndConvertEndian(str, gtl::eCODEPAGE::DEFAULT);

	}

#pragma warning(pop)

}

static void StringCodepageConv_U8toU32_NoCountAndConvertNoShrink(benchmark::State& state) {

#pragma warning(push)
#pragma warning(disable: 4566)

	std::u8string_view svFrom { TEST_STRING[state.range(0)] };
	for (auto _ : state) {
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
			continue;
		str.reserve(nOutputLen);

		pos = svFrom.data();
		while (pos < end) {
			gtl::internal::UTFCharConverter<char32_t, char8_t, true, true, true>(pos, end, str);
		}

		// check endian, Convert
		gtl::CheckAndConvertEndian(str, gtl::eCODEPAGE::DEFAULT);

	}

#pragma warning(pop)

}


static void StringCodepageConv_U8toU32_coroutine(benchmark::State& state) {

#pragma warning(push)
#pragma warning(disable: 4566)

	std::u8string_view svFrom { TEST_STRING[state.range(0)] };
	for (auto _ : state) {
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

	}

#pragma warning(pop)

}

BENCHMARK(StringCodepageConv_U8toU32_CountAndConvert)->Arg(0)->Arg(1)->Arg(2);
BENCHMARK(StringCodepageConv_U8toU32_NoCountAndConvert)->Arg(0)->Arg(1)->Arg(2);
BENCHMARK(StringCodepageConv_U8toU32_NoCountAndConvertNoShrink)->Arg(0)->Arg(1)->Arg(2);
BENCHMARK(StringCodepageConv_U8toU32_coroutine)->Arg(0)->Arg(1)->Arg(2);




//=====================================================================================================

