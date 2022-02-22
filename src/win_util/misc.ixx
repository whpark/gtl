module;

#include "framework.h"
#include <chrono>

export module gtl.win_util:misc;
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

		template < typename ... Args >
		void Lap(std::string_view sv, Args&& ... args) {
			auto t = tclock::now();
			osA << std::format(GTL__PRINT_FMT_STOPWATCH_PRE, ' ', base_t::depth * 4);
			osA << std::format(sv, std::forward<Args>(args)...);
			osA << std::format(GTL__PRINT_FMT_STOPWATCH_POST, std::chrono::duration_cast<tresolution>(t - base_t::t0));
			base_t::t0 = tclock::now();
		}
		template < typename ... Args >
		void Lap(std::wstring_view sv, Args&& ... args) {
			auto t = tclock::now();
			osW << std::format(L"" GTL__PRINT_FMT_STOPWATCH_PRE, ' ', base_t::depth * 4);
			osW << std::format(sv, std::forward<Args>(args)...);
			osW << std::format(L"" GTL__PRINT_FMT_STOPWATCH_POST, std::chrono::duration_cast<tresolution>(t - base_t::t0));
			base_t::t0 = tclock::now();
		}
	#undef GTL__PRINT_FMT_STOPWATCH_POST
	#undef GTL__PRINT_FMT_STOPWATCH_PRE
	};

	using xStopWatch = TStopWatch;


}

