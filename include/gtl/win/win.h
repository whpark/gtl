#pragma once

//////////////////////////////////////////////////////////////////////
//
// mfc_util.h: from Mocha - xUtil.h
//
// PWH
// 2000.????
// 2021.06.10 gtl.
//
///////////////////////////////////////////////////////////////////////////////

#define NOMINMAX

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include "gtl/gtl.h"
#include "_lib_gtl_win.h"


#define EXT_DEFAULT_IMAGE_FILE	_T(".png")
#define FILTER_IMAGE_FILES		_T("Image Files(*.bmp;*.jpg;*.tiff;*.png)|*.bmp;*.jpg;*.tiff;*.png|Bitmap(*.bmp)|*.bmp|PNG File(*.png)|*.png|JPEG File(*.jpg)|*.jpg|All Files(*.*)|*.*||")
#define IMAGE_FILE_EXTS			_T("*.bmp;*.jpg;*.jpeg;*.tiff;*.png;*.gif")

#define INVALID_CHAR_FOR_FILE_NAME _T("\\/:*?<>|")


namespace gtl::win {
#pragma pack(push, 8)	// default align. (8 bytes)

	//GTL__WIN_API gtl::xString GetErrorMessage(CException& e);
	GTL__WIN_API gtl::xString GetErrorMessage(DWORD dwLastError);

	//-------------------------------------------------------------------------
	/// @brief StopWatch
	/// @tparam tclock 
	template < typename tchar, class ttraits = std::char_traits<tchar> >
	struct TDebugOutputStreamBuf : public std::basic_streambuf<tchar, ttraits> {
	public:
		using base_t = std::basic_streambuf<tchar, ttraits>;

		std::basic_string<tchar> str;

		virtual ~TDebugOutputStreamBuf() {
			OutputBuf();
		}
		void OutputBuf() {
			if (str.empty())
				return;
			if constexpr (gtlc::is_one_of<tchar, wchar_t, char16_t>) {
				OutputDebugStringW((wchar_t const*)str.c_str());
			} else if constexpr (gtlc::is_one_of<tchar, char, char8_t>) {
				OutputDebugStringA((char const*)str.c_str());
			} else {
				static_assert(gtlc::dependent_false_v);
			}
		}
		virtual base_t::int_type overflow(base_t::int_type c) override {
			str += (tchar)c;
			if (c == '\n') {
				OutputBuf();
				str.clear();
			}
			return 1;//traits_type::eof();
		}
	};

	//-------------------------------------------------------------------------
	/// @brief StopWatch
	template < typename tresolution = std::chrono::duration<double>, typename tclock = std::chrono::steady_clock >
	class TStopWatch : public gtl::TStopWatch<char, tresolution, tclock > {
	public:
		static inline TDebugOutputStreamBuf<char> osbufA;
		static inline TDebugOutputStreamBuf<wchar_t> osbufW;
		static inline std::ostream osA{&osbufA};	// !! Destruction Order ... (<- static inline)
		static inline std::wostream osW{&osbufW};	// !! Destruction Order ... (<- static inline)
		using base_t = gtl::TStopWatch<char, tresolution, tclock>;

	public:
		TStopWatch() : base_t(osA) {
		}
		~TStopWatch() {
			Lap("end");
			//osbufA.flush();
			//osbufW.flush();
		}

	public:
	#define GTL__PRINT_FMT_STOPWATCH_PRE "STOP_WATCH {:{}}[ "
	#define GTL__PRINT_FMT_STOPWATCH_POST " ] {}\n"

		template < typename ... targs >
		void Lap(gtl::internal::tformat_string<char, targs...> const& fmt, targs&& ... args) {
			auto t = tclock::now();
			osA << fmt::format(GTL__PRINT_FMT_STOPWATCH_PRE, ' ', base_t::depth * 4);
			osA << Format(fmt, std::forward<targs>(args)...);
			osA << fmt::format(GTL__PRINT_FMT_STOPWATCH_POST, std::chrono::duration_cast<tresolution>(t - base_t::t0));
			base_t::t0 = tclock::now();
		}
		template < typename ... targs >
		void Lap(gtl::internal::tformat_string<wchar_t, targs...> const& fmt, targs&& ... args) {
			auto t = tclock::now();
			osW << fmt::format(L"" GTL__PRINT_FMT_STOPWATCH_PRE, ' ', base_t::depth * 4);
			osW << Format(fmt, std::forward<targs>(args)...);
			osW << fmt::format(L"" GTL__PRINT_FMT_STOPWATCH_POST, std::chrono::duration_cast<tresolution>(t - base_t::t0));
			base_t::t0 = tclock::now();
		}
	#undef GTL__PRINT_FMT_STOPWATCH_POST
	#undef GTL__PRINT_FMT_STOPWATCH_PRE
	};

	using xStopWatch = TStopWatch<std::chrono::duration<double>, std::chrono::steady_clock>;

#pragma pack(pop)
}	// namespace gtl::win
