#pragma once

//////////////////////////////////////////////////////////////////////
//
// time.h: CSysTime
//
//			FILETIME, SYSTEMTIME <=> std::chrono::system_clock
//
// PWH
// 2019.01.10.
// 2019.07.24. QL -> GTL
// 2020.01.07. XTime -> CSysTime 으로 이름 변경, chrono 사용법에 맞춰 수정.
//				sec_t, msec_t 등 추가.
//				CSysTimeSpan 삭제. (필요 없음)
//
//////////////////////////////////////////////////////////////////////

#include "gtl/string.h"
#include "fmt/chrono.h"
//#include "gtl/string_view.h"

//#include "gtl/_pre_lib_util.h"

#include <windows.h>

namespace gtl {
#pragma pack(push, 8)

	class GTL_CLASS CSysTime;

	using system_clock_t = std::chrono::system_clock;

	using sec_t = std::chrono::duration<double>;
	using msec_t = std::chrono::duration<double, std::ratio<1, 1'000>>;
	using usec_t = std::chrono::duration<double, std::ratio<1, 1'000'000>>;
	using nsec_t = std::chrono::duration<double, std::ratio<1, 1'000'000'000>>;
	using minute_t = std::chrono::duration<double, std::ratio<60>>;
	using hour_t = std::chrono::duration<double, std::ratio<60*60>>;
	using day_t = std::chrono::duration<double, std::ratio<24*60*60>>;


	template <class _Rep, class _Period>
	minute_t GetTotalMins(std::chrono::duration<_Rep, _Period> d) {
		return std::chrono::duration_cast<minute_t>(d);
	}
	template <class _Rep, class _Period>
	hour_t GetTotalHours(std::chrono::duration<_Rep, _Period> d) {
		return std::chrono::duration_cast<hour_t>(d);
	}
	template <class _Rep, class _Period>
	day_t GetTotalDays(std::chrono::duration<_Rep, _Period> d) {
		return std::chrono::duration_cast<day_t>(d);
	}


	//-------------------------------------------------------------------------
	//
	class GTL_CLASS CSysTime : public std::chrono::time_point<system_clock_t> {
	public:
		using base_t = std::chrono::time_point<system_clock_t>;
		using clock_t = base_t::clock;// == system_clock_t;
		using duration_t = base_t::duration;
		using file_time_t = std::filesystem::file_time_type;

	public:
		CSysTime() = default;
		CSysTime(const CSysTime&) = default;
		CSysTime(CSysTime&&) = default;
		CSysTime& operator = (const CSysTime&) = default;
		CSysTime& operator = (CSysTime&&) = default;

		using base_t::base_t;
		CSysTime(std::time_t t) : base_t{clock_t::from_time_t(t)} {}
		CSysTime(file_time_t t) : base_t(system_clock_t::duration(t.time_since_epoch().count() - eSysTimeToFileTime)) {}
		CSysTime(const char*) : base_t(base_t::clock::now()) {}		// system_clock_t::now() 함수가. 타이핑 하기 귀찮으니까, "" or "now" 등으로...

		static base_t now() {
			return base_t::clock::now();
		}

		operator std::time_t () const {
			return clock_t::to_time_t(*this);
		}


		duration_t operator - (const CSysTime& B) const {
			return (const base_t&)(*this) - (const base_t&)B;
		}

		using base_t::operator +=;
		using base_t::operator -=;

		auto operator <=> (const CSysTime&) const = default;
		bool operator == (const CSysTime&) const = default;

		// 초 단위
		sec_t GetTotalSec() const { return time_since_epoch(); }

#ifdef _WINDOWS
		CSysTime(FILETIME ft) :
			base_t(system_clock_t::duration((rep&)ft - eSysTimeToFileTime))
		{
			static_assert(sizeof(rep) == sizeof(ft));
		}

		CSysTime(const SYSTEMTIME& st) {
			FILETIME ft;
			SystemTimeToFileTime(&st, &ft);
			*this = ft;
		}

		operator FILETIME () const {
			FILETIME ft;
			static_assert(sizeof(rep) == sizeof(ft) /*&& (std::endian::little == std::endian::native)*/);
			(CSysTime::rep&)ft = time_since_epoch().count() + eSysTimeToFileTime;
			return ft;
		}

		operator SYSTEMTIME () const {
			SYSTEMTIME st;
			FILETIME ft = *this;
			FileTimeToSystemTime(&ft, &st);
			return st;
		}
		FILETIME GetLocalFileTime() const {
			FILETIME ft = *this, ftLocal;
			FileTimeToLocalFileTime(&ft, &ftLocal);
			return ftLocal;
		}
		SYSTEMTIME GetLocalSystemTime() const {
			SYSTEMTIME st;
			FILETIME ft = GetLocalFileTime();
			FileTimeToSystemTime(&ft, &st);
			return st;
		}

#endif

		operator file_time_t () const {
			return file_time_t{file_time_t::duration{time_since_epoch().count() + eSysTimeToFileTime}};
		}


		[[nodiscard]] [[deprecated ("Use fmt::format")]]
		std::wstring OldFormat(std::wstring const& sv) const {
			std::wstring str;

			std::time_t t = *this;
			std::tm tm;
			localtime_s(&tm, &t);

			std::vector<TCHAR> buffer;
			buffer.assign(sv.size()+2048, 0);
			std::wcsftime(buffer.data(), buffer.capacity(), sv.c_str(), &tm);
			str = buffer.data();

			return str;
		}
		std::wstring Format(std::wstring_view sv = L"{0:%Y-%m-%d}") const {
			std::time_t t = *this;
			return fmt::format(sv, fmt::localtime(t));
		}

		constexpr inline static const CSysTime::rep eTimeToSysTime = 10'000'000;	// --> 초단위로 변경
		// for Windows only
		constexpr inline static const CSysTime::rep eSysTimeToFileTime = 116'444'736'000'000'000; //== __std_fs_file_time_epoch_adjustment;	// 1601/01/01~1970/01/01, 134774 days, 1 tick = 100 ns


		static_assert(system_clock_t::duration::period::num == 1);
		static_assert(system_clock_t::duration::period::den == eTimeToSysTime);
	};






	namespace example::time {

		template < typename T >
		void Func() {

			CSysTime t1("now");
			CSysTime t2("now");
			sec_t ts { t2 - t1 };


			// 큰 단위 -> 작은 단위 변형시 loss-less (ex, s -> ms  OK)
			// 작은 단위 -> 큰 단위 변형시 Data-loss (ex, ms -> s  NG) =====>>> 이때에는 duration_cast<> 사용.
			using namespace std::chrono;
			auto si = duration_cast<milliseconds>(ts);
			//milliseconds{ts}.count();	// 작은단위(nano) -> 큰단위(milli) 이므로 안됨

		}


	}

#pragma pack(pop)
}	// namespace gtl
