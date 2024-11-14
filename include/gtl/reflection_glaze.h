#pragma once

#include "gtl/coord.h"
#include "gtl/base64.h"
#include "gtl/unit.h"

#include "opencv2/opencv.hpp"

#include "glaze/glaze.hpp"
#include "glaze/core/opts.hpp"
#include "glaze/core/custom.hpp"
//#include "glaze/core/macros.hpp"

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//<< from glaze/core/macros.h          [Glaze 1.3.2]
// Glaze Library
// For the license information refer to glaze.hpp

#pragma once

// utility macros

// https://www.scs.stanford.edu/~dm/blog/va-opt.html

#define GLZ_PARENS ()

//#define GLZ_EXPAND(...) GLZ_EXPAND4(GLZ_EXPAND4(GLZ_EXPAND4(GLZ_EXPAND4(__VA_ARGS__))))
//#define GLZ_EXPAND4(...) GLZ_EXPAND3(GLZ_EXPAND3(GLZ_EXPAND3(GLZ_EXPAND3(__VA_ARGS__))))
//#define GLZ_EXPAND3(...) GLZ_EXPAND2(GLZ_EXPAND2(GLZ_EXPAND2(GLZ_EXPAND2(__VA_ARGS__))))
//#define GLZ_EXPAND2(...) GLZ_EXPAND1(GLZ_EXPAND1(GLZ_EXPAND1(GLZ_EXPAND1(__VA_ARGS__))))
//#define GLZ_EXPAND1(...) __VA_ARGS__

#define GLZ_FOR_EACH(macro, ...) __VA_OPT__(GLZ_EXPAND(GLZ_FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define GLZ_FOR_EACH_HELPER(macro, a, ...) \
   macro(a) __VA_OPT__(, ) __VA_OPT__(GLZ_FOR_EACH_AGAIN GLZ_PARENS(macro, __VA_ARGS__))
#define GLZ_FOR_EACH_AGAIN() GLZ_FOR_EACH_HELPER

// Glaze specific macros

#define GLZ_X(a) #a, &T::a
#define GLZ_QUOTED_X(a) #a, glz::quoted < &T::a>()

#define GLZ_META(C, ...)                                                      \
   template <>                                                                \
   struct glz::meta<C>                                                        \
   {                                                                          \
      using T = C;                                                            \
      static constexpr auto value = object(GLZ_FOR_EACH(GLZ_X, __VA_ARGS__)); \
   }

#define GLZ_LOCAL_META(C, ...)                                                     \
   struct glaze                                                                    \
   {                                                                               \
      using T = C;                                                                 \
      static constexpr auto value = glz::object(GLZ_FOR_EACH(GLZ_X, __VA_ARGS__)); \
   }

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>



// macro for derived classes
#define GLZ_META_DERIVED(C, parent,...)                              \
   template <>                                                       \
   struct glz::meta<C>                                               \
   {                                                                 \
      using T = C;                                                   \
      static constexpr auto value = object(                          \
        #parent, [](auto&& self) -> auto& { return (parent&)self; }, \
        GLZ_FOR_EACH(GLZ_X, __VA_ARGS__));                           \
   }

#define GLZ_LOCAL_META_DERIVED(C, parent, ...)                       \
   struct glaze                                                      \
   {                                                                 \
      using T = C;                                                   \
      static constexpr auto value = glz::object(                     \
        #parent, [](auto&& self) -> auto& { return (parent&)self; }, \
        GLZ_FOR_EACH(GLZ_X, __VA_ARGS__));                           \
   }


namespace gtl {
#pragma pack(push, 8)

	template <typename T>
	decltype(auto) ValueOr(glz::json_t const& j, T const& def) {
		if (auto* v = j.get_if<T>()) {
			return *v;
		}
		return def;
	};


	// for cv::Mat

	struct cvMat {
		int rows{};
		int cols{};
		int type{};
		std::vector<std::string> data{};

		cvMat() = default;
		cvMat(cvMat const&) = default;
		cvMat(cvMat&&) = default;
		cvMat& operator=(cvMat const&) = default;
		cvMat& operator=(cvMat&&) = default;

