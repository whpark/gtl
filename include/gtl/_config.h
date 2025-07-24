#pragma once

#define GTL__STRING_PRIMITIVES__WINDOWS_FRIENDLY false
#define GTL__STRING_SUPPORT_CODEPAGE_KSSM true
#define GTL__STRING_SUPPORT_CODEPAGE_KSSM_LIBICONV true
#define GTL__USE_WINDOWS_API false

#define GTL__DEFAULT_CODEPAGE 65001	// 65001 : UTF8
									//   949 : Korean (KSC5601, CP949 ...)

// for boost json, you can set only one or none to 'true'
#ifndef GTL__USE_BOOST_JSON
#	define GTL__USE_BOOST_JSON					true
#endif
#define GTL__BOOST_JSON__AS_STANDALONE		false
#define GTL__BOOST_JSON__AS_NESTED_LIB		false
#define GTL__BOOST_JSON__AS_SHARED_LIB		false

#ifndef GTL__USE_NLOHMANN_JSON
#	define GTL__USE_NLOHMANN_JSON				true
#endif

