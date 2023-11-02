﻿#pragma once

#include "gtl/coord.h"
#include "gtl/base64.h"
#include "gtl/unit.h"

#include "opencv2/opencv.hpp"

#include "glaze/glaze.hpp"
#include "glaze/core/macros.hpp"


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


template <>
struct glz::detail::to_json<cv::Mat> {
	template <auto Opts, is_context Ctx, class B, class IX>
	inline static void op(auto&& value, Ctx&& ctx, B&& b, IX&& ix) {
		to_json<gtl::cvMat>::op<Opts>(gtl::cvMat(value), std::forward<Ctx>(ctx), std::forward<B>(b), std::forward<IX>(ix));
	}
};

template <>
struct glz::detail::from_json<cv::Mat> {
	template <auto Opts, is_context Ctx, class B, class IX>
	inline static void op(auto&& value, Ctx&& ctx, B&& b, IX&& ix) {
		gtl::cvMat m;
		from_json<gtl::cvMat>::op<Opts>(m, std::forward<Ctx>(ctx), std::forward<B>(b), std::forward<IX>(ix));
		value = m;
	}
};

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

// std::basic_string
// std::u8string
template <>
struct glz::detail::to_json<std::u8string> {
	template <auto Opts, is_context Ctx, class B, class IX>
	inline static void op(auto&& value, Ctx&& ctx, B&& b, IX&& ix) {
		to_json<std::string>::op<Opts>((std::string const&)value, std::forward<Ctx>(ctx), std::forward<B>(b), std::forward<IX>(ix));
	}
};

template <>
struct glz::detail::from_json<std::u8string> {
	template <auto Opts, is_context Ctx, class B, class IX>
	GLZ_ALWAYS_INLINE static void op(auto&& value, Ctx&& ctx, B&& b, IX&& ix) {
		from_json<std::string>::op<Opts>((std::string&)value, std::forward<Ctx>(ctx), std::forward<B>(b), std::forward<IX>(ix));
	}
};


// std::wstring
template <>
struct glz::detail::to_json<std::wstring> {
	template <auto Opts, is_context Ctx, class B, class IX>
	inline static void op(auto&& value, Ctx&& ctx, B&& b, IX&& ix) {
		to_json<std::string>::op<Opts>(gtl::WtoU8A(value), std::forward<Ctx>(ctx), std::forward<B>(b), std::forward<IX>(ix));
	}
};

template <>
struct glz::detail::from_json<std::wstring> {
	template <auto Opts, is_context Ctx, class B, class IX>
	GLZ_ALWAYS_INLINE static void op(auto&& value, Ctx&& ctx, B&& b, IX&& ix) {
		std::string str;
		from_json<std::string>::op<Opts>(str, std::forward<Ctx>(ctx), std::forward<B>(b), std::forward<IX>(ix));
		value = gtl::U8toW(str);
	}
};


// std::u16string
template <>
struct glz::detail::to_json<std::u16string> {
	template <auto Opts, is_context Ctx, class B, class IX>
	inline static void op(auto&& value, Ctx&& ctx, B&& b, IX&& ix) {
		to_json<std::wstring>::op<Opts>((std::wstring&)value, std::forward<Ctx>(ctx), std::forward<B>(b), std::forward<IX>(ix));
	}
};

template <>
struct glz::detail::from_json<std::u16string> {
	template <auto Opts, is_context Ctx, class B, class IX>
	GLZ_ALWAYS_INLINE static void op(auto&& value, Ctx&& ctx, B&& b, IX&& ix) {
		from_json<std::wstring>::op<Opts>((std::wstring&)value, std::forward<Ctx>(ctx), std::forward<B>(b), std::forward<IX>(ix));
	}
};


// std::u32string
template <>
struct glz::detail::to_json<std::u32string> {
	template <auto Opts, is_context Ctx, class B, class IX>
	inline static void op(auto&& value, Ctx&& ctx, B&& b, IX&& ix) {
		to_json<std::u8string>::op<Opts>(gtl::ToStringU8(value), std::forward<Ctx>(ctx), std::forward<B>(b), std::forward<IX>(ix));
	}
};

template <>
struct glz::detail::from_json<std::u32string> {
	template <auto Opts, is_context Ctx, class B, class IX>
	GLZ_ALWAYS_INLINE static void op(auto&& value, Ctx&& ctx, B&& b, IX&& ix) {
		std::u8string str;
		from_json<std::u8string>::op<Opts>(str, std::forward<Ctx>(ctx), std::forward<B>(b), std::forward<IX>(ix));
		value = gtl::ToStringU32(str);
	}
};


// std::filesystem::path
template <>
struct glz::detail::to_json<std::filesystem::path> {
	template <auto Opts, is_context Ctx, class B, class IX>
	inline static void op(auto&& value, Ctx&& ctx, B&& b, IX&& ix) {
		to_json<std::wstring>::op<Opts>(value.wstring(), std::forward<Ctx>(ctx), std::forward<B>(b), std::forward<IX>(ix));
	}
};

template <>
struct glz::detail::from_json<std::filesystem::path> {
	template <auto Opts, is_context Ctx, class B, class IX>
	GLZ_ALWAYS_INLINE static void op(auto&& value, Ctx&& ctx, B&& b, IX&& ix) {
		std::wstring str;
		from_json<std::wstring>::op<Opts>(str, std::forward<Ctx>(ctx), std::forward<B>(b), std::forward<IX>(ix));
		value = str;
	}
};

// std::chrono::duration
template <class _Rep, class _Period>
struct glz::detail::to_json<std::chrono::duration<_Rep, _Period>> {
	template <auto Opts, is_context Ctx, class B, class IX>
	inline static void op(std::chrono::duration<_Rep, _Period> const& value, Ctx&& ctx, B&& b, IX&& ix) {
		to_json<_Rep>::op<Opts>(value.count(), std::forward<Ctx>(ctx), std::forward<B>(b), std::forward<IX>(ix));
	}
};

template <class _Rep, class _Period>
struct glz::detail::from_json<std::chrono::duration<_Rep, _Period>> {
	template <auto Opts, is_context Ctx, class B, class IX>
	GLZ_ALWAYS_INLINE static void op(std::chrono::duration<_Rep, _Period>& value, Ctx&& ctx, B&& b, IX&& ix) {
		_Rep rep;
		from_json<_Rep>::op<Opts>(rep, std::forward<Ctx>(ctx), std::forward<B>(b), std::forward<IX>(ix));
		value = std::chrono::duration<_Rep, _Period>(rep);
	}
};

// std::chrono::time_point
template <class _Clock, class _Duration>
struct glz::detail::to_json<std::chrono::time_point<_Clock, _Duration>> {
	template <auto Opts, is_context Ctx, class B, class IX>
	inline static void op(std::chrono::time_point<_Clock, _Duration> const& value, Ctx&& ctx, B&& b, IX&& ix) {
		to_json<_Duration>::op<Opts>(value.time_since_epoch(), std::forward<Ctx>(ctx), std::forward<B>(b), std::forward<IX>(ix));
	}
};

template <class _Clock, class _Duration>
struct glz::detail::from_json<std::chrono::time_point<_Clock, _Duration>> {
	template <auto Opts, is_context Ctx, class B, class IX>
	GLZ_ALWAYS_INLINE static void op(std::chrono::time_point<_Clock, _Duration>& value, Ctx&& ctx, B&& b, IX&& ix) {
		_Duration duration;
		from_json<_Duration>::op<Opts>(duration, std::forward<Ctx>(ctx), std::forward<B>(b), std::forward<IX>(ix));
		value = std::chrono::time_point<_Clock, _Duration>(duration);
	}
};
