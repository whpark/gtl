#include "pch.h"

#include "gtl/gtl.h"
#include "gtl/archive.h"

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

std::optional<std::basic_string<char16_t>> ReadLine(std::istream& stream, char16_t cDelim) {
	std::basic_string<char16_t> str;
	uint16_t c{};

	while (stream.read((char*)&c, sizeof(c))) {
		if (c == cDelim)
			break;
		str += c;
	}

	return str.empty() && stream.eof() ? std::optional<std::basic_string<char16_t>>{} : str;
}

namespace gtl {

	using tstream = std::istream;

	eCODEPAGE ReadCodepageBOM(std::istream& stream, eCODEPAGE eDefaultCodepage = eCODEPAGE::UTF8) {
		//CHECK_ARCHIVE_LOADABLE;
		unsigned char c = 0;

		auto peek = [](tstream& stream, std::string_view sv) -> bool {

			auto get = [](tstream& stream) -> std::optional<char> {
				char c{};
				static_assert(sizeof(c) == sizeof(typename tstream::char_type));
				if (stream.get((typename tstream::char_type&)c))
					return c;
				return {};
			};

			auto pos = stream.tellg();
			bool bMatch {true};
			for (auto c : sv) {
				char ci{};
				if (stream.read(&ci, sizeof(ci)) and (ci != c)) {
					bMatch = false;
					break;
				}
			}
			if (bMatch)
				return true;
			stream.seekg(pos);
			return false;
		};

		static std::vector<std::pair<std::string_view, eCODEPAGE>> const codepages {
			{GetCodepageBOM(eCODEPAGE::UTF32BE), eCODEPAGE::UTF32BE},	// in BOM Length order ...
			{GetCodepageBOM(eCODEPAGE::UTF32LE), eCODEPAGE::UTF32LE},
			{GetCodepageBOM(eCODEPAGE::UTF16BE), eCODEPAGE::UTF16BE},
			{GetCodepageBOM(eCODEPAGE::UTF16LE), eCODEPAGE::UTF16LE},
			{GetCodepageBOM(eCODEPAGE::UTF8), eCODEPAGE::UTF8},
		};

		for (auto const& [sv, codepage] : codepages) {
			if (peek(stream, sv)) {
				//SetDefaultCodepage(codepage);
				return codepage;
			}
		}

		//SetDefaultCodepage(eDefaultCodepage);

		return eDefaultCodepage;
	};

}

TEST(gtl_string, TArchive) {


	for (auto const& dir : std::filesystem::directory_iterator(u"Z:\\Downloads\\")) {
		if (dir.is_directory() or dir.is_other())
			continue;
		gtl::CStringU16 strFilename = dir.path().filename().generic_u16string();
		strFilename.MakeLower();
		auto str2 = strFilename.GetUpper();
		std::basic_regex<wchar_t> re{LR"xx(.*\.BOM\.CXX)xx"};
		if (!std::regex_match((std::wstring&)strFilename, re))
			continue;
		//if (!strFilename.ends_with(u".bom.cxx"sv))
		//	continue;
		std::ifstream stream(dir.path());

		auto codepage = gtl::ReadCodepageBOM(stream);
		if ((codepage == gtl::eCODEPAGE::UTF16LE) or (codepage == gtl::eCODEPAGE::UTF16BE)) {
			fmt::print("---------------------------------------\n");
			fmt::print("filename : {}\n", gtl::ToStringA(strFilename).c_str());
			fmt::print("codepage : {}\n\n", (int)codepage);
			while (stream.good()) {
				if (auto r = ReadLine(stream, u'\n'); r) {
					gtl::TrimRight(*r);
					std::string str = gtl::ToStringA(*r, {.from = codepage});
					fmt::print("{}\n", str.c_str());
				}
			}
		}

	}


	//auto r = std::getline<char16_t>(stream, str, gtl::GetByteSwap(u'\n'));
	//gtl::TArchive arIn(stream);
}

