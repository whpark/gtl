#include "pch.h"

#include "gtl/gtl.h"
#include "gtl/archive.h"

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

template <typename tchar>
auto ReadFile(auto & ar) -> std::vector<gtl::TString<tchar>> {
	std::vector<gtl::TString<tchar>> strs;
	for (decltype(ar.ReadLine<tchar>()) r; r = ar.ReadLine<tchar>(); ) {
		strs.emplace_back(std::move(*r));
	}
	return strs;
};

TEST(gtl_string, TArchive) {
	using namespace gtl;

	std::vector<std::u8string> const strs {
		u8R"xx(#include <iostream>)xx",
		u8R"xx(#include <string>)xx",
		u8R"xx(#include <string_view>)xx",
		u8R"xx()xx",
		u8R"xx(using namespace std::literals;)xx",
		u8R"xx()xx",
		u8R"xx(int main() {)xx",
		u8R"xx(	std::cout << u8"가나다라마바사";)xx",
		u8R"xx(})xx",
	};

	for (auto const& dir : std::filesystem::directory_iterator(uR"x(.\stream_test\)x")) {
		if (dir.is_directory() or dir.is_other())
			continue;
		gtl::CStringU16 strFilename(dir.path().filename().string<wchar_t>());
		auto str2 = strFilename.GetUpper();
		std::basic_regex<wchar_t> re{LR"xx(.*\.[Bb][Oo][Mm]\.[Cc][Xx][Xx]$)xx"};
		if (!std::regex_match((std::wstring&)strFilename, re))
			continue;

		std::ifstream stream(dir.path());
		gtl::TArchive ar(stream);

		stream.seekg(0);
		auto codepage = ar.ReadCodepageBOM(gtl::eCODEPAGE::UTF8);

		auto strsA = ReadFile<char>(ar);
		EXPECT_TRUE(strs.size() == strsA.size());
		for (int i = 0; i < strs.size(); i++) {
			//if (strsA[i].Compare(strs[i]) != 0) {
			//	int __{};
			//}
			EXPECT_TRUE(strsA[i].Compare(strs[i]) == 0);
		}

		//if ((codepage == gtl::eCODEPAGE::UTF16LE) or (codepage == gtl::eCODEPAGE::UTF16BE)) {
		//	fmt::print("---------------------------------------\n");
		//	fmt::print("filename : {}\n", gtl::ToStringA(strFilename).c_str());
		//	fmt::print("codepage : {}\n\n", (int)codepage);
		//	while (stream.good()) {
		//		if (auto r = ar.ReadLineU16(); r) {
		//			gtl::TrimRight(*r);
		//			std::string str = gtl::ToStringA(*r, {.from = eMBCS_Codepage_g});
		//			fmt::print("{}\n", str.c_str());
		//		}
		//	}
		//}

	}


	{
		std::ifstream stream(uR"x(.\stream_test\short file.txt)x");
		TArchive ar(stream);
		ar.ReadCodepageBOM(gtl::eCODEPAGE::DEFAULT__OR_USE_MBCS_CODEPAGE);
		auto strs = ReadFile<char16_t>(ar);

		EXPECT_TRUE(strs.size() == 1);
		if (strs.size() >= 1) {
			EXPECT_TRUE(strs[0] == u"n"sv);
		}
	}

	{
		std::ifstream stream(uR"x(.\stream_test\cp949.txt)x");
		TArchive ar(stream);
		ar.ReadCodepageBOM((eCODEPAGE)949);
		auto strs = ReadFile<wchar_t>(ar);

		EXPECT_EQ(strs.size(), 1);
		if (strs.size() >= 1) {
			EXPECT_TRUE(strs[0] == L"가나다라마바사"sv);
		}
	}

	{
		std::ifstream stream(uR"x(.\stream_test\cp949.txt)x");
		TArchive ar(stream);
		ar.ReadCodepageBOM((eCODEPAGE)949);
		auto strs = ReadFile<char32_t>(ar);

		EXPECT_EQ(strs.size(), 1);
		if (strs.size() >= 1) {
			EXPECT_TRUE(strs[0] == U"가나다라마바사"sv);
		}
	}

	if (0) {
		static std::vector<unsigned char> const na1250 { 0x81, 0x83, 0x88, 0x90, 0x98, 0xA0, 0xAD };
		static std::vector<unsigned char> const na1251 { 0x98, 0xA0, 0xAD };
		static std::vector<unsigned char> const na1252 { 0x81, 0x8d, 0x8f, 0x90, 0x9d, 0xA0, 0xAD };

		struct {
			int eCodepage{};
			std::vector<unsigned char> const& na;
		} tbl[3] = {
			{1250, na1250},
			{1251, na1251},
			{1252, na1252},
		};

		for (auto const& [eCodepage, na] : tbl) {
			std::ofstream stream(fmt::format(uR"x(.\stream_test\cp{}.txt)x", eCodepage));
			for (unsigned char c = 0x80; c >= 0x80; c++) {
				if (c > 0x80 and !(c & 0x0f))
					stream.write("\n", 1);
				if (std::find(na.begin(), na.end(), c) != na.end())
					continue;
				stream.write(&(char&)c, 1);
			}
		}
	}

	{
		std::ifstream stream(uR"x(.\stream_test\cp1250.txt)x");
		TArchive ar(stream);
		ar.ReadCodepageBOM((eCODEPAGE)1250);
		auto strs = ReadFile<char16_t>(ar);

		EXPECT_EQ(strs.size(), 8);
		if (strs.size() >= 8) {
			EXPECT_TRUE(strs[0] == u"€‚„…†‡‰Š‹ŚŤŽŹ"sv);
			EXPECT_TRUE(strs[1] == u"‘’“”•–—™š›śťžź"sv);
			EXPECT_TRUE(strs[2] == u"ˇ˘Ł¤Ą¦§¨©Ş«¬®Ż"sv);
			EXPECT_TRUE(strs[3] == u"°±˛ł´µ¶·¸ąş»Ľ˝ľż"sv);
			EXPECT_TRUE(strs[4] == u"ŔÁÂĂÄĹĆÇČÉĘËĚÍÎĎ"sv);
			EXPECT_TRUE(strs[5] == u"ĐŃŇÓÔŐÖ×ŘŮÚŰÜÝŢß"sv);
			EXPECT_TRUE(strs[6] == u"ŕáâăäĺćçčéęëěíîď"sv);
			EXPECT_TRUE(strs[7] == u"đńňóôőö÷řůúűüýţ˙"sv);
		}
	}

	{
		std::ifstream stream(uR"x(.\stream_test\cp1251.txt)x");
		TArchive ar(stream);
		ar.ReadCodepageBOM((eCODEPAGE)1251);
		auto strs = ReadFile<char16_t>(ar);

		EXPECT_EQ(strs.size(), 8);
		if (strs.size() >= 8) {
			EXPECT_TRUE(strs[0] == u"ЂЃ‚ѓ„…†‡€‰Љ‹ЊЌЋЏ"sv);
			EXPECT_TRUE(strs[1] == u"ђ‘’“”•–—™љ›њќћџ"sv);
			EXPECT_TRUE(strs[2] == u"ЎўЈ¤Ґ¦§Ё©Є«¬®Ї"sv);
			EXPECT_TRUE(strs[3] == u"°±Ііґµ¶·ё№є»јЅѕї"sv);
			EXPECT_TRUE(strs[4] == u"АБВГДЕЖЗИЙКЛМНОП"sv);
			EXPECT_TRUE(strs[5] == u"РСТУФХЦЧШЩЪЫЬЭЮЯ"sv);
			EXPECT_TRUE(strs[6] == u"абвгдежзийклмноп"sv);
			EXPECT_TRUE(strs[7] == u"рстуфхцчшщъыьэюя"sv);
		}
	}

	{
		std::ifstream stream(uR"x(.\stream_test\cp1252.txt)x");
		TArchive ar(stream);
		ar.ReadCodepageBOM((eCODEPAGE)1252);
		auto strs = ReadFile<char16_t>(ar);

		EXPECT_EQ(strs.size(), 8);
		if (strs.size() >= 8) {
			EXPECT_TRUE(strs[0] == u"€‚ƒ„…†‡ˆ‰Š‹ŒŽ"sv);
			EXPECT_TRUE(strs[1] == u"‘’“”•–—˜™š›œžŸ"sv);
			EXPECT_TRUE(strs[2] == u"¡¢£¤¥¦§¨©ª«¬®¯"sv);
			EXPECT_TRUE(strs[3] == u"°±²³´µ¶·¸¹º»¼½¾¿"sv);
			EXPECT_TRUE(strs[4] == u"ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏ"sv);
			EXPECT_TRUE(strs[5] == u"ÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞß"sv);
			EXPECT_TRUE(strs[6] == u"àáâãäåæçèéêëìíîï"sv);
			EXPECT_TRUE(strs[7] == u"ðñòóôõö÷øùúûüýþÿ"sv);
		}
	}

	//{
	//	std::wifstream stream(LR"(Z:\Downloads\z.txt)");
	//	std::wstring str;
	//	auto r = std::getline<wchar_t>(stream, str, gtl::GetByteSwap(L'\n'));
	//	gtl::TArchive arIn(stream);
	//}

}
