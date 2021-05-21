#pragma once

#define GTL_STRING_PRIMITIVES__WINDOWS_FRIENDLY false
#define GTL_STRING_SUPPORT_CODEPAGE_KSSM true
#define GTL_STRING_SUPPORT_CODEPAGE_KSSM_LIBICONV true
#define GTL_USE_WINDOWS_API true

#define GTL_DEFAULT_CODEPAGE 949	// for Windows only in South Korea. you can change the default codepage for your own.

// for boost json, you can set only one or none to 'true'
#define GTL__BOOST_JSON__AS_STANDALONE		true
#define GTL__BOOST_JSON__AS_NESTED_LIB		false
#define GTL__BOOST_JSON__AS_SHARED_LIB		false
