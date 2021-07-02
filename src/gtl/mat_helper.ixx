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

#include "gtl/_config.h"
#include "gtl/_macro.h"

#include "opencv2/opencv.hpp"

export module gtl:mat_helper;
import :coord;
import :archive;

namespace gtl {
	bool s_bGPUChecked {};
	bool s_bUseGPU {};

	constexpr static bool const bLoopUnrolling = false;
	constexpr static bool const bMultiThreaded = false;

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
		cr[0] = (uint8)crTransparent[0];
		cr[1] = (uint8)crTransparent[1];
		cr[2] = (uint8)crTransparent[2];

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
		if (auto buf = FileToBuffer<uint8>(path); buf) {
			try {
				img = cv::imdecode(*buf, -1);
			}
			catch (...) {
			}
		}
		return img;
	}

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
#pragma pack(pop)

	namespace internal {

		template < bool bPixelIndex, typename telement = uint8, bool bLoopUnrolling = true, bool bMultiThreaded = true >
		bool MatToBitmapFile(std::ostream& f, cv::Mat const& img, int nBPP, std::vector<telement> const& pal, callback_progress_t funcCallback) {

			int width32 = (img.cols * nBPP + 31) / 32 * 4;
			int pixel_per_byte = (8/nBPP);
			int nColPixel = img.cols/ pixel_per_byte * pixel_per_byte;

			using Func_PackSingleRow = std::function<void(int y, std::vector<uint8>& line, telement const* ptr, std::vector<telement> const& pal)>;
			Func_PackSingleRow PackSingleRow;

			if constexpr (bLoopUnrolling) {

				switch (nBPP) {
				case 1 :
					PackSingleRow = [img_cols = img.cols, nBPP, pixel_per_byte, nColPixel](int y, std::vector<uint8>& line, telement const* ptr, std::vector<telement> const& pal) {
						int x{};
						for (; x < nColPixel; x += pixel_per_byte) {
							int col = x / pixel_per_byte;
							if constexpr (bPixelIndex) {
								line[col] = (ptr[x + 0] << 7) | (ptr[x + 1] << 6) | (ptr[x + 2] << 5) | (ptr[x + 3] << 4) | (ptr[x + 4] << 3) | (ptr[x + 5] << 2) | (ptr[x + 6] << 1) | (ptr[x + 7]);
							}
							else {
								line[col] = (pal[ptr[x + 0]] << 7) | (pal[ptr[x + 1]] << 6) | (pal[ptr[x + 2]] << 5) | (pal[ptr[x + 3]] << 4) | (pal[ptr[x + 4]] << 3) | (pal[ptr[x + 5]] << 2) | (pal[ptr[x + 6]] << 1) | (pal[ptr[x + 7]]);
							}
						}
						int col = x / pixel_per_byte;
						if (col < line.size())
							line[col] = 0;
						for (int shift{ 8-nBPP }; x < img_cols; x++, shift--) {
							if constexpr (bPixelIndex) {
								line[col] |= ptr[x] << shift;
							}
							else {
								line[col] |= pal[ptr[x]] << shift;
							}
						}
					};
					break;
				case 4 :
					PackSingleRow = [img_cols = img.cols, nBPP, pixel_per_byte, nColPixel](int y, std::vector<uint8>& line, telement const* ptr, std::vector<telement> const& pal) {
						int x{};
						for (; x < nColPixel; x += pixel_per_byte) {
							int col = x / pixel_per_byte;
							if constexpr (bPixelIndex) {
								line[col] = (ptr[x + 0] << 4) | (ptr[x + 1] << 0);
							}
							else {
								line[col] = (pal[ptr[x + 0]] << 4) | (pal[ptr[x + 1]] << 0);
							}
						}
						int col = x / pixel_per_byte;
						if (col < line.size())
							line[col] = 0;
						for (int shift{ 8-nBPP }; x < img_cols; x++, shift-=nBPP) {
							if constexpr (bPixelIndex) {
								line[col] |= ptr[x] << shift;
							}
							else {
								line[col] |= pal[ptr[x]] << shift;
							}
						}
					};
					break;
				case 8 :
					PackSingleRow = [img_cols = img.cols](int y, std::vector<uint8>& line, telement const* ptr, std::vector<telement> const& pal) {
						for (int x{}; x < img_cols; x++) {
							if constexpr (bPixelIndex) {
								line[x] = ptr[x];
							}
							else {
								line[x] = pal[ptr[x]];
							}
						}
					};
					break;
				case 24 :
					//follows
					break;
				default :
					return false;
				}
			}
			else {
				switch (nBPP) {
				case 1 :
				case 4 :
					PackSingleRow = [img_cols = img.cols, nBPP, pixel_per_byte](int y, std::vector<uint8>& line, telement const* ptr, std::vector<telement> const& pal) {
						std::memset(line.data(), 0, line.size()*sizeof(line[0]));
						for (int x{}; x < img_cols; x++) {
							int col = x / pixel_per_byte;
							int shift = 8 - (nBPP * ((x%pixel_per_byte)+1));
							if constexpr (bPixelIndex) {
								line[col] |= ptr[x] << shift;
							}
							else {
								line[col] |= pal[ptr[x]] << shift;
							}
						}
					};
					break;
				case 8 :
					PackSingleRow = [img_cols = img.cols](int y, std::vector<uint8>& line, telement const* ptr, std::vector<telement> const& pal) {
						for (int x{}; x < img_cols; x++) {
							if constexpr (bPixelIndex) {
								line[x] = ptr[x];
							}
							else {
								line[x] = pal[ptr[x]];
							}
						}
					};
					break;
				case 24 :
					//follows
					break;
				default :
					return false;
				}
			}

			if (nBPP == 24) {
				if (sizeof(telement) != 3)
					return false;
				PackSingleRow = [img_cols = img.cols, nBPP, pixel_per_byte](int y, std::vector<uint8>& line, telement const* ptr, std::vector<telement> const& pal) {
					telement* line3 = (telement*)line.data();
					for (int x{}; x < img_cols; x++) {
						if constexpr (bPixelIndex) {
							line3[x] = ptr[x];
						}
						else {
							line3[x] = pal[ptr[x]];
						}
					}
				};
			}

			if constexpr (bMultiThreaded) {
				auto nCPUDetected = std::thread::hardware_concurrency();
				auto nThread = std::min((uint)img.rows, (nCPUDetected <= 0) ? 2 : nCPUDetected);

				struct BUFFER {
					std::atomic<int> y{-1};
					std::atomic<bool> bReady{false};
					std::vector<uint8> line;
					//std::set<int> idsThread;

					BUFFER() : y{-1} {}
				};
				size_t sizeBuffer = nThread*2;
				std::mutex mtxBuffer;
				std::vector<BUFFER> buffers(sizeBuffer);
				for (auto& buf : buffers)
					buf.line.assign(width32, 0);

				std::atomic<int> yCur, yWritten;
				bool bWritten{};

				auto GetBuffer = [img_rows = img.rows, &mtxBuffer, &buffers, &yCur, &yWritten]() -> BUFFER* {
					if (yCur >= img_rows)
						return nullptr;
					std::unique_lock lock(mtxBuffer);
					auto y = yCur.fetch_add(1);
					if (y >= img_rows)
						return nullptr;
					auto& buf = buffers[y % buffers.size()];
					int yOld = buf.y;
					while (yOld >= 0) {
						buf.y.wait(yOld);
						yOld = buf.y;
					}
					buf.y = y;
					buf.y.notify_one();

					return &buf;
				};

				auto PackBuffer = [&yCur, &img, &buffers, &PackSingleRow, &pal, &yWritten/*, &id*/, &GetBuffer](std::stop_token stop) {
					do {
						auto* pBuffer = GetBuffer();
						if (!pBuffer)
							break;
						auto& buf = *pBuffer;

						PackSingleRow(buf.y, buf.line, img.ptr(buf.y), pal);

						buf.bReady = true;
						buf.bReady.notify_one();
					} while(!stop.stop_requested());
				};

				auto Writer = [&f, &buffers, img_rows = img.rows, &yWritten, &bWritten, &funcCallback](std::stop_token stop_token) {
					int iPercent{};
					while (yWritten < img_rows /*and !stop_token.stop_requested()*/) {
						auto index = yWritten % buffers.size();
						auto& buf = buffers[index];

						while (yWritten != buf.y) {
							buf.y.wait(-1);
						}
						while (!buf.bReady)
							buf.bReady.wait(false);

						if (!f.write((char*)buf.line.data(), buf.line.size()))
							return;

						buf.bReady = false;
						buf.y = -1;
						buf.y.notify_one();

						yWritten++;
						yWritten.notify_one();
					
						if (funcCallback) {
							int iPercentNew = yWritten * 100 / img_rows;
							if (iPercent != iPercentNew) {
								iPercent = iPercentNew;
								if (!funcCallback(iPercent, false, false))
									return;
							}
						}
					};
					f.flush();
					bWritten = (bool)f;
				};

				std::vector<std::jthread> threads;
				threads.reserve(nThread+1);
				threads.emplace_back(Writer);
				for (uint i{}; i < nThread; i++) {
					threads.emplace_back(PackBuffer);
				}

				threads.front().join();
				if (!bWritten) {
					for (auto& thread : threads)
						thread.request_stop();
				}
				threads.clear();
				return bWritten;
			}
			else {
				int iPercent{};
				std::vector<uint8> line((size_t)width32, 0);
				for (int y{}; y < img.rows; y++) {
					auto const* ptr = img.ptr<telement>(y);
					PackSingleRow(y, line, ptr, pal);
					f.write((char const*)line.data(), width32);

					if (funcCallback) {
						int iPercentNew = y * 100 / img.rows;
						if (iPercent != iPercentNew) {
							iPercent = iPercentNew;
							if (!funcCallback(iPercent, false, false))
								return false;
						}
					}
				}
				return true;
			}
		}

	}	// namespace internal


	bool SaveBitmapMat(std::filesystem::path const& path, cv::Mat const& img, int nBPP, std::span<gtl::color_bgra_t> palette = {}, bool bPixelIndex = false, callback_progress_t funcCallback = nullptr) {
		// todo : CV_8UC3 with palette.

		bool bOK{};

		// Trigger notifying it's over.
		CTrigger trigger([&funcCallback, &bOK]{if (funcCallback) funcCallback(-1, true, bOK);});

		if (img.empty())
			return false;

		auto type = img.type();

		int cx = img.cols;
		int cy = img.rows;
		if ((cx >= 0xffff) or (cy >= 0xffff))
			return false;
		if ((ptrdiff_t)cx * cy >= (ptrdiff_t)(0xffff'ffff - sizeof(BMP_FILE_HEADER) - sizeof(BITMAP_HEADER)))
			return false;
		int pixel_size = (type == CV_8UC3) ? 3 : ((type == CV_8UC1) ? 1 : 0);
		if (pixel_size <= 0)
			return false;

		BITMAP_HEADER header{};

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
			char redundant[4]{};
			size_t sr = width32 - (cx * 3);
			for (int y{}; y < img.rows; y++) {
				auto* ptr = img.ptr<cv::Vec3b>(y);
				f.write((char const*)ptr, 3 * img.cols);
				f.write(redundant, sr);
			}

			//constexpr bool bLoopUnrolling = true;
			//constexpr bool bMultiThreaded = true;
			//return MatToBitmapFile<true, cv::Vec3b, bLoopUnrolling, bMultiThreaded>(f, img, nBPP, {});

			bOK = true;
			return true;
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
			fh.offsetData = sizeof(fh) + sizeof(header) + (0x01 << nBPP) * sizeof(gtl::color_bgra_t);
			fh.sizeFile = fh.offsetData + width32 * cy;
			header.nBPP = nBPP;
			header.nColorUsed = header.nColorImportant = (uint32_t)palette.size();

			f.write((char const*)&fh, sizeof(fh));
			f.write((char const*)&header, sizeof(header));

			f.write((char const*)palette.data(), palette.size() * sizeof(palette[0]));

			std::vector<uint8> pal((size_t)256, 0);
			if (!bPixelIndex) {
				for (size_t i{}; i < palette.size(); i++) {
					pal[palette[i].r] = (uint8)i;
				}
			}

			if (bPixelIndex)
				bOK = gtl::internal::MatToBitmapFile<true, uint8, bLoopUnrolling, bMultiThreaded>(f, img, nBPP, pal, funcCallback);
			else
				bOK = gtl::internal::MatToBitmapFile<false, uint8, bLoopUnrolling, bMultiThreaded>(f, img, nBPP, pal, funcCallback);
			return bOK;
		}

		return false;
	}

	namespace internal {

		template < typename telement = cv::Vec3b, bool bLoopUnrolling = true, bool bMultiThreaded = false >
		bool MatFromBitmapFile(std::istream& f, cv::Mat& img, int nBPP, std::vector<telement> palette, callback_progress_t funcCallback) {
			//auto type = img.type();
			//if ( (type != CV_8UC3) and (type != CV_8UC4) and (type != CV_8UC1) )
			//	return false;
			size_t s = img.elemSize();
			if (sizeof(telement) != img.elemSize())
				return false;

			int width32 = (img.cols * nBPP + 31) / 32 * 4;
			int pixel_per_byte = (8/nBPP);
			int nColPixel = pixel_per_byte ? img.cols/ pixel_per_byte * pixel_per_byte : img.cols;

			using Func_UnPackSingleRow = std::function<void(int y, std::vector<uint8> const& line, telement* ptr, std::vector<telement> const& palette)>;
			Func_UnPackSingleRow UnPackSingleRow;

			if ((nBPP == 24) or (nBPP == 32)) {
				std::vector<uint8> line((size_t)width32, 0);
				int nByte = nBPP / 8;
				int iPercent{};
				for (int y{}; y < img.rows; y++) {
					if (!f.read((char*)line.data(), line.size()))
						return false;
					auto* ptr = img.ptr<telement>(y);

					for (int x{}; x < img.cols; x++) {
						int xc = x * nByte;
						if constexpr (std::is_same_v<telement, cv::Vec3b>) {
							ptr[x][0] = line[xc + 0];
							ptr[x][1] = line[xc + 1];
							ptr[x][2] = line[xc + 2];
						}
					}
					if (funcCallback) {
						int iPercentNew = y*100/img.rows;
						if (iPercent != iPercentNew) {
							iPercent = iPercentNew;
							if (!funcCallback(iPercent, false, false))
								return false;
						}
					}
				}
				return true;
			}

			using Func_UnpackLine = std::function<void()>;

			if constexpr (bLoopUnrolling) {
				if (nBPP == 1) {
					if (palette.size() < 2)
						return false;
					UnPackSingleRow = [img_cols = img.cols, &nBPP, &pixel_per_byte, &nColPixel](int y, std::vector<uint8> const& line, telement* ptr, std::vector<telement> const& palette) {
						int x{};
						for (; x < nColPixel; x += pixel_per_byte) {
							int col = x / pixel_per_byte;
							ptr[x + 0] = palette[(line[col] >> 7) & 0b0000'0001];
							ptr[x + 1] = palette[(line[col] >> 6) & 0b0000'0001];
							ptr[x + 2] = palette[(line[col] >> 5) & 0b0000'0001];
							ptr[x + 3] = palette[(line[col] >> 4) & 0b0000'0001];
							ptr[x + 4] = palette[(line[col] >> 3) & 0b0000'0001];
							ptr[x + 5] = palette[(line[col] >> 2) & 0b0000'0001];
							ptr[x + 6] = palette[(line[col] >> 1) & 0b0000'0001];
							ptr[x + 7] = palette[(line[col] >> 0) & 0b0000'0001];
						}
						int col = x / pixel_per_byte;
						for (int shift{ 8-nBPP }; x < img_cols; x++, shift -= nBPP) {
							ptr[x] = palette[(line[col] >> shift) & 0b0000'0001];
						}
					};
				}
				else if (nBPP == 4) {
					if (palette.size() < 16)
						return false;
					UnPackSingleRow = [img_cols = img.cols, &nBPP, &pixel_per_byte, &nColPixel](int y, std::vector<uint8> const& line, telement* ptr, std::vector<telement> const& palette) {
						int x{};
						for (; x < nColPixel; x += 2) {
							int col = x / 2;
							ptr[x + 0] = palette[(line[col] >> 4) & 0b0000'1111];
							ptr[x + 1] = palette[(line[col] >> 0) & 0b0000'1111];
						}
						int col = x / 2;
						for (int shift{ 4 }; x < img_cols; x++, shift -= 4) {
							ptr[x] = palette[(line[col] >> shift) & 0b0000'1111];
						}
					};
				}
				else if (nBPP == 8) {
					if (palette.size() < 256)
						return false;
					UnPackSingleRow = [img_cols = img.cols](int y, std::vector<uint8> const& line, telement* ptr, std::vector<telement> const& palette) {
						for (int x{}; x < img_cols; x++) {
							ptr[x] = palette[line[x]];
						}
					};
				}
				else
					return false;
			}
			else {
				if ((nBPP != 1) and (nBPP != 4) and (nBPP != 8))
					return false;
				if ( palette.size() < (0x01 << nBPP) )
					return false;

				if (nBPP == 8) {
					UnPackSingleRow = [img_cols = img.cols](int y, std::vector<uint8> const& line, telement* ptr, std::vector<telement> const& palette) {
						for (int x{}; x < img_cols; x++) {
							ptr[x] = palette[line[x]];
						}
					};
				} else {
					uint8 mask = (0x01 << nBPP) - 1;
					UnPackSingleRow = [img_cols = img.cols, &nBPP, &pixel_per_byte, mask](int y, std::vector<uint8> const& line, telement* ptr, std::vector<telement> const& palette) {
						for (int x{}; x < img_cols; x++) {
							int col = x / pixel_per_byte;
							int shift = 8 - (nBPP * ((x%pixel_per_byte)+1));
							ptr[x] = palette[(line[col] >> shift) & mask];
						}
					};
				}
			}

			if constexpr (bMultiThreaded) {
				auto nCPUDetected = std::thread::hardware_concurrency();
				auto nThread = std::min((uint)img.rows, (nCPUDetected <= 0) ? 2 : nCPUDetected);

				struct BUFFER {
					std::atomic<int> y{-1};
					std::vector<uint8> line;
				};
				std::vector<BUFFER> buffers((size_t)nThread*2);
				for (auto& buf : buffers)
					buf.line.assign((size_t)width32, (uint8)0);

				std::condition_variable cv;
				std::mutex mtxQ;
				std::queue<BUFFER*> q;
				bool bError{};

				auto Reader = [&f, &buffers, img_rows = img.rows, &q, &mtxQ, &cv, &bError, &funcCallback]() {
					int y{};
					int iPercent{};
					for (; y < img_rows; y++) {
						auto index = y % buffers.size();
						auto& buf = buffers[index];
						int yOld = buf.y;
						while (yOld >= 0) {
							buf.y.wait(yOld);
							yOld = buf.y;
						}
						if (!f.read((char*)buf.line.data(), buf.line.size())) {
							bError = true;
							break;
						}
						buf.y = y;
						//buf.y.notify_one();
						{
							std::unique_lock lock(mtxQ);
							q.push(&buf);
						}
						cv.notify_one();

						if (funcCallback) {
							int iPercentNew = y * 100 / img_rows;
							if (iPercent != iPercentNew) {
								iPercent = iPercentNew;
								if (!funcCallback(iPercent, false, false)) {
									bError = true;
									break;
								}
							}
						}
					}
				};

				auto UnpackBuffer = [&q, &mtxQ, &cv, &UnPackSingleRow, &img, &palette](std::stop_token tk) {
					do {
						BUFFER* pBuf{};
						while (!pBuf) {
							std::unique_lock lock(mtxQ);
							if (q.empty()) {
								if (tk.stop_requested())
									return;
								cv.wait(lock);
							}
							else {
								pBuf = q.front();
								q.pop();
							}
						}

						UnPackSingleRow(pBuf->y, pBuf->line, img.ptr<telement>(pBuf->y), palette);
						pBuf->y = -1;
						pBuf->y.notify_one();

					} while (true);
				};

				std::thread reader{Reader};
				std::stop_source ss;
				std::vector<std::jthread> threads;
				threads.reserve(nThread);
				for (uint i{}; i < nThread; i++) {
					threads.emplace_back(UnpackBuffer, ss.get_token());
				}

				reader.join();
				ss.request_stop();
				cv.notify_all();

				return !bError;

			}
			else {
				std::vector<uint8> line((size_t)width32, 0);
				int iPercent{};
				for (int y{}; y < img.rows; y++) {
					if (!f.read((char*)line.data(), line.size()))
						return false;
					auto* ptr = img.ptr<telement>(y);

					UnPackSingleRow(y, line, ptr, palette);

					if (funcCallback) {
						int iPercentNew = y * 100 / img.rows;
						if (iPercent != iPercentNew) {
							iPercent = iPercentNew;
							if (!funcCallback(iPercent, false, false))
								return false;
						}
					}
				}
			}

			return true;
		}

	}	// namespace internal

	/// @brief Save Image to BITMAP. Image is COLOR or GRAY level image.
	/// @param path 
	/// @param img : CV_8UC1 : gray scale, CV_8UC3 : color (no palette supported), for CV8UC3, palette is not used.
	/// @param nBPP 
	/// @param palette 
	/// @param bPixelIndex if true, img value is NOT a pixel but a palette index. a full palette must be given.
	/// @return 
	cv::Mat LoadBitmapMat(std::filesystem::path const& path, callback_progress_t funcCallback = nullptr) {
		bool bOK{};

		// Trigger notifying it's over.
		CTrigger trigger([&funcCallback, &bOK] {if (funcCallback) funcCallback(-1, true, bOK); });


		cv::Mat img;

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
		case sizeof(BITMAP_HEADER) :
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
		if ((cx <= 0) or (cy <= 0) or ((uint64_t)cx * (uint64_t)cy >= 0xffff'ff00ull))
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

		bool bGrayScale {};
		if (header.nBPP <= 8) {
			bGrayScale = true;
			for (auto const& v : palette) {
				if ((v[0] != v[1]) and (v[1] != v[2])) {
					bGrayScale = false;
					break;
				}
			}
		}

		img = cv::Mat::zeros(cv::Size(cx, cy), bGrayScale ? CV_8UC1 : CV_8UC3);
		if (bGrayScale) {
			std::vector<uint8> paletteG(palette.size(), 0);
			for (size_t i{}; i < palette.size(); i++)
				paletteG[i] = palette[i][0];

			bOK = gtl::internal::MatFromBitmapFile<uint8, bLoopUnrolling, bMultiThreaded>(f, img, header.nBPP, paletteG, funcCallback);
		}
		else {
			bOK = gtl::internal::MatFromBitmapFile<cv::Vec3b, bLoopUnrolling, bMultiThreaded>(f, img, header.nBPP, palette, funcCallback);
		}
		if (!bOK)
			img.release();

		if (!img.empty() and bFlipY) {
			cv::flip(img, img, 0);
		}

		return img;
	}

	//=============================================================================
	//

}	// namespace gtl

