#include "pch.h"

#include "gtl/win_util/MatHelper.h"

namespace gtl::win_util {


	bool s_bGPUChecked {};
	bool s_bUseGPU {};

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
					return TRUE;
				} catch (...) {
					TRACE((GTL__FUNCSIG " - Error\n").c_str());
				}
			}
			//#endif
			cvtColor(imgSrc, imgDest, eCode);
		} catch (...) {
			return FALSE;
		}
		return TRUE;
	}

	bool ResizeImage(cv::Mat const& imgSrc, cv::Mat& imgDest, double dScale, int eInterpolation) {
		try {
			//#ifdef HAVE_CUDA
			if (s_bUseGPU) {
				try {
					cv::cuda::GpuMat dst;
					cv::resize(cv::cuda::GpuMat(imgSrc), dst, cv::Size(), dScale, dScale, eInterpolation);
					dst.download(imgDest);
					return TRUE;
				} catch (...) {
					TRACE((GTL__FUNCSIG " - Error\n").c_str());
				}
			}
			//#endif
			resize(imgSrc, imgDest, cv::Size(), dScale, dScale, eInterpolation);
		} catch (...) {
			return FALSE;
		}
		return TRUE;
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
					TRACE((GTL__FUNCSIG " - Error\n").c_str());
				}
			}
			//#endif
			matchTemplate(img, imgTempl, matResult, method);
		} catch (...) {
			return FALSE;
		}
		return TRUE;
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
					TRACE((GTL__FUNCSIG " - Error\n").c_str());
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

	bool MatToDC(cv::Mat const& _img, cv::Size const& sizeView, CDC& dc, CRect const& rect, CBitmap const& mask) {
		if (!mask.m_hObject)
			return MatToDC(_img, sizeView, dc, rect);

		cv::Mat img;
		if (_img.type() == CV_8UC3)
			img = _img;
		else if (_img.type() == CV_8UC1)
			ConvertColor(_img, img, cv::COLOR_GRAY2BGR);
		else
			return false;

		int cx = AdjustAlign32(img.cols);
		int cy = img.rows;

		try {
			int nChannel = 3;
			CRect rectSrc(0, 0, sizeView.width, sizeView.height);
			CRect rectDst(rect);
			CalcViewPosition(sizeView, rect, rectSrc, rectDst);

			CImage img2;
			img2.Create(cx, cy, 24, BI_RGB);
			CImage img3;
			img3.Create(cx, cy, 24, BI_RGB);
			int nBufferPitch = cx*nChannel;
			for (int y = 0; y < img.rows; y++)
				memcpy(img2.GetPixelAddress(0, y), img.ptr<cv::Vec3b>(y), nBufferPitch);
			img2.MaskBlt(img3.GetDC(), POINT(), mask, MAKEROP4(SRCCOPY, SRCAND));
			img3.ReleaseDC();
			img3.MaskBlt(dc, rectDst, CPoint(0, 0), mask, CPoint(0, 0), MAKEROP4(SRCCOPY, SRCPAINT));

		} catch (...) {
			TRACE( (GTL__FUNCSIG " - Unknown Error...").c_str() );
			return false;
		}
		return true;
	}


	bool MatToDC(cv::Mat const& img, cv::Size const& sizeEffective, CDC& dc, CRect const& rectTarget) {

		auto const type = img.type();

		int pixel_size = 1;
		if (type == CV_8UC3) {
			pixel_size = 3;
		} else if (type == CV_8UC1) {
			pixel_size = 1;
		} else {
			return false;
		}

		BYTE const* pImage = nullptr;
		int cx = gtl::AdjustAlign32(img.cols);
		int cy = img.rows;

		try {

			if (cx*pixel_size != img.step) {
				thread_local static std::vector<BYTE> buf;
				int nBufferPitch = cx *pixel_size;
				int row_size = img.cols * pixel_size;
				auto size = nBufferPitch * img.rows;
				if (buf.size() != size) {
					buf.assign(size, 0);
				}
				pImage = buf.data();
				auto * pos = buf.data();
				for (int y = 0; y < img.rows; y++) {
					memcpy(pos, img.ptr(y), row_size);
					pos += nBufferPitch;
				}
			} else {
				pImage = img.ptr<BYTE>(0);
			}

			BITMAPINFO bmpInfo;
			memset(&bmpInfo, 0, sizeof(bmpInfo));
			bmpInfo.bmiHeader.biSize = sizeof(bmpInfo);
			bmpInfo.bmiHeader.biWidth = cx;
			bmpInfo.bmiHeader.biHeight = -cy;
			bmpInfo.bmiHeader.biPlanes = 1;
			bmpInfo.bmiHeader.biBitCount = 8*pixel_size;
			bmpInfo.bmiHeader.biCompression = BI_RGB;
			bmpInfo.bmiHeader.biSizeImage = cx * cy * pixel_size;
			bmpInfo.bmiHeader.biXPelsPerMeter = 0;
			bmpInfo.bmiHeader.biYPelsPerMeter = 0;
			bmpInfo.bmiHeader.biClrUsed = 0;
			bmpInfo.bmiHeader.biClrImportant = 0;

			SetDIBitsToDevice(dc, 
								rectTarget.left, rectTarget.top, std::min(rectTarget.Width(), sizeEffective.width), std::min(rectTarget.Height(), sizeEffective.height), 
								0, 0, 0, img.rows, pImage, &bmpInfo, DIB_RGB_COLORS);

			pImage = nullptr;
		} catch (...) {
			TRACE(_T(__FUNCTION__) _T(" - Unknown Error..."));
			return false;
		}

		return true;
	}

	bool MatToDCTransparent(cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect, COLORREF crTransparent) {
		if (img.empty() || !dc.m_hDC || rect.IsRectEmpty())
			return false;

		CDC dcMem;
		dcMem.CreateCompatibleDC(&dc);
		CBitmap bmp;
		bmp.CreateCompatibleBitmap(&dc, sizeView.width, sizeView.height);
		dcMem.SelectObject(&bmp);
		CBrush brush(crTransparent);
		dcMem.FillRect(rect, &brush);
		MatToDC(img, sizeView, dcMem, CRect(0, 0, sizeView.width, sizeView.height));

		CRect rectSrc, rectDst;
		CalcViewPosition(sizeView, rect, rectSrc, rectDst);

		return dc.TransparentBlt(rectDst.left, rectDst.top, rectDst.Width(), rectDst.Height(), &dcMem, rectSrc.left, rectSrc.top, rectSrc.Width(), rectSrc.Height(), crTransparent) != FALSE;
	}

	bool MatToDCAlphaBlend(cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect, BLENDFUNCTION blend) {
		if (img.empty() || !dc.m_hDC || rect.IsRectEmpty())
			return FALSE;

		CDC dcMem;
		dcMem.CreateCompatibleDC(&dc);
		CBitmap bmp;
		bmp.CreateCompatibleBitmap(&dc, sizeView.width, sizeView.height);
		dcMem.SelectObject(&bmp);
		//CBrush brush(crTransparent);
		//dcMem.FillRect(rect, &brush);
		MatToDC(img, sizeView, dcMem, CRect(0, 0, sizeView.width, sizeView.height));

		CRect rectSrc, rectDst;
		CalcViewPosition(sizeView, rect, rectSrc, rectDst);

		return dc.AlphaBlend(rectDst.left, rectDst.top, rectDst.Width(), rectDst.Height(), &dcMem, rectSrc.left, rectSrc.top, rectSrc.Width(), rectSrc.Height(), blend) != FALSE;
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
		cr[0] = (BYTE)crTransparent[0];
		cr[1] = (BYTE)crTransparent[1];
		cr[2] = (BYTE)crTransparent[2];

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


	//=============================================================================
	//

}	// namespace gtl::win_util
