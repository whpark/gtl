#pragma once
//=========
//Automatically Generated File.
//
//        PWH.
//
//=========


#include <cstdint>
#include <map>

namespace gtl {
#pragma pack(push, 8)


	struct S_HANGEUL_CODE {
		wchar_t cUnicode;
		uint16_t cKSC;
		uint16_t c949;
		uint16_t cKSSM;
	};
	
	using T_HANGEUL_TABLE = std::array<S_HANGEUL_CODE, 11223>;

	T_HANGEUL_TABLE const& GetHangeulCodeTable();
	std::map<uint16_t, wchar_t> const& GetHangeulCodeMapKSSMtoW();
	std::map<wchar_t, uint16_t> const& GetHangeulCodeMapWtoKSSM();

#pragma pack(pop)
}	// namespace gtl

