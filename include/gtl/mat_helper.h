/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MatHelper.h : cv::Mat, CDC,...
//
// PWH.
//
// 2021.06.11 from Mocha xMathUtil
// 2021.06.29 from win_util
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gtl/string.h"
#include "gtl/coord.h"
#include "gtl/archive.h"
#include "gtl/misc.h"
#include "opencv2/opencv.hpp"

namespace gtl {
#pragma pack(push, 8)	// default align. (8 bytes)


	//-----------------------------------------------------------------------------
	// Mat Load/Store
#if 0
	template < class ARCHIVE >
	bool LoadMat(ARCHIVE& ar, cv::Mat& mat) {
		gtl::xStringA str;
		ar >> str;
		if (str != "mat")
			return false;

		int rows, cols, type;
		ar >> rows >> cols >> type;
		if ((rows < 0) || (cols < 0))
			return false;
		if ((rows == 0) || (cols == 0)) {
			mat.release();
			return true;
		}
		mat = cv::Mat::zeros(rows, cols, type);
		if (mat.empty())
			return false;
		for (int i = 0; i < rows; i++) {
			ar.read((char*)mat.ptr(i), (UINT)(mat.cols * mat.elemSize()));	// step -> (cols*elemSize())
		}
		return true;
	}
	template < class ARCHIVE >
	bool StoreMat(ARCHIVE& ar, cv::Mat const& mat) {
		gtl::xStringA str("mat");
		ar << str;

		ar << mat.rows;
		ar << mat.cols;
		ar << mat.type();
		for (int i = 0; i < mat.rows; i++) {
			ar.write((char*)mat.ptr(i), mat.cols * mat.elemSize());	// step -> (cols*elemSize())
		}

		return true;
	}
#endif

	static inline bool ReadMat(std::istream& is, cv::Mat& mat) {
		auto ReadVar = [&is, &mat](auto& var) -> bool {
			return (bool)is.read((char*)&var, sizeof(var));
		};

		do {
			// read sign
			{
				uint8_t buf[3];
				uint8_t b = (uint8_t)sizeof(buf);
				if (!ReadVar(b))
					break;
				if (b != sizeof(buf))
					break;
				if (!ReadVar(buf))
					break;
				if (memcmp(buf, "mat", b) != 0)
					break;
			}

			int rows{}, cols{}, type{};
			if (!ReadVar(rows)) break;
			if (!ReadVar(cols)) break;
			if (!ReadVar(type)) break;

			if ( (rows < 0) || (cols < 0) )
				break;
			if ( (rows == 0) || (cols == 0) ) {
				mat.release();
				return true;
			}

			mat = cv::Mat::zeros(rows, cols, type);
			if (mat.empty())
				break;

			for (int i = 0; i < rows; i++)
				is.read((char*)mat.ptr(i), (mat.cols*mat.elemSize()));	// step -> (cols*elemSize())

			return true;

		} while (false);

		return false;
	}
	static inline bool SaveMat(std::ostream& os, cv::Mat const& mat) {
		uint8_t buf[4] = { 3, 'm', 'a', 't' };
		os.write((char const*)buf, sizeof(buf));
		os.write((char const*)&mat.rows, sizeof(mat.rows));
		os.write((char const*)&mat.cols, sizeof(mat.cols));
		int type = mat.type();
		os.write((char const*)&type, sizeof(type));
		for (int i = 0; i < mat.rows; i++) {
			os.write((char*)mat.ptr(i), mat.cols*mat.elemSize());	// step -> (cols*elemSize())
		}

		return true;
	}


#if 0	// using boost
	template < typename TData > requires (sizeof(TData) == 1)
		std::string EncodeBase64(std::span<TData> data) {
		using namespace boost::archive::iterators;
		using It = base64_from_binary<transform_width<std::span<TData>::iterator, 6, 8>>;
		auto tmp = std::string(It(std::begin(data)), It(std::end(data)));
		return tmp.append((3 - data.size() % 3) % 3, '=');
	}

	template < typename TString, typename TOutpytIterator >
	void DecodeBase64(TString const& str, TOutpytIterator iterOutput) {
		using namespace boost::archive::iterators;
		using str2bin = transform_width<binary_from_base64<TString::const_iterator>, 8, 6>;
		std::copy(str2bin(str.begin()), str2bin(str.end()), iterOutput);
	}

