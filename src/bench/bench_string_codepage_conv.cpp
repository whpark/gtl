#include "benchmark/benchmark.h"

#include "gtl/gtl.h"


using namespace std::literals;
using namespace gtl::literals;

#define TEST_STRING "asdfasdf가나다라마adrg바sfdgdh사아자차카타dd파하긎긣꿳fghs뎓뫓멙뻍😊❤✔"
//#define TEST_STRING "asdfasdfaskdfjaklsjgflak;sdfjaskl;dfjnvakls;dfnvja;slfvnlikasjf"

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

static void StringCodepageConv_U8toU32_CountAndConvert(benchmark::State& state) {

#pragma warning(push)
#pragma warning(disable: 4566)


	for (auto _ : state) {
		std::u32string str;
		std::u8string_view svFrom { _u8(TEST_STRING) };

		//if (svFrom.empty())
		//	return str;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		size_t nOutputLen = 0;
		auto const* pszEnd = svFrom.data() + svFrom.size();
		for (auto const* psz = svFrom.data(); psz < pszEnd; psz++) {
			auto const c = psz[0];

			if (c <= 0x7f)
				;
			else if ((c & 0b1110'0000) == 0b1100'0000) {	// 0~0x7f
				if ( (psz+1 >= pszEnd)
					|| ((*++psz & 0b1100'0000) != 0b1000'0000)
					)
					throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
			}
			else if ((c & 0b1111'0000) == 0b1110'0000) {	// ~0x7ff
				if ((psz+2 >= pszEnd)
					|| ((*++psz & 0b1100'0000) != 0b1000'0000)
					|| ((*++psz & 0b1100'0000) != 0b1000'0000)
					)
					throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
			}
			else if ((c & 0b1111'1000) == 0b1111'0000) {	// ~0xffff
				if ((psz+3 >= pszEnd)
					|| ((*++psz & 0b1100'0000) != 0b1000'0000)
					|| ((*++psz & 0b1100'0000) != 0b1000'0000)
					|| ((*++psz & 0b1100'0000) != 0b1000'0000)
					)
					throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
			}
			else {
				throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
			}

			nOutputLen++;
		}

		if (nOutputLen <= 0)
			continue;

		str.reserve(nOutputLen);
		for (auto const* psz = svFrom.data(); psz < pszEnd; psz++) {
			constexpr uint8_t mask_3bits { 0b0000'0111 };
			constexpr uint8_t mask_4bits { 0b0000'1111 };
			constexpr uint8_t mask_6bits { 0b0011'1111 };

			auto const c { *psz };

			char32_t v{};
			if (c <= 0x7f)
				v = c;
			else if ((c & 0b1110'0000) == 0b1100'0000) {	// 0~0x7f
				v = ((char32_t)*psz & mask_6bits) << 6;
				v |= (char32_t)*++psz & mask_6bits;
			}
			else if ((c & 0b1111'0000) == 0b1110'0000) {	// ~0x7ff
				v = ((char32_t)*psz & mask_4bits) << 12;
				v |= ((char32_t)*++psz & mask_6bits) << 6;
				v |= (*++psz & mask_6bits);
			}
			else if ((c & 0b1111'1000) == 0b1111'0000) {	// ~0xffff
				v = ((char32_t)*psz & mask_3bits) << 18;
				v |= ((char32_t)*++psz & mask_6bits) << 12;
				v |= ((char32_t)*++psz & mask_6bits) << 6;
				v |= (*++psz & mask_6bits);
			}

			str.push_back(v);
		}

		// check endian, Convert
		gtl::CheckAndConvertEndian(str, gtl::eCODEPAGE::DEFAULT);

	}


#pragma warning(pop)

}
BENCHMARK(StringCodepageConv_U8toU32_CountAndConvert);


static void StringCodepageConv_U8toU32_NoCountAndConvert(benchmark::State& state) {

#pragma warning(push)
#pragma warning(disable: 4566)

	for (auto _ : state) {
		std::u32string str;
		std::u8string_view svFrom { _u8(TEST_STRING) };

		//if (svFrom.empty())
		//	return str;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		size_t nOutputLen = svFrom.size();
		auto const* pszEnd = svFrom.data() + svFrom.size();
		if (nOutputLen <= 0)
			continue;

		str.reserve(nOutputLen);
		for (auto const* psz = svFrom.data(); psz < pszEnd; psz++) {
			constexpr uint8_t mask_3bits { 0b0000'0111 };
			constexpr uint8_t mask_4bits { 0b0000'1111 };
			constexpr uint8_t mask_6bits { 0b0011'1111 };

			auto const c { *psz };

			char32_t v{};
			if (c <= 0x7f)
				v = c;
			else if ((c & 0b1110'0000) == 0b1100'0000) {	// 0~0x7f
				if ( (psz+1 >= pszEnd)
					|| ((psz[1] & 0b1100'0000) != 0b1000'0000)
					)
					throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
				v = ((char32_t)*psz & mask_6bits) << 6;
				v |= (char32_t)*++psz & mask_6bits;
			}
			else if ((c & 0b1111'0000) == 0b1110'0000) {	// ~0x7ff
				if ((psz+2 >= pszEnd)
					|| ((psz[1] & 0b1100'0000) != 0b1000'0000)
					|| ((psz[2] & 0b1100'0000) != 0b1000'0000)
					)
					throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
				v = ((char32_t)*psz & mask_4bits) << 12;
				v |= ((char32_t)*++psz & mask_6bits) << 6;
				v |= (*++psz & mask_6bits);
			}
			else if ((c & 0b1111'1000) == 0b1111'0000) {	// ~0xffff
				if ((psz+3 >= pszEnd)
					|| ((psz[1] & 0b1100'0000) != 0b1000'0000)
					|| ((psz[2] & 0b1100'0000) != 0b1000'0000)
					|| ((psz[3] & 0b1100'0000) != 0b1000'0000)
					)
					throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
				v = ((char32_t)*psz & mask_3bits) << 18;
				v |= ((char32_t)*++psz & mask_6bits) << 12;
				v |= ((char32_t)*++psz & mask_6bits) << 6;
				v |= (*++psz & mask_6bits);
			}

			str.push_back(v);
		}
		str.shrink_to_fit();

		// check endian, Convert
		gtl::CheckAndConvertEndian(str, gtl::eCODEPAGE::DEFAULT);

	}

#pragma warning(pop)

}
BENCHMARK(StringCodepageConv_U8toU32_NoCountAndConvert);

static void StringCodepageConv_U8toU32_NoCountAndConvertNoShrink(benchmark::State& state) {

#pragma warning(push)
#pragma warning(disable: 4566)

	for (auto _ : state) {
		std::u32string str;
		std::u8string_view svFrom { _u8(TEST_STRING) };

		//if (svFrom.empty())
		//	return str;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		size_t nOutputLen = svFrom.size();
		auto const* pszEnd = svFrom.data() + svFrom.size();
		if (nOutputLen <= 0)
			continue;

		str.reserve(nOutputLen);
		for (auto const* psz = svFrom.data(); psz < pszEnd; psz++) {
			constexpr uint8_t mask_3bits { 0b0000'0111 };
			constexpr uint8_t mask_4bits { 0b0000'1111 };
			constexpr uint8_t mask_6bits { 0b0011'1111 };

			auto const c { *psz };

			char32_t v{};
			if (c <= 0x7f)
				v = c;
			else if ((c & 0b1110'0000) == 0b1100'0000) {	// 0~0x7f
				if ( (psz+1 >= pszEnd)
					|| ((psz[1] & 0b1100'0000) != 0b1000'0000)
					)
					throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
				v = ((char32_t)*psz & mask_6bits) << 6;
				v |= (char32_t)*++psz & mask_6bits;
			}
			else if ((c & 0b1111'0000) == 0b1110'0000) {	// ~0x7ff
				if ((psz+2 >= pszEnd)
					|| ((psz[1] & 0b1100'0000) != 0b1000'0000)
					|| ((psz[2] & 0b1100'0000) != 0b1000'0000)
					)
					throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
				v = ((char32_t)*psz & mask_4bits) << 12;
				v |= ((char32_t)*++psz & mask_6bits) << 6;
				v |= (*++psz & mask_6bits);
			}
			else if ((c & 0b1111'1000) == 0b1111'0000) {	// ~0xffff
				if ((psz+3 >= pszEnd)
					|| ((psz[1] & 0b1100'0000) != 0b1000'0000)
					|| ((psz[2] & 0b1100'0000) != 0b1000'0000)
					|| ((psz[3] & 0b1100'0000) != 0b1000'0000)
					)
					throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
				v = ((char32_t)*psz & mask_3bits) << 18;
				v |= ((char32_t)*++psz & mask_6bits) << 12;
				v |= ((char32_t)*++psz & mask_6bits) << 6;
				v |= (*++psz & mask_6bits);
			}

			str.push_back(v);
		}

		// check endian, Convert
		gtl::CheckAndConvertEndian(str, gtl::eCODEPAGE::DEFAULT);

	}

#pragma warning(pop)

}

BENCHMARK(StringCodepageConv_U8toU32_NoCountAndConvertNoShrink);
