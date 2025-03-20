#pragma once

//////////////////////////////////////////////////////////////////////
//
// base64.h: Encode/Decode base64, using boost.
//
// PWH
// 2023.05.10. moved from mat_helper
//
//////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <string>

#include "concepts.h"

// boost encode / decode base64
#include "boost/archive/iterators/binary_from_base64.hpp"
#include "boost/archive/iterators/base64_from_binary.hpp"
#include "boost/archive/iterators/transform_width.hpp"

namespace gtl {
#pragma pack(push, 8)

	template < typename T = uint8_t > requires (std::is_trivially_copyable_v<T> and sizeof(T) == 1)
	static std::string EncodeBase64(std::span<T> data) {
		using namespace boost::archive::iterators;
		using It = base64_from_binary<transform_width< typename std::span<T>::iterator, 6, 8>>;
		auto tmp = std::string(It(std::begin(data)), It(std::end(data)));
		return tmp.append((3 - data.size() % 3) % 3, '=');
	}

	template < typename TString, typename TOutpytIterator >
	void DecodeBase64(TString const& str, TOutpytIterator iterOutput, size_t max_size) {
		using namespace boost::archive::iterators;
		using str2bin = transform_width<binary_from_base64<typename TString::const_iterator>, 8, 6>;
		auto end = str.begin() + std::min(str.size(), max_size*8/6 + ((max_size%3)?1:0));
		std::copy(str2bin(str.begin()), str2bin(end), iterOutput);
	}

#pragma pack(pop)
}	// namespace gtl;