	template < typename archive >
	void SyncMatBase64(archive& ar, cv::Mat& mat) {
		if constexpr (archive::is_saving::value) {
			ar & mat.cols;
			ar & mat.rows;
			ar & mat.type();
			if (mat.cols and mat.rows) {
				if (mat.isContinuous()) {
					ar & EncodeBase64(std::span(mat.datastart, mat.dataend));
				} else {
					std::string str;
					str.reserve((mat.cols * mat.rows * mat.elemSize() +1 ) * 8 / 6);
					size_t len = mat.cols*mat.elemSize();
					for (int y {}; y < mat.rows; y++) {
						auto const* p = mat.ptr(y);
						str += EncodeBase64(std::span(p, len));
					}
					ar & str;
				}
			}
		} else if constexpr (archive::is_loading::value) {
			int rows{};
			int cols{};
			int type{};

			ar & cols;
			ar & rows;
			ar & type;
			if (!rows or !cols) {
				return;
			}
			std::string str;
			ar & str;
			mat = cv::Mat::zeros(rows, cols, type);
			DecodeBase64(str, mat.begin<uchar>());
		}
	}
#endif

	GTL__API bool IsMatEqual(cv::Mat const& a, cv::Mat const& b);


	//-----------------------------------------------------------------------------
	// 외곽선 글자
	GTL__API void putTextC(cv::InputOutputArray img, std::string const& str, xPoint2i org,
		int fontFace, double fontScale, cv::Scalar color,
		int thickness = 1, int lineType = 8,
		bool bottomLeftOrigin = false, bool bOutline = true);


	//-----------------------------------------------------------------------------
	// Mat 
	GTL__API bool CheckGPU(bool bUse);
	GTL__API bool IsGPUEnabled();
	GTL__API bool ConvertColor(cv::Mat const& imgSrc, cv::Mat& imgDest, int eCode);
	GTL__API bool ResizeImage(cv::Mat const& imgSrc, cv::Mat& imgDest, double dScale, int eInterpolation = cv::INTER_LINEAR/*, int eScaleDownMethod = cv::INTER_AREA*/);
	GTL__API bool MatchTemplate(cv::Mat const& img, cv::Mat const& imgTempl, cv::Mat& matResult, int method);
	GTL__API bool MatchTemplate(cv::Mat const& img, cv::Mat const& imgTempl, int method, xPoint2d& ptBest, double& dMinMax, double& dRate, double dScale = 0.0, int eInterpolation = cv::INTER_LINEAR);

	//-----------------------------------------------------------------------------
	// Mat to DC
	template < typename TRECT >
	bool CalcViewPosition(cv::Size const& sizeView, TRECT const& rectView, TRECT& rectImage/* out */, TRECT& rectDst/* out */)	// image size -> display rect and source rect
	{
		if (rectView.IsRectEmpty()) {
			rectImage.SetRectEmpty();
			rectDst.SetRectEmpty();
			return false;
		}
		auto wDest = rectView.Width();
		auto hDest = rectView.Height();

		if (wDest >= sizeView.width) {
			rectDst.left = rectView.left + (wDest - sizeView.width) / 2;
			rectDst.right = rectDst.left + sizeView.width;
			rectImage.left = 0;
			rectImage.right = sizeView.width;
		}
		else if (wDest < sizeView.width) {
			rectDst.left = rectView.left;
			rectDst.right = rectView.right;
			rectImage.left = (sizeView.width - wDest) / 2;
			rectImage.right = rectImage.left + wDest;
		}

		if (hDest >= sizeView.height) {
			rectDst.top = rectView.top + (hDest - sizeView.height) / 2;
			rectDst.bottom = rectDst.top + sizeView.height;
			rectImage.top = 0;
			rectImage.bottom = sizeView.height;
		}
		else if (hDest < sizeView.height) {
			rectDst.top = rectView.top;
			rectDst.bottom = rectView.bottom;
			rectImage.top = (sizeView.height - hDest) / 2;
			rectImage.bottom = rectImage.top + hDest;
		}

		return true;

	}
	GTL__API bool MatToMatTransparent(cv::Mat const& imgSource, cv::Mat& imgTarget, cv::Mat const& matMask);
	GTL__API bool MatToMatTransparent(cv::Mat const& imgSource, cv::Mat& imgTarget, cv::Scalar const& crTransparent);
	//GTL__API cv::Mat LoadImageMat(std::filesystem::path const& path);



#pragma pack(push, 1)
	struct BMP_FILE_HEADER {
		char sign[2]{ 'B', 'M' };
		uint32_t sizeFile{};
		uint16_t reserved1{};
		uint16_t reserved2{};
		uint32_t offsetData{};
	};
	struct BITMAP_HEADER {
		uint32_t		size;
		int32_t			width;
		int32_t			height;
		uint16_t		planes;
		uint16_t		nBPP;
		uint32_t		compression;
		uint32_t		sizeImage;
		int32_t			XPelsPerMeter;
		int32_t			YPelsPerMeter;
		uint32_t		nColorUsed;
		uint32_t		nColorImportant;
	};

