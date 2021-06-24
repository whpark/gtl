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

#include "opencv2/opencv.hpp"

export module gtl:mat_helper;
import :coord;

namespace gtl {
	bool s_bGPUChecked {};
	bool s_bUseGPU {};
}

export namespace gtl {

	//-----------------------------------------------------------------------------
	// Mat Load/Store
	bool ReadMat(std::istream& is, cv::Mat& mat) {
		auto ReadVar = [&is, &mat] (auto& var) -> bool {
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

		} while (false);

		return true;
	}
	inline static bool SaveMat(std::ostream& os, cv::Mat const& mat) {
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
			} catch (cv::Exception&) {
				s_bUseGPU = false;
			}
		} else
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
		} catch (...) {
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
				} catch (...) {
					//TRACE((GTL__FUNCSIG " - Error\n").c_str());
				}
			}
			//#endif
			resize(imgSrc, imgDest, cv::Size(), dScale, dScale, eInterpolation);
		} catch (...) {
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
				} catch (...) {
					//TRACE((GTL__FUNCSIG " - Error\n").c_str());
				}
			}
			//#endif
			matchTemplate(img, imgTempl, matResult, method);
		} catch (...) {
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
					} else {
						cv::resize(cv::cuda::GpuMat(imgG), imgG, cv::Size(), dScale, dScale, eInterpolation);
						cv::resize(cv::cuda::GpuMat(imgGTempl), imgGTempl, cv::Size(), dScale, dScale, eInterpolation);
					}
					cv::cuda::GpuMat matResult;
					cv::matchTemplate(imgG, imgGTempl, matResult, method);
					cv::minMaxLoc(matResult, &dMin, &dMax, &ptMin, &ptMax);
					bSuccess = true;
				} catch (...) {
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
	bool CalcViewPosition(cv::Size const& sizeView, TRECT const& rectView, TRECT& rectImage, TRECT& rectDst) {
		if (rectView.IsRectEmpty()) {
			rectImage.SetRectEmpty();
			rectDst.SetRectEmpty();
			return false;
		}
		auto wDest = rectView.Width();
		auto hDest = rectView.Height();

		if (wDest >= sizeView.width) {
			rectDst.left = rectView.left + (wDest - sizeView.width)/2;
			rectDst.right = rectDst.left + sizeView.width;
			rectImage.left = 0;
			rectImage.right = sizeView.width;
		} else if (wDest < sizeView.width) {
			rectDst.left = rectView.left;
			rectDst.right = rectView.right;
			rectImage.left = (sizeView.width - wDest)/2;
			rectImage.right = rectImage.left + wDest;
		}

		if (hDest >= sizeView.height) {
			rectDst.top = rectView.top + (hDest - sizeView.height)/2;
			rectDst.bottom = rectDst.top + sizeView.height;
			rectImage.top = 0;
			rectImage.bottom = sizeView.height;
		} else if (hDest < sizeView.height) {
			rectDst.top = rectView.top;
			rectDst.bottom = rectView.bottom;
			rectImage.top = (sizeView.height - hDest)/2;
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
		} catch (...) {
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
}