		cvMat(cv::Mat m) {
			rows = m.rows;
			cols = m.cols;
			type = m.type();
			data.reserve(m.rows);
			size_t const step = m.step;
			for (int y{}; y < m.rows; y++) {
				auto* ptr = m.ptr(y);
				// ptr to base64
				data.emplace_back(EncodeBase64(std::span(ptr, step)));
			}
		}

		operator cv::Mat() const {
			cv::Mat m = cv::Mat::zeros(rows, cols, type);
			size_t const step = m.step;
			for (int y{}; y < m.rows; y++) {
				auto* ptr = m.ptr(y);
				// base64 to ptr
				DecodeBase64(data[y], ptr, step);
			}
			return m;
		}

		struct glaze {
			static constexpr auto value = glz::object(
				"rows", &cvMat::rows,
				"cols", &cvMat::cols,
				"type", &cvMat::type,
				"data", &cvMat::data
			);
		};
	};

	// gtl::coord
	template < typename T, int dim >
	struct glz::meta<gtl::TSizeT<T, dim>> {
		static constexpr auto value = [](auto&& self) -> decltype(auto) { return self.arr(); };
	};

	template < typename T, int dim >
	struct glz::meta<gtl::TPointT<T, dim>> {
		static constexpr auto value = [](auto&& self) -> decltype(auto) { return self.arr(); };
	};

	template < typename T, int dim >
	struct glz::meta<gtl::TRectT<T, dim>> {
		static constexpr auto value = [](auto&& self) -> decltype(auto) { return self.arr(); };
	};

	template < typename T, int dim >
	struct glz::meta<gtl::TSRectT<T, dim>> {
		static constexpr auto value = [](auto&& self) -> decltype(auto) { return self.arr(); };
	};

	// gtl::xCoordTrans2d, xCorodTrans3d
	template < int dim >
	struct glz::meta<gtl::TCoordTransDim<dim>> {
		using T = gtl::TCoordTransDim<dim>;
		static constexpr auto value = object("scale", &T::m_scale, "mat", &T::m_mat, "origin", &T::m_origin, "offset", &T::m_offset);
	};

	// gtl::tlength_mm_t
	template < typename T >
	struct glz::meta<gtl::tlength_mm_t<T>> {
		static constexpr auto value = &gtl::tlength_mm_t<T>::dValue;
	};

	// gtl::tangle_deg_t
	template < std::floating_point tvalue_t >
	struct glz::meta<gtl::tangle_deg_t<tvalue_t>> {
		static constexpr auto value = &gtl::tangle_deg_t<tvalue_t>::dValue;
	};

	// gtl::tangle_rad_t
	template < std::floating_point tvalue_t >
	struct glz::meta<gtl::tangle_rad_t<tvalue_t>> {
		static constexpr auto value = &gtl::tangle_rad_t<tvalue_t>::dValue;
	};

#pragma pack(pop)
}	// namespace gtl;


namespace glz::detail {
	template <>
	struct from<JSON, cv::Mat> {
		template <auto Opts>
		static void op(auto&& value, auto&&... args) {
			gtl::cvMat m;
			read<JSON>::op<Opts>(m, args...);
			value = m;
		}
	};
	template <>
	struct to<JSON, cv::Mat> {
		template <auto Opts>
		static void op(auto&& value, auto&&... args) noexcept {
			write<JSON>::op<Opts>(gtl::cvMat(value), args...);
		}
	};
}

// cv::Matx<...>
template <typename _Tp, int m, int n>
struct glz::meta<cv::Matx<_Tp, m, n>> {
	using T = cv::Matx<_Tp, m, n>;
	using array_t = std::array<_Tp, sizeof(T::val)/sizeof(_Tp)>;
	static constexpr auto value = [](auto&& self)->auto&{ return *(array_t*)&self.val; };
};

// cv::Vec
template<typename _Tp, int cn>
struct glz::meta<cv::Vec<_Tp, cn>> {
	using T = cv::Vec<_Tp, cn>;
	using array_t = std::array<_Tp, sizeof(T::val)/sizeof(_Tp)>;
	static constexpr auto value = [](auto&& self)->auto&{ return *(array_t*)&self.val; };
};

