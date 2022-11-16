#pragma once

//////////////////////////////////////////////////////////////////////
//
// stop_watch.h: 디버깅용 시간 측정 (prefer spdlog)
//
// PWH
// 2021.10.31. misc.h에서 분리
//
//////////////////////////////////////////////////////////////////////

//#include "gtl/_default.h"
#include <cmath>
#include <bitset>
#include <algorithm>
#include <iostream>
#include <format>
#include <chrono>
#include <functional>
#include "gtl/concepts.h"
#include "gtl/string.h"

namespace gtl {
#pragma pack(push, 8)

	//-------------------------------------------------------------------------
	/// @brief StopWatch
	/// @tparam tclock 
	template < typename tchar, typename tresolution = std::chrono::duration<double>, typename tclock = std::chrono::steady_clock >
	class TStopWatch {
	protected:
		tclock::time_point t0 { tclock::now() };
		std::basic_ostream<tchar>& os;
		thread_local static inline int depth {-1};

		//struct item {
		//	tclock::time_point t0;
		//	tclock::time_point t1;
		//	std::string str;
		//};
		//std::deque<item> laps;
		constexpr static auto& GetDefaultOutStream() {
			if constexpr (gtlc::is_one_of<tchar, char, char8_t>) {
				return std::cout;
			} else if constexpr (gtlc::is_one_of<tchar, wchar_t, char16_t>) {
				return std::wcout;
			}
		}

	public:
		TStopWatch(std::basic_ostream<tchar>& os = GetDefaultOutStream()) : os(os) {
			depth++;
		}
		~TStopWatch() {
			//Lap(RuntimeFormatString(ToExoticString<tchar>("end")));
			if constexpr (gtlc::is_one_of<tchar, char>) {
				Lap("end");
			} else if constexpr (gtlc::is_one_of<tchar, char8_t>) {
				Lap(u8"end");
			} else if constexpr (gtlc::is_one_of<tchar, char16_t>) {
				Lap(u"end");
			} else if constexpr (gtlc::is_one_of<tchar, char32_t>) {
				Lap(U"end");
			} else if constexpr (gtlc::is_one_of<tchar, wchar_t>) {
				Lap(L"end");
			} else {
				static_assert(gtlc::dependent_false_v);
			}
			os.flush();
			depth--;
		}

		tclock::time_point GetLastTick() const { return t0; };

	public:
		void Start() {
			t0 = tclock::now();
		}
		void Stop() {}

	public:
		template < typename ... targs >
		void Lap(gtl::internal::tformat_string<tchar, targs...> const& fmt, targs&& ... args) {
			auto t = tclock::now();
		#define GTL__PRINT_FMT_STOPWATCH "STOP_WATCH {:{}}[ "
			if constexpr (gtlc::is_one_of<tchar, char>) {
				os << fmt::format(GTL__PRINT_FMT_STOPWATCH, ' ', depth * 4);
			} else if constexpr (gtlc::is_one_of<tchar, char8_t>) {
				os << fmt::format(u8"" GTL__PRINT_FMT_STOPWATCH, ' ', depth * 4);
			} else if constexpr (gtlc::is_one_of<tchar, char16_t>) {
				os << fmt::format(u"" GTL__PRINT_FMT_STOPWATCH, ' ', depth * 4);
			} else if constexpr (gtlc::is_one_of<tchar, char32_t>) {
				os << fmt::format(U"" GTL__PRINT_FMT_STOPWATCH, ' ', depth * 4);
			} else if constexpr (gtlc::is_one_of<tchar, wchar_t>) {
				os << fmt::format(L"" GTL__PRINT_FMT_STOPWATCH, ' ', depth * 4);
			} else {
				static_assert(gtlc::dependent_false_v);
			}
		#undef GTL__PRINT_FMT_STOPWATCH

			os << Format(fmt, std::forward<targs>(args)...);

		#define GTL__PRINT_FMT_STOPWATCH " ] {}\n"
			if constexpr (gtlc::is_one_of<tchar, char>) {
				os << fmt::format(GTL__PRINT_FMT_STOPWATCH, std::chrono::duration_cast<tresolution>(t-t0));
			} else if constexpr (gtlc::is_one_of<tchar, char8_t>) {
				os << fmt::format(u8"" GTL__PRINT_FMT_STOPWATCH, std::chrono::duration_cast<tresolution>(t - t0));
			} else if constexpr (gtlc::is_one_of<tchar, char16_t>) {
				os << fmt::format(u"" GTL__PRINT_FMT_STOPWATCH, std::chrono::duration_cast<tresolution>(t - t0));
			} else if constexpr (gtlc::is_one_of<tchar, char32_t>) {
				os << fmt::format(U"" GTL__PRINT_FMT_STOPWATCH, std::chrono::duration_cast<tresolution>(t - t0));
			} else if constexpr (gtlc::is_one_of<tchar, wchar_t>) {
				os << fmt::format(L"" GTL__PRINT_FMT_STOPWATCH, std::chrono::duration_cast<tresolution>(t - t0));
			} else {
				static_assert(gtlc::dependent_false_v);
			}
		#undef GTL__PRINT_FMT_STOPWATCH
			//os << (tchar)'\n';
			t0 = tclock::now();
		}
	};


#pragma pack(pop)
}	// namespac gtl
