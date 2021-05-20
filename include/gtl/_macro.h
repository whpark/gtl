#pragma once

//////////////////////////////////////////////////////////////////////
//
// _macro.h: macros to export from gtl_module.
//
// PWH
//    2021.05.20.
//
//////////////////////////////////////////////////////////////////////


#include <version>	// version information
#include <source_location>
#include "gtl/_config.h"

#define GTL_DEPR_SEC [[deprecated("NOT Secure")]]

// until c++20 source_location
#ifdef __cpp_lib_source_location
#	define GTL__FUNCSIG std::string(std::source_location::current().function_name()) + " : "
#else
	#ifdef _MSC_VER
		#define GTL__FUNCSIG __FUNCSIG__ " : "
	#else
		#error ..... ????...
	#endif
#endif

#define NOMINMAX	// disable Windows::min/max
#if defined(min) || defined(max)
#	error min/max must not be defined. turn it off using '#define NOMINMAX' before '#include <Windows.h>''
#endif


// NUM_ARGS(...) macro arg °¹¼ö
#define NUM_ARGS_64(                            _64,_63,_62,_61,_60,_59,_58,_57,_56,_55,_54,_53,_52,_51,_50,_49,_48,_47,_46,_45,_44,_43,_42,_41,_40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1, N, ...) N
#define NUM_ARGS(...) NUM_ARGS_64( __VA_ARGS__ , 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
static_assert(NUM_ARGS(1, 2, 3) == 3, "for MSVC, add compiler option /Zc:preprocessor");


//-----------------------------------------------------------------------------
// Text Constant macros
#define TEXT_A(x) x
#define ___WIDE_TEXT(x) L##x
#define TEXT_W(x) ___WIDE_TEXT(x)
#define ___UTF8_TEXT(x) u8##x
#define TEXT_u8(x) ___UTF8_TEXT(x)
#define ___UTF16_TEXT(x) u##x
#define TEXT_u(x) ___UTF16_TEXT(x)
#define ___UTF32_TEXT(x) U##x
#define TEXT_U(x) ___UTF32_TEXT(x)

#define _EOL			"\r\n"
#define EOL				TEXT_A(_EOL)
#define EOLA			TEXT_A(_EOL)
#define EOLW			TEXT_W(_EOL)
#define EOLu8			TEXT_u8(_EOL)
#define EOLu			TEXT_u(_EOL)
#define EOLU			TEXT_U(_EOL)
#define SPACE_STRING	" \t\r\n"

