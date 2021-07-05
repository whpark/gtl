module;

#include "framework.h"
#include <chrono>

export module gtlw:misc;
import gtl;

export namespace gtl::win_util {

	using string_t = std::wstring;
	using string_view_t = std::wstring_view;

	CString GetErrorMessage(CException& e) {
		CString str;
		e.GetErrorMessage(str.GetBuffer(1024), 1024);
		str.ReleaseBuffer();
		str.TrimRight();
		return str;
	}

	CString GetErrorMessage(DWORD dwLastError) {
		wchar_t buf[1024];
		FormatMessage( 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			dwLastError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR)buf,
			0,
			NULL 
		);

		CString str = buf;
		str.TrimRight();

		return str;
	}

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
				static_assert(false);
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
	template < typename tchar, typename tresolution = std::chrono::duration<double> >
	class TStopWatch : public gtl::TStopWatch<tchar, tresolution > {
	public:
		static inline TDebugOutputStreamBuf<tchar> osbuf;
		static inline std::basic_ostream<tchar> os{&osbuf};	// !! Destruction Order ... (<- static inline)
		using base_t = gtl::TStopWatch<tchar, tresolution>;

		TStopWatch() : /*os(&osbuf),*/ base_t(os) {};
	};

	using xStopWatchA = TStopWatch<char>;
	using xStopWatchW = TStopWatch<wchar_t>;


}