// cv::Scalar_
template<typename _Tp>
struct glz::meta<cv::Scalar_<_Tp>> {
	using T = cv::Scalar_<_Tp>;
	using array_t = std::array<_Tp, sizeof(T::val)/sizeof(_Tp)>;
	static constexpr auto value = [](auto&& self)->auto&{ return *(array_t*)&self.val; };
};

namespace glz::detail {

	// std::u8string
	template <>
	struct from<JSON, std::u8string> {
		template <auto Opts>
		static void op(auto&& value, auto&&... args) {
			read<JSON>::op<Opts>((std::string&)value, args...);
		};
	};
	template <>
	struct to<JSON, std::u8string> {
		template <auto Opts>
		static void op(auto&& value, auto&&... args) noexcept {
			write<JSON>::op<Opts>((std::string&)value, args...);
		};
	};

	// std::wstring
	template <>
	struct from<JSON, std::wstring> {
		template <auto Opts>
		static void op(auto&& value, auto&&... args) {
			std::string str;
			read<JSON>::op<Opts>(str, args...);
			value = gtl::U8toW(str);
		};
	};
	template <>
	struct to<JSON, std::wstring> {
		template <auto Opts>
		static void op(auto&& value, auto&&... args) noexcept {
			auto str = gtl::WtoU8A(value);
			write<JSON>::op<Opts>(str, args...);
		};
	};

	// std::u16string
	template <>
	struct from<JSON, std::u16string> {
		template <auto Opts>
		static void op(auto&& value, auto&&... args) {
			std::string str;
			read<JSON>::op<Opts>(str, args...);
			value = gtl::ToStringU16((std::u8string&)str);
		};
	};
	template <>
	struct to<JSON, std::u16string> {
		template <auto Opts>
		static void op(auto& value, auto&&... args) noexcept {
			write<JSON>::op<Opts>(gtl::WtoU8A((std::wstring&)value), args...);
		};
	};

	// std::u32string
	template <>
	struct from<JSON, std::u32string> {
		template <auto Opts>
		static void op(auto&& value, auto&&... args) {
			std::string str;
			read<JSON>::op<Opts>(str, args...);
			value = gtl::ToStringU32((std::u8string&)str);
		};
	};
	template <>
	struct to<JSON, std::u32string> {
		template <auto Opts>
		static void op(auto&& value, auto&&... args) noexcept {
			write<JSON>::op<Opts>((std::string&)gtl::ToStringU8((std::wstring&)value), args...);
		};
	};

	// std::filesystem::path
	template <>
	struct from<JSON, std::filesystem::path> {
		template <auto Opts>
		static void op(auto&& value, auto&&... args) {
			std::wstring str;
			read<JSON>::op<Opts>(str, args ...);
			value = str;
		}
	};
	template <>
	struct to<JSON, std::filesystem::path> {
		template <auto Opts>
		static void op(auto&& value, auto&& ... args) noexcept {
			write<JSON>::op<Opts>(value.wstring(), args ...);
		}
	};

	// std::chrono::duration
	template <class tRep, class tPeriod>
	struct from<JSON, std::chrono::duration<tRep, tPeriod>> {
		template <auto Opts>
		static void op(auto&& value, auto&&... args) {
			tRep rep;
			read<JSON>::op<Opts>(rep, args...);
			value = std::chrono::duration<tRep, tPeriod>(rep);
		}
	};
	template <class tRep, class tPeriod>
	struct to<JSON, std::chrono::duration<tRep, tPeriod>> {
		template <auto Opts>
		static void op(auto&& value, auto&&... args) noexcept {
			write<JSON>::op<Opts>(value.count(), args...);
		}
	};

	// std::chrono::time_point
	template <class tClock, class tDuration>
	struct from<JSON, std::chrono::time_point<tClock, tDuration>> {
		template <auto Opts>
		static void op(auto&& value, auto&&... args) {
			tDuration duration;
			read<JSON>::op<Opts>(duration, args...);
			value = std::chrono::time_point<tClock, tDuration>(duration);
		}
	};
	template <class tClock, class tDuration>
	struct to<JSON, std::chrono::time_point<tClock, tDuration>> {
		template <auto Opts>
		static void op(auto&& value, auto&&... args) noexcept {
			write<JSON>::op<Opts>(value.time_since_epoch(), args...);
		}
	};

};


