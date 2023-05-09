#pragma once

#include "opencv2/opencv.hpp"

#include "glaze/glaze.hpp"
//#include "glaze/core/macros.hpp"

// encode / decode base64
#include "boost/archive/iterators/binary_from_base64.hpp"
#include "boost/archive/iterators/base64_from_binary.hpp"
#include "boost/archive/iterators/transform_width.hpp"


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


// for cv::Mat
namespace gtl {
#pragma pack(push, 8)

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

		template < typename T = uint8_t > requires (sizeof(T) == 1)
		static std::string EncodeBase64(std::span<T> data) {
			using namespace boost::archive::iterators;
			using It = base64_from_binary<transform_width< typename std::span<T>::iterator, 6, 8>>;
			auto tmp = std::string(It(std::begin(data)), It(std::end(data)));
			return tmp.append((3 - data.size() % 3) % 3, '=');
		}

		template < typename TString, typename TOutpytIterator >
		static void DecodeBase64(TString const& str, TOutpytIterator iterOutput, size_t max_size) {
			using namespace boost::archive::iterators;
			using str2bin = transform_width<binary_from_base64<typename TString::const_iterator>, 8, 6>;
			//for (auto i = str2bin(str.begin()), e = str2bin(str.end()); max_size-- > 0 && i != e; ++i) {
			//	*iterOutput++ = *i;
			//}
			auto e = std::min(str.begin()+std::min(str.size(), max_size*8/6), str.end());
			std::copy(str2bin(str.begin()), str2bin(e), iterOutput);
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

#pragma pack(pop)
}


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

