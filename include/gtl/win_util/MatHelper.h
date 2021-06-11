/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MatHelper.h : cv::Mat, CDC,...
//
// PWH.
//
// 2021.06.11 from Mocha xMathUtil
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gtl/win_util/win_util.h"
#include "opencv2/opencv.hpp"


namespace gtl::win_util {
#pragma pack(push, 8)	// default align. (8 bytes)


	//-----------------------------------------------------------------------------
	// Mat Load/Store
	template < class ARCHIVE >
	bool LoadMat(ARCHIVE& ar, cv::Mat& mat) {
		CStringA str;
		ar >> str;
		if (str != "mat")
			return FALSE;

		int rows, cols, type;
		ar >> rows >> cols >> type;
		if ( (rows < 0) || (cols < 0) )
			return FALSE;
		if ( (rows == 0) || (cols == 0) ) {
			mat.release();
			return TRUE;
		}
		mat = cv::Mat::zeros(rows, cols, type);
		if (mat.empty())
			return FALSE;
		for (int i = 0; i < rows; i++) {
			ar.read((char*)mat.ptr(i), (UINT)(mat.cols*mat.elemSize()));	// step -> (cols*elemSize())
		}
		return TRUE;
	}
	template < class ARCHIVE >
	bool StoreMat(ARCHIVE& ar, cv::Mat const& mat) {
		gtl::CStringA str("mat");
		ar << str;

		ar << mat.rows;
		ar << mat.cols;
		ar << mat.type();
		for (int i = 0; i < mat.rows; i++) {
			ar.write((char*)mat.ptr(i), mat.cols*mat.elemSize());	// step -> (cols*elemSize())
		}

		return true;
	}
	inline static bool ReadMat(std::istream& file, cv::Mat& mat) {
		auto ReadVar = [&file, &mat] (auto& var) -> bool {
			return (bool)file.read((char*)&var, sizeof(var));
		};

		do {
			int rows{}, cols{}, type{};
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
				file.read((char*)mat.ptr(i), (mat.cols*mat.elemSize()));	// step -> (cols*elemSize())

		} while (false);

		return true;
	}
	inline static bool SaveMat(std::ostream& file, cv::Mat const& mat) {
		uint8_t buf[4] = { 3, 'm', 'a', 't' };
		file.write((char const*)buf, sizeof(buf));
		file.write((char const*)&mat.cols, sizeof(mat.cols));
		file.write((char const*)&mat.rows, sizeof(mat.rows));
		int type = mat.type();
		file.write((char const*)&type, sizeof(type));
		for (int i = 0; i < mat.rows; i++) {
			file.write((char*)mat.ptr(i), mat.cols*mat.elemSize());	// step -> (cols*elemSize())
		}

		return true;
	}


	//-----------------------------------------------------------------------------
	// ¿Ü°û¼± ±ÛÀÚ
	template <class TSTR >
	void putTextC(cv::InputOutputArray img, TSTR& psz, cv::Point org,
				  int fontFace, double fontScale, cv::Scalar color,
				  int thickness = 1, int lineType = 8,
				  bool bottomLeftOrigin = false, bool bOutline = true)
	{
		CStringA str(psz);
		cv::Size size = cv::getTextSize((LPCSTR)str, fontFace, fontScale, thickness, NULL);

		if (bOutline) {
			cv::Scalar crBkgnd;
			crBkgnd = cv::Scalar(255, 255, 255) - color;
			//if (crBkgnd == Scalar(0, 0, 0))
			//	crBkgnd = Scalar(1, 1, 1);
			int iShift = std::max(1, thickness/2);
			cv::putText(img, (LPCSTR)str, CPoint2i(org)-CSize2i(size)/2+CPoint2i(0, iShift), fontFace, fontScale, crBkgnd, thickness, lineType, bottomLeftOrigin);
			cv::putText(img, (LPCSTR)str, CPoint2i(org)-CSize2i(size)/2+CPoint2i(iShift, 0), fontFace, fontScale, crBkgnd, thickness, lineType, bottomLeftOrigin);
			cv::putText(img, (LPCSTR)str, CPoint2i(org)-CSize2i(size)/2+CPoint2i(-iShift, 0), fontFace, fontScale, crBkgnd, thickness, lineType, bottomLeftOrigin);
			cv::putText(img, (LPCSTR)str, CPoint2i(org)-CSize2i(size)/2+CPoint2i(0, -iShift), fontFace, fontScale, crBkgnd, thickness, lineType, bottomLeftOrigin);
		}

		cv::putText(img, (LPCSTR)str, CPoint2i(org)-CSize2i(size)/2, fontFace, fontScale, color, thickness, lineType, bottomLeftOrigin);
	}


	//-----------------------------------------------------------------------------
	// Mat 
	bool GTL_WINUTIL_API CheckGPU(bool bUse);
	bool GTL_WINUTIL_API IsGPUEnabled();
	bool GTL_WINUTIL_API ConvertColor(cv::Mat const& imgSrc, cv::Mat& imgDest, int eCode);
	bool GTL_WINUTIL_API ResizeImage(cv::Mat const& imgSrc, cv::Mat& imgDest, double dScale, int eInterpolation = cv::INTER_LINEAR/*, int eScaleDownMethod = cv::INTER_AREA*/);
	bool GTL_WINUTIL_API MatchTemplate(cv::Mat const& img, cv::Mat const& imgTempl, cv::Mat& matResult, int method);
	bool GTL_WINUTIL_API MatchTemplate(cv::Mat const& img, cv::Mat const& imgTempl, int method, CPoint2d& ptBest, double& dMinMax, double& dRate, double dScale = 0.0, int eInterpolation = cv::INTER_LINEAR);

	//-----------------------------------------------------------------------------
	// Mat to DC
	bool GTL_WINUTIL_API CalcViewPosition(cv::Size const& sizeView, CRect const& rectView, CRect& rectImage/* out */, CRect& rectDst/* out */);	// image size -> display rect and source rect
	bool GTL_WINUTIL_API MatToDC            (cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect);
	bool GTL_WINUTIL_API MatToDC            (cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect, CBitmap const& mask);	// mask : monochrome bitmap, background of img value must be zero.
	bool GTL_WINUTIL_API MatToDCTransparent (cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect, COLORREF crTransparent);
	bool GTL_WINUTIL_API MatToDCAlphaBlend  (cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect, BLENDFUNCTION blend);
	bool GTL_WINUTIL_API MatToMatTransparent(cv::Mat const& imgSource, cv::Mat& imgTarget, cv::Mat const& matMask);
	bool GTL_WINUTIL_API MatToMatTransparent(cv::Mat const& imgSource, cv::Mat& imgTarget, cv::Scalar const& crTransparent);


#pragma pack(pop)
}
