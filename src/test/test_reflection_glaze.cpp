#include "pch.h"

#include "gtl/gtl.h"
#include "gtl/mat_helper.h"
#include "gtl/reflection_glaze.h"

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

namespace stdfs = std::filesystem;
namespace stdc = std::chrono;

namespace gtl::test::reflection_glaze {

	struct topmost {
		cv::Mat m;

		bool operator == (topmost const& B) const {
			return IsMatEqual(m, B.m);
		};
	};

	struct parent : topmost {
		using this_t = parent;

		std::string str;
		std::u8string stru8 = u8"가";
		std::wstring strw = L"나";
		std::u16string stru16 = u"다";
		std::u32string stru32 = U"라";
		gtl::mm_t len{10_um};
		gtl::deg_t td1{90_deg};
		gtl::deg_t td2{3.14159265358979323846264_rad/2.};
		gtl::rad_t tr1{90_deg};
		gtl::rad_t tr2{3.14159265358979323846264_rad/2.};
		int dd{};
		double dValue{};
		int iValue{};
		bool on_off{};
		std::vector<int> values{1,2,3,4,5};

		//cv::Mat m;

		auto operator <=> (this_t const& ) const = default;

		GLZ_LOCAL_META(this_t, dd, str, dValue, iValue, on_off, values, m, stru8, strw, stru16, stru32, len, td1, td2, tr1, tr2);
		//struct glaze {
		//	static constexpr auto value = glz::object(
		//		"str", &this_t::str,
		//		"dValue", &this_t::dValue,
		//		"iValue", &this_t::iValue,
		//		"on_off", &this_t::on_off
		//	);

		//};	//GLZ_LOCAL_META(derived, str2);

	};

	struct derived1 : parent {
		using this_t = derived1;
		using base_t = parent;

		std::string a;

		auto operator <=> (this_t const& ) const = default;

		GLZ_LOCAL_META_DERIVED(derived1, parent, a);
	};


	struct derived2 : derived1 {
		using this_t = derived2;
		using base_t = derived1;

		std::string str2;
		gtl::xPoint3d pt {0, 0};
		gtl::xRect3d rc { 0, 0, 0, 0};
		gtl::xCoordTrans3d ct{0, {1, 0, 0, 0, 1, 0, 0, 0, 1}, {0, 0}, {0, 0}};

		auto operator <=> (this_t const& ) const = default;

		GLZ_LOCAL_META_DERIVED(this_t, base_t, str2, pt, rc, ct);
	};

	struct second {
		using this_t = second;

		std::vector<derived2> d{{}, {}, {}, {}};

		auto operator <=> (this_t const& ) const = default;

		GLZ_LOCAL_META(second, d);
	};

	TEST(test, reflection_glaze_1) {
		constexpr glz::opts op{.error_on_unknown_keys = false, .prettify=true};

		stdfs::path folder("./reflection_test");
		stdfs::create_directories(folder);

		std::string buf;

		derived2 d;
		d.str = "abc";
		d.iValue = 3;
		d.a = "a";
		d.str2 = "def";
		d.m = cv::Mat::eye(3, 3, CV_32FC1);

		glz::write<op>(d, buf);
		gtl::ContainerToFile(buf, folder / "1.json");

		derived2 d2;
		auto r = glz::read<op>(d2, buf);

		EXPECT_EQ((topmost&)d, (topmost&)d2);
		EXPECT_EQ((parent&)d, (parent&)d2);
		EXPECT_EQ((derived1&)d, (derived1&)d2);
		EXPECT_EQ((derived2&)d, (derived2&)d2);

		//r = glz::read<op>(d, buf);
		//EXPECT_EQ(d, d2);

		// 2.json
		{
			second s;
			s.d[0].str2 = "ghi";
			glz::write_json(s, buf);
			gtl::ContainerToFile(std::span(buf), folder / "2.json");
		}
		if (auto r = gtl::FileToContainer<std::string>(folder / "2.json")) {
			second d;
			//std::string str(r->begin(), r->end());
			//glz::read_json((parent&)d, *r);
			//glz::read_json(d, *r);
			//second s;
			//glz::read_json(s, *r);

			glz::context ctx{};
			glz::read<op>(d, *r, ctx);

			EXPECT_EQ(d.d[0].str2, "ghi"s);
		}


		// 3.json
		cv::Mat m(3, 3, CV_32FC1);
		for (int y{}; y < m.rows; y++) {
			float* ptr = m.ptr<float>(y);
			auto f = y * m.cols;
			for (int x{}; x < m.cols; x++) {
				ptr[x] = f + x;
			}
		}
		glz::write_json(m, buf);
		gtl::ContainerToFile(std::span(buf), folder / "3.json");
		if (auto r = gtl::FileToContainer<std::string>(folder / "3.json")) {
			cv::Mat m;
			glz::read_json(m, *r);

			for (int y{}; y < m.rows; y++) {
				float* ptr = m.ptr<float>(y);
				for (int x{}; x < m.cols; x++) {
					EXPECT_EQ(ptr[x], y * m.cols + x);
				}
			}
			//second s;
			//glz::read_json(s, *r);
		}

		auto TestVariousSize = [&](int rows, int cols, stdfs::path const& path) {
			{
				std::string str;
				cv::Mat m(rows, cols, CV_8UC1);
				for (int y{}, i{127}; y < m.rows; y++) {
					uchar* ptr = m.ptr<uchar>(y);
					for (int x{}; x < m.cols; x++) {
						ptr[x] = i++;
					}
				}
				glz::write<op>(m, str);
				gtl::ContainerToFile(str, path);
			}
			if (auto str = gtl::FileToContainer<std::string>(path); str) {
				cv::Mat m;
				glz::read<op>(m, *str);
				for (int y{}, i{127}; y < m.rows; y++) {
					uchar* ptr = m.ptr<uchar>(y);
					for (int x{}; x < m.cols; x++) {
						EXPECT_EQ(ptr[x], i++);
					}
				}
			}
		};
		TestVariousSize(1,  9, folder / "mat01_09.json");
		TestVariousSize(1, 10, folder / "mat01_10.json");
		TestVariousSize(1, 11, folder / "mat01_11.json");
		TestVariousSize(1, 12, folder / "mat01_12.json");
		TestVariousSize(1, 13, folder / "mat01_13.json");
	}

}
