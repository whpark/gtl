#include "pch.h"

#include "gtl/gtl.h"

#include "boost/archive/iterators/binary_from_base64.hpp"
#include "boost/archive/iterators/base64_from_binary.hpp"
#include "boost/archive/iterators/transform_width.hpp"
#include "boost/algorithm/string.hpp"

#include "opencv2/opencv.hpp"


import gtl;

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

namespace gtl::test::misc {
	using namespace gtl;

	class xTestVirtualBase {
	public:
		std::vector<int> data;

		//xTestVirtualBase() = default;
		//xTestVirtualBase(xTestVirtualBase const&) = default;
		//xTestVirtualBase(xTestVirtualBase &&) = default;

		//virtual ~xTestVirtualBase() = default;
	};

	class xTestVirtualDerived : public xTestVirtualBase {
	public:
		std::vector<int> data2;

		virtual ~xTestVirtualDerived() = default;
	};

	void Func1(int, int, double) {}
	void Func1(int, double, int) {}


	TEST(misc, xFinalAction) {
		int i{};
		{
			xFinalAction fa{[&]{ i = 3;} };

		}
		EXPECT_EQ(i, 3);


		Func1(4, 5, 6);

		xTestVirtualBase ob1;
		ob1.data.push_back(1);
		ob1.data.push_back(2);
		ob1.data.push_back(3);
		auto ob2 = std::move(ob1);
		auto ob3 = ob1;

		EXPECT_EQ(ob1.data.size(), 0);
		EXPECT_EQ(ob2.data.size(), 3);

	}


//	template < typename TData > requires (sizeof(TData) == 1)
//	std::string EncodeBase64(std::span<TData> data) {
//		using namespace boost::archive::iterators;
//		using It = base64_from_binary<transform_width<std::span<TData>::iterator, 6, 8>>;
//		auto tmp = std::string(It(std::begin(data)), It(std::end(data)));
//		return tmp.append((3 - data.size() % 3) % 3, '=');
//	}
//
//	template < typename TString, typename TOutpytIterator >
//	void DecodeBase64(TString const& str, TOutpytIterator iterOutput) {
//		using namespace boost::archive::iterators;
//		using str2bin = transform_width<binary_from_base64<TString::const_iterator>, 8, 6>;
//		std::copy(str2bin(str.begin()), str2bin(str.end()), iterOutput);
//	}
//
//#if 0
//	template < typename archive >
//	void SyncMatBase64(archive& ar, cv::Mat& mat) {
//		if constexpr (archive::is_saving::value) {
//			ar & mat.cols;
//			ar & mat.rows;
//			ar & mat.type();
//			if (mat.cols and mat.rows) {
//				if (mat.isContinuous()) {
//					ar & EncodeBase64(std::span(mat.datastart, mat.dataend));
//				} else {
//					std::string str;
//					str.reserve(mat.cols * mat.rows * mat.elemSize() +1 )* 8 / 6;
//					size_t len = mat.cols*mat.elemSize();
//					for (int y {}; y < mat.rows; y++) {
//						auto const* p = mat.ptr(y);
//						str += EncodeBase64(std::span(p, len));
//					}
//					ar & str;
//				}
//			}
//		} else if constexpr (archive::is_loading::value) {
//			int rows{};
//			int cols{};
//			int type{};
//
//			ar & cols;
//			ar & rows;
//			ar & type;
//			if (!rows or !cols) {
//				return;
//			}
//			std::string str;
//			ar & str;
//			mat = cv::Mat::zeros(rows, cols, type);
//			DecodeBase64(str, mat.begin<uchar>());
//		}
//	}
//
//	TEST(misc, mat_json) {
//		cv::Mat mat = cv::Mat::eye(100, 100, CV_8UC1);
//
//		auto str = EncodeBase64(std::span(mat.datastart, mat.dataend));
//		cv::Mat mat2(100, 100, CV_8UC1);
//		DecodeBase64(mat2.begin<uchar>(), str);
//
//		EXPECT_EQ(mat, mat2);
//
//	}
}