	using float_fix_point_2_30_t = int32_t;
	struct cie_xyz_t {
		float_fix_point_2_30_t x, y, z;
	};
	struct cie_xyz_rgb {
		cie_xyz_t r, g, b;
	};

	struct BITMAP_V4_HEADER {
		uint32_t		size;
		int32_t			width;
		int32_t			height;
		uint16_t		planes;
		uint16_t		nBPP;
		uint32_t		compression;
		uint32_t		sizeImage;
		int32_t			XPelsPerMeter;
		int32_t			YPelsPerMeter;
		uint32_t		nColorUsed;
		uint32_t		nColorImportant;
		uint32_t		maskRed;
		uint32_t		maskGreen;
		uint32_t		maskBlue;
		uint32_t		maskAlpha;
		uint32_t		cstype;
		cie_xyz_rgb		endpoints;
		uint32_t		gammaRed;
		uint32_t		gammaGreen;
		uint32_t		gammaBlue;
	};

	struct BITMAP_V5_HEADER {
		uint32_t		size;
		uint32_t		width;
		uint32_t		height;
		uint16_t		planes;
		uint16_t		nBPP;
		uint32_t		compression;
		uint32_t		sizeImage;
		uint32_t		XPelsPerMeter;
		uint32_t		YPelsPerMeter;
		uint32_t		nColorUsed;
		uint32_t		nColorImportant;
		uint32_t		maskRed;
		uint32_t		maskGreen;
		uint32_t		maskBlue;
		uint32_t		maskAlpha;
		uint32_t		cstype;
		cie_xyz_rgb		endpoints;
		uint32_t		gammaRed;
		uint32_t		gammaGreen;
		uint32_t		gammaBlue;
		uint32_t		intent;
		uint32_t		profileData;
		uint32_t		profileSize;
		uint32_t		reserved;
	};
	using variant_BITMAP_HEADER = std::variant<BITMAP_HEADER, BITMAP_V4_HEADER, BITMAP_V5_HEADER>;
#pragma pack(pop)

	/// @brief Save Image to BITMAP. Image is COLOR or GRAY level image.
	/// @param path 
	/// @param img : CV_8UC1 : gray scale, CV_8UC3 : color (no palette supported), for CV8UC3, palette is not used.
	/// @param nBPP 
	/// @param palette 
	/// @param bPixelIndex if true, img value is NOT a pixel but a palette index. a full palette must be given.
	/// @return 
	GTL__API bool SaveBitmapMat(std::filesystem::path const& path, cv::Mat const& img, int nBPP, gtl::xSize2i const& pelsPerMeter, std::span<gtl::color_bgra_t> palette = {}, bool bPixelIndex = false, bool bBottom2Top = false, callback_progress_t funcCallback = nullptr);

	GTL__API bool LoadBitmapHeader(std::filesystem::path const& path, BMP_FILE_HEADER& fileHeader, BITMAP_V5_HEADER& header);
	GTL__API bool LoadBitmapHeader(std::istream& is, BMP_FILE_HEADER& fileHeader, variant_BITMAP_HEADER& header);
	GTL__API bool LoadBitmapHeader(std::filesystem::path const& path, BMP_FILE_HEADER& fileHeader, variant_BITMAP_HEADER& header);
	GTL__API cv::Mat LoadBitmapMat(std::filesystem::path const& path, gtl::xSize2i& pelsPerMeter, callback_progress_t funcCallback = nullptr);
	GTL__API cv::Mat LoadBitmapMatPixelArray(std::filesystem::path const& path, gtl::xSize2i& pelsPerMeter, std::vector<gtl::color_bgra_t>& palette, callback_progress_t funcCallback = nullptr);


	inline cv::Mat LoadImageMat(std::filesystem::path const& path) {
		cv::Mat img;
		if (auto buf = FileToContainer<std::vector<uint8_t>>(path); buf) {
			try {
				img = cv::imdecode(*buf, -1);
			}
			catch (...) {
			}
		}
		return img;
	}

	GTL__API bool CopyMatToXY(cv::Mat const& src, cv::Mat& dest, gtl::xPoint2i ptDestTopLeft, cv::Mat const* pMask = nullptr);


	inline bool IsImageExtension(std::filesystem::path const& path) {
		auto ext = gtl::ToLower<char>(path.extension().string());
		return gtl::IsValueOneOf(ext, ".bmp", ".jpg", ".jpeg", ".tiff", ".png", ".gif", ".jfif");
	}



#pragma pack(pop)
}
