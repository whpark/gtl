#pragma once
//=========
//Automatically Generated File.
//
//        PWH.
//
//=========


#include "gtl/config_gtl.h"
#include "gtl/_lib_gtl.h"

#include <cstdint>
#include <map>


namespace gtl::charset {


	GTL_DATA extern std::map<char16_t, char16_t> const mapUL_latin1_g;
	GTL_DATA extern std::map<char16_t, char16_t> const mapLU_latin1_g;
	GTL_DATA extern std::map<char16_t, char16_t> const mapUL_latin_extended_g;
	GTL_DATA extern std::map<char16_t, char16_t> const mapLU_latin_extended_g;
	GTL_DATA extern std::map<char16_t, char16_t> const mapUL_other1_g;
	GTL_DATA extern std::map<char16_t, char16_t> const mapLU_other1_g;
	GTL_DATA extern std::map<char16_t, char16_t> const mapUL_other2_g;
	GTL_DATA extern std::map<char16_t, char16_t> const mapLU_other2_g;
	GTL_DATA extern std::map<char16_t, char16_t> const mapUL_other3_g;
	GTL_DATA extern std::map<char16_t, char16_t> const mapLU_other3_g;
	GTL_DATA extern std::map<char32_t, char32_t> const mapUL_other4_g;
	GTL_DATA extern std::map<char32_t, char32_t> const mapLU_other4_g;
	GTL_DATA extern std::map<char32_t, char32_t> const mapUL_other5_g;
	GTL_DATA extern std::map<char32_t, char32_t> const mapLU_other5_g;
	GTL_DATA extern std::map<char32_t, char32_t> const mapUL_other6_g;
	GTL_DATA extern std::map<char32_t, char32_t> const mapLU_other6_g;

	constexpr static inline std::pair<char16_t, char16_t> const rangeUL_latin1_g { 0x00c0, 0x00df };
	constexpr static inline std::pair<char16_t, char16_t> const rangeUL_latin_extended_g { 0x0100, 0x024f };
	constexpr static inline std::pair<char16_t, char16_t> const rangeUL_other1_g { 0x0370, 0x2cf3 };
	constexpr static inline std::pair<char16_t, char16_t> const rangeUL_other2_g { 0xa640, 0xa7f6 };
	constexpr static inline std::pair<char16_t, char16_t> const rangeUL_other3_g { 0xff21, 0xff3b };
	constexpr static inline std::pair<char32_t, char32_t> const rangeUL_other4_g { 0x10400, 0x118c0 };
	constexpr static inline std::pair<char32_t, char32_t> const rangeUL_other5_g { 0x16e40, 0x16e60 };
	constexpr static inline std::pair<char32_t, char32_t> const rangeUL_other6_g { 0x1e900, 0x1e922 };
	constexpr static inline std::pair<char16_t, char16_t> const rangeLU_latin1_g { 0x00b5, 0x0100 };
	constexpr static inline std::pair<char16_t, char16_t> const rangeLU_latin_extended_g { 0x0101, 0x0250 };
	constexpr static inline std::pair<char16_t, char16_t> const rangeLU_other1_g { 0x0250, 0x2d2e };
	constexpr static inline std::pair<char16_t, char16_t> const rangeLU_other2_g { 0xa641, 0xabc0 };
	constexpr static inline std::pair<char16_t, char16_t> const rangeLU_other3_g { 0xfb00, 0xff5b };
	constexpr static inline std::pair<char32_t, char32_t> const rangeLU_other4_g { 0x10428, 0x118e0 };
	constexpr static inline std::pair<char32_t, char32_t> const rangeLU_other5_g { 0x16e60, 0x16e80 };
	constexpr static inline std::pair<char32_t, char32_t> const rangeLU_other6_g { 0x1e922, 0x1e944 };


} // namespace gtl::charset

