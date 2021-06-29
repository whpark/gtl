/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MatHelper.h : cv::Mat, 
//
// PWH.
//
// 2021.06.11. from Mocha xMathUtil
// 2021.06.24. module
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

module;

#include <filesystem>
#include <span>
#include "opencv2/opencv.hpp"

export module gtl:mat_helper;
import :coord;
import :archive;

namespace gtl {
	bool s_bGPUChecked {};
	bool s_bUseGPU {};
}

export namespace gtl {

	//-----------------------------------------------------------------------------
	// Mat Load/Store
	bool ReadMat(std::istream& is, cv::Mat& mat) {
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
	bool SaveMat(std::ostream& os, cv::Mat const& mat) {
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


	//-----------------------------------------------------------------------------
	// ¿Ü°û¼± ±ÛÀÚ
	void putTextC(cv::InputOutputArray img, std::string const& str, CPoint2i org,
		int fontFace, double fontScale, cv::Scalar color,
		int thickness = 1, int lineType = 8,
		bool bottomLeftOrigin = false, bool bOutline = true)
	{
		CSize2i size { cv::getTextSize(str, fontFace, fontScale, thickness, nullptr) };

		if (bOutline) {
			cv::Scalar crBkgnd;
			crBkgnd = cv::Scalar(255, 255, 255) - color;
			//if (crBkgnd == Scalar(0, 0, 0))
			//	crBkgnd = Scalar(1, 1, 1);
			int iShift = std::max(1, thickness/2);
			cv::putText(img, str, org-size/2+CPoint2i(0, iShift), fontFace, fontScale, crBkgnd, thickness, lineType, bottomLeftOrigin);
			cv::putText(img, str, org-size/2+CPoint2i(iShift, 0), fontFace, fontScale, crBkgnd, thickness, lineType, bottomLeftOrigin);
			cv::putText(img, str, org-size/2+CPoint2i(-iShift, 0), fontFace, fontScale, crBkgnd, thickness, lineType, bottomLeftOrigin);
			cv::putText(img, str, org-size/2+CPoint2i(0, -iShift), fontFace, fontScale, crBkgnd, thickness, lineType, bottomLeftOrigin);
		}

		cv::putText(img, str, org-size/2, fontFace, fontScale, color, thickness, lineType, bottomLeftOrigin);
	}


	//-----------------------------------------------------------------------------
	// Mat 
	bool CheckGPU(bool bUse);
	bool IsGPUEnabled();
	bool ConvertColor(cv::Mat const& imgSrc, cv::Mat& imgDest, int eCode);
	bool ResizeImage(cv::Mat const& imgSrc, cv::Mat& imgDest, double dScale, int eInterpolation = cv::INTER_LINEAR/*, int eScaleDownMethod = cv::INTER_AREA*/);
	bool MatchTemplate(cv::Mat const& img, cv::Mat const& imgTempl, cv::Mat& matResult, int method);
	bool MatchTemplate(cv::Mat const& img, cv::Mat const& imgTempl, int method, CPoint2d& ptBest, double& dMinMax, double& dRate, double dScale = 0.0, int eInterpolation = cv::INTER_LINEAR);

	//-----------------------------------------------------------------------------
	// Mat to DC
	bool MatToMatTransparent(cv::Mat const& imgSource, cv::Mat& imgTarget, cv::Mat const& matMask);
	bool MatToMatTransparent(cv::Mat const& imgSource, cv::Mat& imgTarget, cv::Scalar const& crTransparent);

	bool IsGPUEnabled() {
		return s_bUseGPU;
	}

	bool CheckGPU(bool bUse) {
		if (s_bGPUChecked)
			return s_bUseGPU;

		if (bUse) {
			try {
				cv::Mat img(2, 2, CV_64F);
				cv::cuda::GpuMat mat;
				mat.upload(img);
				s_bUseGPU = true;
			}
			catch (cv::Exception&) {
				s_bUseGPU = false;
			}
		}
		else
			s_bUseGPU = false;

		s_bGPUChecked = true;

		return s_bUseGPU;
	}

	bool ConvertColor(cv::Mat const& imgSrc, cv::Mat& imgDest, int eCode) {
		try {
			//#ifdef HAVE_CUDA
			if (s_bUseGPU) {
				try {
					cv::cuda::GpuMat dst;
					cv::cvtColor(cv::cuda::GpuMat(imgSrc), dst, eCode);
					dst.download(imgDest);
					return true;
				} catch (...) {
					//TRACE((GTL__FUNCSIG " - Error\n").c_str());
				}
			}
			//#endif
			cvtColor(imgSrc, imgDest, eCode);
		}
		catch (...) {
			return false;
		}
		return true;
	}

	bool ResizeImage(cv::Mat const& imgSrc, cv::Mat& imgDest, double dScale, int eInterpolation) {
		try {
			//#ifdef HAVE_CUDA
			if (s_bUseGPU) {
				try {
					cv::cuda::GpuMat dst;
					cv::resize(cv::cuda::GpuMat(imgSrc), dst, cv::Size(), dScale, dScale, eInterpolation);
					dst.download(imgDest);
					return true;
				}
				catch (...) {
					//TRACE((GTL__FUNCSIG " - Error\n").c_str());
				}
			}
			//#endif
			resize(imgSrc, imgDest, cv::Size(), dScale, dScale, eInterpolation);
		}
		catch (...) {
			return false;
		}
		return true;
	}

	bool MatchTemplate(cv::Mat const& img, cv::Mat const& imgTempl, cv::Mat& matResult, int method) {
		try {
			//#ifdef HAVE_CUDA
			if (s_bUseGPU) {
				try {
					cv::cuda::GpuMat mat;
					cv::matchTemplate(cv::cuda::GpuMat(img), cv::cuda::GpuMat(imgTempl), mat, method);
					mat.download(matResult);
					return true;
				}
				catch (...) {
					//TRACE((GTL__FUNCSIG " - Error\n").c_str());
				}
			}
			//#endif
			matchTemplate(img, imgTempl, matResult, method);
		}
		catch (...) {
			return false;
		}
		return true;
	}

	bool MatchTemplate(cv::Mat const& img, cv::Mat const& imgTempl, int method, CPoint2d& ptBest, double& dMinMax, double& dRate, double dScale, int eInterpolation) {
		try {
			double dMin, dMax;
			cv::Point ptMin, ptMax;
			bool bSuccess {};

			//#ifdef HAVE_CUDA
			if (s_bUseGPU) {
				try {
					cv::cuda::GpuMat imgG;
					cv::cuda::GpuMat imgGTempl;
					if (dScale == 0.0) {
						imgG.upload(img);
						imgGTempl.upload(imgTempl);
					}
					else {
						cv::resize(cv::cuda::GpuMat(imgG), imgG, cv::Size(), dScale, dScale, eInterpolation);
						cv::resize(cv::cuda::GpuMat(imgGTempl), imgGTempl, cv::Size(), dScale, dScale, eInterpolation);
					}
					cv::cuda::GpuMat matResult;
					cv::matchTemplate(imgG, imgGTempl, matResult, method);
					cv::minMaxLoc(matResult, &dMin, &dMax, &ptMin, &ptMax);
					bSuccess = true;
				}
				catch (...) {
					//TRACE((GTL__FUNCSIG " - Error\n").c_str());
				}
			}
			//#endif

			if (!bSuccess) {
				cv::Mat matResult;
				cv::Mat imgG(img), imgGTempl(imgTempl);
				if (dScale != 0.0) {
					cv::resize(imgG, imgG, cv::Size(), dScale, dScale, eInterpolation);
					cv::resize(imgGTempl, imgGTempl, cv::Size(), dScale, dScale, eInterpolation);
				}
				cv::matchTemplate(imgG, imgGTempl, matResult, method);
				cv::minMaxLoc(matResult, &dMin, &dMax, &ptMin, &ptMax);
			}

			switch (method) {
			case cv::TM_SQDIFF :
			case cv::TM_SQDIFF_NORMED :
				ptBest = ptMin;
				break;
			case cv::TM_CCORR :
			case cv::TM_CCORR_NORMED :
			case cv::TM_CCOEFF :
			case cv::TM_CCOEFF_NORMED :
				ptBest = ptMax;
				break;
			}

			switch (method) {
			case cv::TM_SQDIFF :
				dMinMax = dMin;
				dRate = (dMax-dMin) / dMax;
				break;
			case cv::TM_SQDIFF_NORMED :
				dMinMax = dMin;
				dRate = (dMax-dMin) / dMax;
				break;
			case cv::TM_CCORR :
			case cv::TM_CCOEFF :
				dMinMax = dMax;
				dRate = dMax;
				break;
			case cv::TM_CCORR_NORMED :
			case cv::TM_CCOEFF_NORMED :
				dMinMax = dMax;
				dRate = dMax;
				break;
			}

			if (_isnan(dRate))
				dRate = 0;

			if (dScale == 0.0) {
				ptBest.x += imgTempl.cols/2;
				ptBest.y += imgTempl.rows/2;
			} else {
				ptBest.x = ptBest.x / dScale + imgTempl.cols/2;
				ptBest.y = ptBest.y / dScale + imgTempl.rows/2;
			}

		} catch (...) {
			return false;
		}
		return true;
	}

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

	bool MatToMatTransparent(cv::Mat const& imgSource, cv::Mat& imgTarget, cv::Mat const& matMask) {
		try {
			//std::vector<Mat> mats;
			//split(imgSource, mats);
			//merge(mats, imgSource);
			subtract(imgSource, imgSource, imgSource, matMask);
			add(imgSource, imgTarget, imgSource, matMask);
		}
		catch (...) {
			return false;
		}
		return true;
	}

	bool MatToMatTransparent(cv::Mat const& imgSource, cv::Mat& imgTarget, cv::Scalar const& crTransparent) {
		if (imgTarget.empty() || (imgSource.type() != imgTarget.type()) || (imgSource.size() != imgTarget.size()) ) {
			return false;
		}
		if ( (imgSource.channels() != 3) || (imgSource.depth() != CV_8U) )
			return false;

		cv::Vec3b cr;
		cr[0] = (uint8_t)crTransparent[0];
		cr[1] = (uint8_t)crTransparent[1];
		cr[2] = (uint8_t)crTransparent[2];

		for (int y = 0; y < imgSource.rows; y++) {
			const cv::Vec3b* ptr = imgSource.ptr<cv::Vec3b>(y);
			cv::Vec3b* ptr2 = imgTarget.ptr<cv::Vec3b>(y);
			for (int x = 0; x < imgSource.cols; x++) {
				if (ptr[x] != cr)
					ptr2[x] = ptr[x];
			}
		}

		return false;
	}

	inline cv::Mat LoadImageMat(std::filesystem::path const& path) {
		cv::Mat img;
		if (auto buf = FileToBuffer<uint8_t>(path); buf) {
			try {
				img = cv::imdecode(*buf, -1);
			}
			catch (...) {
			}
		}
		return img;
	}

	namespace internal {

		template < bool bPixelIndex, bool bLoopUnrolling = true >
		void MatToBitmapFile(std::ostream& f, cv::Mat const& img, int nBPP, std::vector<uint8_t> const& pal) {
			int width32 = (img.cols * nBPP + 31) / 32 * 4;
			std::vector<uint8_t> line((size_t)width32, 0);
			if constexpr (bLoopUnrolling) {
				if (nBPP == 1) {
					int ncol = img.cols / 8 * 8;
					for (int y{}; y < img.rows; y++) {
						auto* ptr = img.ptr<uint8_t>(y);
						//memset(line.data(), 0, line.size());

						int x{};
						for (; x < ncol; x += 8) {
							int col = x / 8;
							if constexpr (bPixelIndex) {
								line[col] = (ptr[x + 0] << 7) | (ptr[x + 1] << 6) | (ptr[x + 2] << 5) | (ptr[x + 3] << 4) | (ptr[x + 4] << 3) | (ptr[x + 5] << 2) | (ptr[x + 6] << 1) | (ptr[x + 7]);
							}
							else {
								line[col] = (pal[ptr[x + 0]] << 7) | (pal[ptr[x + 1]] << 6) | (pal[ptr[x + 2]] << 5) | (pal[ptr[x + 3]] << 4) | (pal[ptr[x + 4]] << 3) | (pal[ptr[x + 5]] << 2) | (pal[ptr[x + 6]] << 1) | (pal[ptr[x + 7]]);
							}
						}
						int col = x / 8;
						for (int shift{ 7 }; x < img.cols; x++, shift--) {
							if constexpr (bPixelIndex) {
								line[col] = ptr[x] << shift;
							}
							else {
								line[col] |= pal[ptr[x]] << shift;
							}
						}

						f.write((char const*)line.data(), width32);
					}
				}
				else if (nBPP == 4) {
					int ncol = img.cols / 2 * 2;
					for (int y{}; y < img.rows; y++) {
						auto* ptr = img.ptr<uint8_t>(y);
						//memset(line.data(), 0, line.size());

						int x{};
						for (; x < ncol; x += 2) {
							int col = x / 2;
							if constexpr (bPixelIndex) {
								line[col] = (ptr[x + 0] << 4) | (ptr[x + 1] << 0);
							}
							else {
								line[col] = (pal[ptr[x + 0]] << 4) | (pal[ptr[x + 1]] << 0);
							}
						}
						int col = x / 2;
						for (int shift{ 4 }; x < img.cols; x++, shift -= 4) {
							if constexpr (bPixelIndex) {
								line[col] |= ptr[x] << shift;
							}
							else {
								line[col] |= pal[ptr[x]] << shift;
							}
						}
						f.write((char const*)line.data(), width32);
					}
				}
				else if (nBPP == 8) {
					for (int y{}; y < img.rows; y++) {
						auto* ptr = img.ptr<uint8_t>(y);
						//memset(line.data(), 0, line.size());
						for (int x{}; x < img.cols; x++) {
							if constexpr (bPixelIndex) {
								line[x] = ptr[x];
							}
							else {
								line[x] = pal[ptr[x]];
							}
						}
						f.write((char const*)line.data(), width32);
					}
				}
			}
			else {
				int d = 8 / nBPP;
				for (int y{}; y < img.rows; y++) {
					auto* ptr = img.ptr<uint8_t>(y);
					//memset(line.data(), 0, line.size());

					for (int x{}; x < img.cols; x++) {
						int col = x / d;
						int shift = 8 - ((x * nBPP) % 8);
						if constexpr (bPixelIndex) {
							line[col] |= ptr[x] << shift;
						}
						else {
							line[col] |= pal[ptr[x]] << shift;
						}
					}
					f.write((char const*)line.data(), width32);
				}
			}
		}
	}

	bool SaveBitmapMat(std::filesystem::path const& path, cv::Mat const& img, int nBPP, std::span<gtl::color_bgra_t> palette = {}, bool bPixelIndex = false) {
	#pragma pack(push, 2)
		struct BMP_FILE_HEADER {
			char sign[2]{ 'B', 'M' };
			uint32_t sizeFile{};
			uint16_t reserved1{};
			uint16_t reserved2{};
			uint32_t offsetData{};
		};
		struct BITMAPINFOHEADER {
			uint32_t      size;
			int32_t       width;
			int32_t       height;
			uint16_t      planes;
			uint16_t      nBPP;
			uint32_t      compression;
			uint32_t      sizeImage;
			int32_t       XPelsPerMeter;
			int32_t       YPelsPerMeter;
			uint32_t      nColorUsed;
			uint32_t      nColorImportant;
		};
	#pragma pack(pop)

		if (img.empty())
			return false;

		auto type = img.type();

		int cx = img.cols;
		int cy = img.rows;
		if ((cx >= 0xffff) or (cy >= 0xffff))
			return false;
		if ((ptrdiff_t)cx * cy >= (ptrdiff_t)(0xffff'ffff - sizeof(BMP_FILE_HEADER) - sizeof(BITMAPINFOHEADER)))
			return false;
		int pixel_size = (type == CV_8UC3) ? 3 : ((type == CV_8UC1) ? 1 : 0);
		if (pixel_size <= 0)
			return false;

		BITMAPINFOHEADER header{};

		header.size = sizeof(header);
		header.width = cx;
		header.height = -cy;
		header.planes = 1;
		header.compression = 0;//BI_RGB;
		header.sizeImage = 0;//cx * cy * pixel_size;
		header.XPelsPerMeter = 0;
		header.YPelsPerMeter = 0;

		if (pixel_size == 3) {
			std::ofstream f(path, std::ios_base::binary);
			if (!f)
				return false;

			BMP_FILE_HEADER fh;

			auto width32 = gtl::AdjustAlign32(cx * 3);
			fh.offsetData = sizeof(fh) + sizeof(header);
			fh.sizeFile = fh.offsetData + width32 * cy;
			header.nBPP = 8 * pixel_size;
			header.nColorUsed = header.nColorImportant = 0;

			f.write((char const*)&fh, sizeof(fh));
			f.write((char const*)&header, sizeof(header));
			char redundant[4];
			size_t sr = width32 - (cx * 3);
			for (int y{}; y < img.rows; y++) {
				auto* ptr = img.ptr<cv::Vec3b>(y);
				f.write((char const*)ptr, 3 * img.cols);
				f.write(redundant, sr);
			}
		}
		else if (type == CV_8UC1) {
			if ((nBPP != 1) and /*(nBPP != 2) and */(nBPP != 4) and (nBPP != 8)) {
				return false;
			}
			if (palette.empty()) {
				return false;
			}

			std::ofstream f(path, std::ios_base::binary);
			if (!f)
				return false;

			BMP_FILE_HEADER fh;

			auto width32 = (cx * nBPP + 31) / 32 * 4;
			fh.offsetData = sizeof(fh) + sizeof(header) + (0x01ul << nBPP) * sizeof(gtl::color_bgra_t);
			fh.sizeFile = fh.offsetData + width32 * cy;
			header.nBPP = nBPP;
			header.nColorUsed = header.nColorImportant = palette.size();

			f.write((char const*)&fh, sizeof(fh));
			f.write((char const*)&header, sizeof(header));

			f.write((char const*)palette.data(), palette.size() * sizeof(palette[0]));

			std::vector<uint8_t> pal((size_t)256, 0);
			if (!bPixelIndex) {
				for (size_t i{}; i < palette.size(); i++) {
					pal[palette[i].r] = i;
				}
			}

			constexpr bool bLoopUnrolling = true;
			if (bPixelIndex)
				gtl::internal::MatToBitmapFile<true, bLoopUnrolling>(f, img, nBPP, pal);
			else
				gtl::internal::MatToBitmapFile<false, bLoopUnrolling>(f, img, nBPP, pal);

		}

		return true;
	}


	namespace internal {

		template < bool bLoopUnrolling = true >
		bool MatFromBitmapFile(std::istream& f, cv::Mat& img, int nBPP, std::vector<cv::Vec3b> palette) {
			if (img.type() != CV_8UC3)
				return false;
			int width32 = (img.cols * nBPP + 31) / 32 * 4;
			std::vector<uint8_t> line((size_t)width32, 0);

			if ((nBPP == 24) or (nBPP == 32)) {
				int nByte = nBPP / 8;
				for (int y{}; y < img.rows; y++) {
					if (!f.read((char*)line.data(), line.size()))
						return false;
					auto* ptr = img.ptr<cv::Vec3b>(y);

					for (int x{}; x < img.cols; x++) {
						int xc = x * nByte;
						ptr[x][0] = line[xc + 0];
						ptr[x][1] = line[xc + 1];
						ptr[x][2] = line[xc + 2];
					}

				}
				return true;
			}

			if constexpr (bLoopUnrolling) {
				if (nBPP == 1) {
					if (palette.size() < 2)
						return false;
					int ncol = img.cols / 8 * 8;
					for (int y{}; y < img.rows; y++) {
						if (!f.read((char*)line.data(), line.size()))
							return false;
						auto* ptr = img.ptr<cv::Vec3b>(y);

						int x{};
						for (; x < ncol; x += 8) {
							int col = x / 8;
							ptr[x + 0] = palette[(line[col] >> 7) & 0b0000'0001];
							ptr[x + 1] = palette[(line[col] >> 6) & 0b0000'0001];
							ptr[x + 2] = palette[(line[col] >> 5) & 0b0000'0001];
							ptr[x + 3] = palette[(line[col] >> 4) & 0b0000'0001];
							ptr[x + 4] = palette[(line[col] >> 3) & 0b0000'0001];
							ptr[x + 5] = palette[(line[col] >> 2) & 0b0000'0001];
							ptr[x + 6] = palette[(line[col] >> 1) & 0b0000'0001];
							ptr[x + 7] = palette[(line[col] >> 0) & 0b0000'0001];
						}
						int col = x / 8;
						for (int shift{ 7 }; x < img.cols; x++, shift--) {
							ptr[x] = palette[(line[col] >> shift) & 0b0000'0001];
						}
					}
				}
				else if (nBPP == 4) {
					if (palette.size() < 16)
						return false;
					int ncol = img.cols / 2 * 2;
					for (int y{}; y < img.rows; y++) {
						if (!f.read((char*)line.data(), line.size()))
							return false;
						auto* ptr = img.ptr<cv::Vec3b>(y);

						int x{};
						for (; x < ncol; x += 2) {
							int col = x / 2;
							ptr[x + 0] = palette[(line[col] >> 4) & 0b0000'1111];
							ptr[x + 1] = palette[(line[col] >> 0) & 0b0000'1111];
						}
						int col = x / 2;
						for (int shift{ 4 }; x < img.cols; x++, shift -= 4) {
							ptr[x] = palette[(line[col] >> shift) & 0b0000'1111];
						}
					}
				}
				else if (nBPP == 8) {
					if (palette.size() < 256)
						return false;
					for (int y{}; y < img.rows; y++) {
						if (!f.read((char*)line.data(), line.size()))
							return false;
						auto* ptr = img.ptr<cv::Vec3b>(y);
						for (int x{}; x < img.cols; x++) {
							ptr[x] = palette[line[x]];
						}
					}
				}
				else
					return false;
			}
			else {
				if ((nBPP != 1) and (nBPP != 4) and (nBPP != 8))
					return false;

				int d = 8 / nBPP;
				uint8_t mask = (0x01 << nBPP) - 1;
				for (int y{}; y < img.rows; y++) {
					if (!f.read((char*)line.data(), line.size()))
						return false;
					auto* ptr = img.ptr<cv::Vec3b>(y);

					for (int x{}; x < img.cols; x++) {
						int col = x / d;
						int shift = 8 - ((x * nBPP) % 8);
						ptr[x] = palette[(line[col] >> shift) & mask];
					}
				}
			}
			return true;
		}
	}


	/// @brief Save Image to BITMAP. Image is COLOR or GRAY level image.
	/// @param path 
	/// @param img : CV_8UC1 : gray scale, CV_8UC3 : color (no palette supported), for CV8UC3, palette is not used.
	/// @param nBPP 
	/// @param palette 
	/// @param bPixelIndex if true, img value is NOT a pixel but a palette index. a full palette must be given.
	/// @return 
	cv::Mat LoadBitmapMat(std::filesystem::path const& path) {
		cv::Mat img;
	#pragma pack(push, 2)
		struct BMP_FILE_HEADER {
			char sign[2]{ 'B', 'M' };
			uint32_t sizeFile{};
			uint16_t reserved1{};
			uint16_t reserved2{};
			uint32_t offsetData{};
		};
		struct BITMAPINFOHEADER {
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

		using FXPT2DOT30 = int32_t;
		struct CIEXYZ {
			FXPT2DOT30 x, y, z;
		};
		struct CIEXYZTRIPLE {
			CIEXYZ r, g, b;
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
			CIEXYZTRIPLE	endpoints;
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
			CIEXYZTRIPLE	endpoints;
			uint32_t		gammaRed;
			uint32_t		gammaGreen;
			uint32_t		gammaBlue;
			uint32_t		intent;
			uint32_t		profileData;
			uint32_t		profileSize;
			uint32_t		reserved;
		};
	#pragma pack(pop)

		std::ifstream f(path, std::ios_base::binary);
		if (!f)
			return img;

		BMP_FILE_HEADER fh;
		BITMAP_V5_HEADER header{};

		if (!f.read((char*)&fh, sizeof(fh)))
			return img;
		uint32_t sizeHeader{};
		if (!f.read((char*)&sizeHeader, sizeof(sizeHeader)))
			return img;
		switch (sizeHeader) {
		case sizeof(BITMAPINFOHEADER) :
		case sizeof(BITMAP_V4_HEADER) :
		case sizeof(BITMAP_V5_HEADER) :
			header.size = sizeHeader;
			f.read(((char*)&header) + sizeof(sizeHeader), sizeHeader - sizeof(sizeHeader));
			break;
		default:
			return img;
		}

		if (header.compression or (header.planes != 1))
			return img;

		int cx = header.width;
		int cy = header.height;
		bool bFlipY{};
		if (cy < 0) {
			cy = -cy;
		}
		else {
			bFlipY = true;
		}
		if ((cx <= 0) or (cy <= 0) or (cx * (uint64_t)cy >= 0xffff'ff00ull))
			return img;

		std::vector<cv::Vec3b> palette;

		// Load Palette
		ptrdiff_t sizePalette = fh.offsetData - sizeof(fh) - header.size;
		for (; sizePalette >= 4; sizePalette -= sizeof(gtl::color_bgra_t)) {
			gtl::color_bgra_t color{};
			if (!f.read((char*)&color, sizeof(color)))
				return img;
			palette.push_back(cv::Vec3b(color.b, color.g, color.r));
		}
		if (!f.seekg(fh.offsetData))
			return img;

		img = cv::Mat::zeros(cv::Size(cx, cy), CV_8UC3);
		constexpr bool bLoopUnrolling = true;
		if (!gtl::internal::MatFromBitmapFile<bLoopUnrolling>(f, img, header.nBPP, palette)) {
			img.release();
		}
		if (bFlipY) {
			cv::flip(img, img, 0);
		}

		return img;
	}

	//=============================================================================
	//

}	// namespace gtl

