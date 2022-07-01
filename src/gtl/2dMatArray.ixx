﻿module;

// CMatView
#include "opencv2/opencv.hpp"

export module gtl:c2dMatArray;

import :concepts;
import :coord;
import :misc;
import :mat_helper;

namespace gtl {

	int imuldiv(int v, const std::pair<int, int>& scale) {
		int64_t r = v;
		r *= scale.first;
		r /= scale.second;
		return r;
	};

	double dmuldiv(double v, const std::pair<int, int>& scale) {
		return (v * scale.first) / scale.second;
	};

}

export namespace gtl {

	//---------------------------------------------------------------------------------------------------------------------------------
	// IMatImage
	//
	class IMatImage {
	public:
		bool m_bUseGPU = true;
	protected:
		int m_eScaleDownMethod = cv::INTER_AREA;
		int m_eScaleUpMethod = cv::INTER_LANCZOS4;

	public:
		IMatImage() {};
		virtual ~IMatImage() {};

	public:
		virtual bool empty() const					= 0;
		virtual int depth() const					= 0;
		virtual int type() const					= 0;
		virtual int channels() const				= 0;
		virtual int GetWidth() const				= 0;
		virtual int GetHeight() const				= 0;
		virtual cv::Size size() const { return cv::Size(GetWidth(), GetHeight()); }

		virtual cv::Mat GetROI(cv::Rect& rc) const { return GetResizedImage(rc, 0.0, -1, -1); }
		virtual cv::Mat GetResizedImage(cv::Rect& rc, double dScale, int eScaleDownMethod = -1, int eScaleUpMethod = -1) const = 0;

		template < typename T > T& at(const cv::Point& pt) { cv::Rect rc(pt, cv::Size(1, 1)); return GetROI(rc).at<T>(0, 0); };

	public:
		int GetScaleDownMethod() const { return m_eScaleDownMethod; }
		int GetScaleUpMethod() const { return m_eScaleUpMethod; }
		virtual void SetResizingMethod(int eScaleDownMethod = -1, int eScaleUpMethod = -1, bool bThumbnailInBkgnd = true) {
			if (eScaleUpMethod >= 0)
				m_eScaleUpMethod = eScaleUpMethod;
			if (eScaleDownMethod >= 0)
				m_eScaleDownMethod = eScaleDownMethod;
		}
	};


	//---------------------------------------------------------------------------------------------------------------------------------
	// C2dMatArray : Thumbnail Cache
	//
	class C2dMatArray : public IMatImage {

	public:
		struct T_ITEM {
			cv::Mat img;
			mutable TCopyTransparent<std::recursive_mutex> mtxThumbnail;
			//mutable bool bTumbnail = false;
			std::vector<cv::Mat> thumbnails;
		};
		using P_ITEM = T_ITEM*;
		using T_THUMBNAIL_SIZES = std::vector<std::pair<int, int>>;
		using F_NOTIFIER = std::function<bool(xPoint2i&, T_ITEM const&)>;

	protected:
		std::mutex m_mtxSet;
		std::vector<T_ITEM> m_set;
		cv::Mat m_imgWhole;
		xSize2i m_sizeArray;
		xSize2i m_sizeImage;
	protected:
		F_NOTIFIER m_funcNotifier;

	public:
		C2dMatArray() = default;
		C2dMatArray(C2dMatArray const&) = delete;
		C2dMatArray(C2dMatArray&&) = default;
		virtual ~C2dMatArray() {
			Destroy();
		}

		// Copies Matrix member only. NO ( Thumbnail Maker, Callback Function )
		C2dMatArray& operator = (const C2dMatArray& B);

		bool Create(const xSize2i& size);																						// 아무것도 없이 그냥 배열만 생성. 데이터 없음.
		bool Create(const cv::Mat& imgWhole, const xSize2i& sizePieceImage, const xSize2i& size);								// imgWhole 을 연결하여 생성.
		void Destroy();

		bool SetThumbnailMaker(const T_THUMBNAIL_SIZES& sizesThumbnail = { {1, 8}, {1, 16}, }, int nThreadThumbnailMaker = 0);	// Thumbnail Maker 가 동작중일때는 변경 안됨
		bool StartThumbnailMaker();
		bool StopThumbnailMaker();
		bool IsThumbnailMakerRunning() const { return m_threadsThumbnailWorker.size() ? true : false; }
		bool UpdateThumbnail(bool bThumbnailInBkgnd);
		bool UpdateThumbnail() { return UpdateThumbnail(IsThumbnailMakerRunning()); }

		const xSize2i& GetArraySize() const { return m_sizeArray; }
		bool SetPartialImage(const xPoint2i& pos, const cv::Mat& img, bool bCopy = false, bool bUpdateThumbnail = true, bool bThumbnailInBkgnd = true);

		void SetNotifier(F_NOTIFIER funcNotifier) { m_funcNotifier = funcNotifier; }
		void ResetNotifier()	{ m_funcNotifier = nullptr; }

	public:
		bool empty() const override		{ return (m_sizeImage.cx <= 0) || (m_sizeImage.cy <= 0); }
		int depth() const override		{ return m_set.size() ? m_set[0].img.depth() : 0; }
		int type() const override		{ return m_set.size() ? m_set[0].img.type() : 0; }
		int channels() const override	{ return m_set.size() ? m_set[0].img.channels() : 0; }
		int GetWidth() const override	{ return m_sizeImage.cx; }
		int GetHeight() const override	{ return m_sizeImage.cy; }
		cv::Size size() const override	{ return cv::Size(m_sizeImage.cx, m_sizeImage.cy); }

		void SetResizingMethod(int eScaleDownMethod = -1, int eScaleUpMethod = -1, bool bThumbnailInBkgnd = true);

	public:
		xPoint2i GetPos(int iIndex) const;
		T_ITEM& GetItem(const xPoint2i& pos);
		const T_ITEM& GetItem(const xPoint2i& pos) const;

	public:
		cv::Mat GetMergedImage() const { cv::Rect rc(cv::Point(0, 0), size()); return GetROI(rc); }
		cv::Mat GetROI(cv::Rect& rc) const override { return GetResizedImage(rc, 0.0, -1, -1); }
		cv::Mat GetResizedImage(cv::Rect& rc, double dScale, int eScaleDownMethod = -1, int eScaleUpMethod = -1) const override;

	protected:
		std::atomic<bool> m_bImageReady{};
		std::stop_source m_stopThumbnail;
		std::mutex m_mtxThumbnail;
		std::condition_variable m_cvThumbnail;
		//bool m_bStopThumbnailMaker = false;
		T_THUMBNAIL_SIZES m_sizesThumbnail;
		int m_nThreadThumbnailMaker = 0;
		std::mutex m_mtxThumbnailWork;
		std::vector<std::unique_ptr<std::jthread>> m_threadsThumbnailWorker;
		std::deque<std::unique_ptr<std::pair<xPoint2i, P_ITEM>>> m_dequeThumbnailWork;
		void ThumbnailMaker(std::stop_token token);
		bool MakeThumbnail(T_ITEM& item);

		bool Resize(cv::Mat& imgSrc, cv::Mat& imgDest, const cv::Size& size, int eResizingMethod);
	};

//}	// export namespace gtl
//namespace gtl {

	//=============================================================================
	//

	C2dMatArray& C2dMatArray::operator = (const C2dMatArray& B) {
		if (this == &B) return *this;
		StopThumbnailMaker();
		if (B.m_imgWhole.empty()) {
			m_set.clear();
			for (const auto& set : B.m_set) {
				m_set.emplace_back();
				m_set.back().img = set.img;
			}
			m_sizeArray		= B.m_sizeArray;
			m_sizeImage		= B.m_sizeImage;
		} else {
			Create(B.m_imgWhole, xSize2i(B.m_sizeImage.cx / B.m_sizeArray.cx, B.m_sizeImage.cy / B.m_sizeArray.cy), B.m_sizeArray);
		}
		return *this;
	}

	bool C2dMatArray::Create(const xSize2i& size) {
		Destroy();

		if ( (size.cx <= 0) || (size.cy <= 0) )
			return false;

		m_set.clear();
		m_sizeArray = size;
		if ( (size.cx > 0) && (size.cy > 0) ) {
			int nSize = size.cx * size.cy;
			m_set.assign(nSize, T_ITEM());
		}
		return true;
	}

	bool C2dMatArray::Create(const cv::Mat& imgWhole, const xSize2i& sizePieceImage, const xSize2i& size) {
		Destroy();

		if ( (imgWhole.empty()) || (size.cx <= 0) || (size.cy <= 0) )
			return false;

		m_set.clear();
		m_sizeArray.SetZero();
		m_sizeImage.SetZero();
		m_imgWhole.release();

		m_sizeArray = size;

		int nSize = size.cx * size.cy;
		if (nSize) {
			m_imgWhole = imgWhole;
			if (m_imgWhole.empty()) {
				return false;
			}
			if ( (imgWhole.cols < (size.cx-1)*sizePieceImage.cx) || (imgWhole.rows < (size.cy-1)*sizePieceImage.cy) )
				return false;

			m_set.assign(nSize, T_ITEM());
			int nRowsLeft = imgWhole.rows;
			for (int y = 0; y < size.cy; y++) {
				int cy = std::min(nRowsLeft, sizePieceImage.cy);
				if (y == size.cy-1)
					cy = nRowsLeft;
				nRowsLeft -= cy;

				int nColsLeft = imgWhole.cols;
				for (int x = 0; x < size.cx; x++) {
					int cx = std::min(nColsLeft, sizePieceImage.cx);
					if (x == size.cx-1)
						cx = nColsLeft;
					nColsLeft -= cx;

					auto& item = GetItem(xPoint2i(x, y));
					//item.bTumbnail = false;
					cv::Rect rc(x * sizePieceImage.cx, y * sizePieceImage.cy, cx, cy);
					item.img = m_imgWhole(rc);
				}
			}
			m_sizeImage = m_imgWhole.size();
		}
		return true;
	}

	void C2dMatArray::Destroy() {
		StopThumbnailMaker();
		m_sizeArray.SetZero();
		m_sizeImage.SetZero();
		m_dequeThumbnailWork.clear();
		m_set.clear();
		m_imgWhole.release();
	}

	bool C2dMatArray::SetThumbnailMaker(const T_THUMBNAIL_SIZES& sizesThumbnail, int nThreadThumbnailMaker) {
		if (m_threadsThumbnailWorker.size()) {
			return false;
		}
		m_sizesThumbnail = sizesThumbnail;
		m_nThreadThumbnailMaker = nThreadThumbnailMaker;
		return true;
	}
	bool C2dMatArray::StartThumbnailMaker() {
		StopThumbnailMaker();

		//m_bStopThumbnailMaker = false;
		//m_evtThumbnail.ResetEvent();
		m_bImageReady = false;
		for (int i = 0; i < m_nThreadThumbnailMaker; i++)
			m_threadsThumbnailWorker.emplace_back(new std::jthread( [&](std::stop_token token) { ThumbnailMaker(token); }, m_stopThumbnail.get_token()));

		return true;
	}
	bool C2dMatArray::StopThumbnailMaker() {
		m_stopThumbnail.request_stop();
		m_cvThumbnail.notify_all();
		for (const auto& rThread : m_threadsThumbnailWorker) {
			rThread->join();
		}
		m_threadsThumbnailWorker.clear();
		return false;
	}

	bool C2dMatArray::UpdateThumbnail(bool bThumbnailInBkgnd) {
		if (bThumbnailInBkgnd) {
			{
				std::lock_guard lock(m_mtxThumbnailWork);
				m_dequeThumbnailWork.clear();

				for (int i = 0; i < m_set.size(); i++) {
					auto& item = m_set[i];
					{
						std::lock_guard<std::recursive_mutex> lock(item.mtxThumbnail);
						item.thumbnails.clear();
					}
					m_dequeThumbnailWork.emplace_back(new std::pair<xPoint2i, P_ITEM>(GetPos(i), &item));
				}
				m_bImageReady = true;
				m_cvThumbnail.notify_all();
			}

			if (!IsThumbnailMakerRunning()) {
				StartThumbnailMaker();
			}

		} else {
			for (auto& item : m_set)
				MakeThumbnail(item);
		}
		return false;
	}

	//bool C2dMatArray::UpdateThumbnail(int nThreadThumbnailMaker) {
	//	return false;
	//}

	bool C2dMatArray::SetPartialImage(const xPoint2i& pos, const cv::Mat& img, bool bCopy, bool bUpdateThumbnail, bool bThumbnailInBkgnd) {
		T_ITEM& item = GetItem(pos);

		if (!m_imgWhole.empty()) {
			if ( (img.size() != item.img.size()) || (img.elemSize() != item.img.elemSize()) )
				return false;
			//if ( (img.size() != item.img.size()) || (img.type() != item.img.type()) )
			//	return false;
			bCopy = true;
		}

		{
			std::lock_guard<std::recursive_mutex> lock(item.mtxThumbnail);
			if (bCopy) {
				img.copyTo(item.img);
			} else {
				item.img = img;
			}
			item.thumbnails.clear();
		}

		if (m_imgWhole.empty()) {
			int nTotalCols = 0;
			for (int ix = 0; ix < m_sizeArray.cx; ix++) {
				int iIndex = ix + m_sizeArray.cx * pos.y;
				if (iIndex >= m_set.size())
					continue;
				nTotalCols += m_set[iIndex].img.cols;
			}
			int nTotalRows = 0;
			for (int iy = 0; iy < m_sizeArray.cy; iy++) {
				int iIndex = pos.x + m_sizeArray.cx * iy;
				if (iIndex >= m_set.size())
					continue;
				nTotalRows += m_set[iIndex].img.rows;
			}
			m_sizeImage.Set(nTotalCols, nTotalRows);
		}

		if (bUpdateThumbnail) {
			if (bThumbnailInBkgnd && m_threadsThumbnailWorker.size()) {
				std::lock_guard lock(m_mtxThumbnailWork);
				m_dequeThumbnailWork.emplace_back(new std::pair<xPoint2i, P_ITEM>(pos, &item));
				m_cvThumbnail.notify_all();
			} else
				MakeThumbnail(item);
		}

		return true;
	}

	void C2dMatArray::SetResizingMethod(int eScaleDownMethod, int eScaleUpMethod, bool bThumbnailInBkgnd) {
		if (eScaleDownMethod >= 0) {
			if (m_eScaleDownMethod != eScaleDownMethod) {
				m_eScaleDownMethod = eScaleDownMethod;
				UpdateThumbnail(bThumbnailInBkgnd);
			}
		}

		if (eScaleUpMethod >= 0)
			m_eScaleUpMethod = eScaleUpMethod;

	}

	xPoint2i C2dMatArray::GetPos(int iIndex) const {
		int x = iIndex % m_sizeArray.cx;
		int y = iIndex / m_sizeArray.cx;
		return xPoint2i(x, y);
	}

	C2dMatArray::T_ITEM& C2dMatArray::GetItem(const xPoint2i& pos) {
		int iIndex = pos.x + m_sizeArray.cx * pos.y;
		if ( (iIndex < 0) || (iIndex >= m_set.size()) || (pos.x >= m_sizeArray.cx) || (pos.y >= m_sizeArray.cy) ) {
			throw std::exception("Index Out of Position");
		}
		return m_set[iIndex];
	}

	const C2dMatArray::T_ITEM& C2dMatArray::GetItem(const xPoint2i& pos) const {
		int iIndex = pos.x + m_sizeArray.cx * pos.y;
		if ( (iIndex < 0) || (iIndex >= m_set.size()) || (pos.x >= m_sizeArray.cx) || (pos.y >= m_sizeArray.cy) ) {
			throw std::exception("Index Out of Position");
		}
		return m_set[iIndex];
	}

	cv::Mat C2dMatArray::GetResizedImage(cv::Rect& rc, double dScale, int eScaleDownMethod, int eScaleUpMethod) const {
		using namespace cv;

		if (dScale <= 0)
			dScale = 1;

		// Find Thumbnail
		int iThumbnail = -1;
		double dScaleThumbnail = 1;
		for (int i = 0; i < m_sizesThumbnail.size(); i++) {
			double dScaleC = dmuldiv(1., m_sizesThumbnail[i]);
			if ( (dScaleC >= dScale) && (dScaleC < dScaleThumbnail) ) {
				iThumbnail = i;
				dScaleThumbnail = dScaleC;
			}
		}
		auto scaleThumbnail = (iThumbnail < 0) ? std::pair<int, int>(1, 1) : m_sizesThumbnail[iThumbnail];

		if (eScaleUpMethod < 0)
			eScaleUpMethod = m_eScaleUpMethod;
		if (eScaleDownMethod < 0)
			eScaleDownMethod = m_eScaleDownMethod;

		Mat img;

		int ix = 0, iy = 0;
		int ny = 1;
		int nx = 1;
		int rowsPred = 0;
		int colsPred = 0;
		int iyEnd = 0;
		int ixEnd = 0;

		// Y Start
		for (iy = 0; iy < m_sizeArray.cy; iy++) {
			const Mat& m = GetItem({0, iy}).img;
			if (m.empty())
				continue;
			if (rowsPred > rc.y)
				return img;
			int r = rowsPred + m.rows;
			if (r > rc.y)
				break;
			rowsPred = r;
		}
		if (iy >= m_sizeArray.cy)
			return img;
		// Y End
		int rowsEnd = rowsPred;
		for (iyEnd = iy; iyEnd < m_sizeArray.cy; iyEnd++) {
			const Mat& m = GetItem({0, iyEnd}).img;
			rowsEnd += m.rows;
			if (rc.y + rc.height <= rowsEnd)
				break;
			ny++;
		}
		if (rc.y + rc.height > rowsEnd)
			rc.height = rowsEnd - rc.y;

		// X Start
		for (ix = 0; ix < m_sizeArray.cx; ix++) {
			const Mat& m = GetItem({ix, iy}).img;
			if (m.empty())
				continue;
			if (colsPred > rc.x)
				return img;
			int c = colsPred + m.cols;
			if (c > rc.x)
				break;
			colsPred = c;
		}
		if (ix >= m_sizeArray.cx)
			return img;
		// X End
		int colsEnd = colsPred;
		for (ixEnd = ix; ixEnd < m_sizeArray.cx; ixEnd++) {
			const Mat& m = GetItem({ixEnd, iy}).img;
			colsEnd += m.cols;
			if (rc.x + rc.width <= colsEnd)
				break;
			nx++;
		}
		if (rc.x + rc.width > colsEnd)
			rc.width = colsEnd - rc.x;

		// Get
		if ( (nx == 1) && (ny == 1) ) {
			auto& item = GetItem({ix, iy});

			Rect rcROI(rc);
			rcROI.x -= colsPred;
			rcROI.y -= rowsPred;

			if (iThumbnail >= 0) {
				std::lock_guard<std::recursive_mutex> lock(item.mtxThumbnail);
				if ( (iThumbnail < item.thumbnails.size()) && !item.thumbnails[iThumbnail].empty() ) {
					auto& thumbnail = item.thumbnails[iThumbnail];
					Rect rcNew(imuldiv(rcROI.x, scaleThumbnail), imuldiv(rcROI.y, scaleThumbnail), imuldiv(rcROI.width, scaleThumbnail), imuldiv(rcROI.height, scaleThumbnail));
					rcNew = gtl::GetSafeROI(rcNew, thumbnail.size());
					double dScaleNew = dScale / dScaleThumbnail;
					if (dScaleNew == 1.0)
						img = thumbnail(rcNew);
					else if (ResizeImage(thumbnail(rcNew), img, dScaleNew, dScaleNew >= 1 ? eScaleUpMethod : eScaleDownMethod))
						return img;
				}
			}
			{
				if (dScale == 1.0)
					img = item.img(rcROI);
				else
					ResizeImage(item.img(rcROI), img, dScale, dScale >= 1 ? eScaleUpMethod : eScaleDownMethod);
			}

		} else {

			auto MoveImages = [&](Mat& imgTarget, Rect& rcT, int iThumbnail) {
				int iRowStart = rcT.y - imuldiv(rowsPred, scaleThumbnail);
				int iRowStartTarget = 0;
				int nRowsLeft = rcT.height;
				for (int y = iy; y <= iyEnd; y++) {
					int iColStart = rcT.x - imuldiv(colsPred, scaleThumbnail);
					int iColStartTarget = 0;
					int nColsLeft = rcT.width;
					int cy = 0;
					for (int x = ix; x <= ixEnd; x++) {
						const auto& item = GetItem({x, y});

						static const Mat mEmpty;

						item.mtxThumbnail.lock();
						const Mat& m = (iThumbnail < 0) ? item.img : ((iThumbnail < item.thumbnails.size()) ? item.thumbnails[iThumbnail] : mEmpty);
						item.mtxThumbnail.unlock();

						if (m.empty())
							continue;
						int cx = std::min(m.cols-iColStart, nColsLeft);
						if (!cy)
							cy = std::min(m.rows-iRowStart, nRowsLeft);

						if ( (cx > 0) && (cy > 0) ) {
							try {
								Rect rcSource(iColStart, iRowStart, cx, cy);
								Rect rcTarget(iColStartTarget, iRowStartTarget, cx, cy);
								if ( IsROI_Valid(rcSource, m.size()) && IsROI_Valid(rcTarget, imgTarget.size()) )
									m(rcSource).copyTo(imgTarget(rcTarget));
								else
									throw std::exception("Unknown Internal Error");
							} catch (cv::Exception& ) {
								return;
							}
						} else {
							if (cx < 0 || cy < 0)
								throw std::exception("Unknown Internal Error");
						}

						iColStart = 0;
						iColStartTarget += cx;
						nColsLeft -= cx;
					}
					iRowStart = 0;
					iRowStartTarget += cy;
					nRowsLeft -= cy;
				}
			};

			if (iThumbnail >= 0) {
				Rect rcT( int(rc.x*dScaleThumbnail), int(rc.y*dScaleThumbnail), int(rc.width*dScaleThumbnail), int(rc.height*dScaleThumbnail));
				Size szT(imuldiv(rc.width, scaleThumbnail), imuldiv(rc.height, scaleThumbnail));
				//if (!CheckROI(rcT, szT))
				//	return img;
				Mat imgT;
				imgT = Mat::zeros(rcT.size(), GetItem({ix, iy}).img.type());

				MoveImages(imgT, rcT, iThumbnail);

				double dScaleNew = dScale / dScaleThumbnail;
				ResizeImage(imgT, img, dScaleNew, dScaleNew >= 1 ? eScaleUpMethod : eScaleDownMethod);
			} else {
				if (m_imgWhole.empty()) {
					img = Mat::zeros(rc.size(), GetItem({ix, iy}).img.type());
					MoveImages(img, rc, iThumbnail);
				} else {
					img = m_imgWhole(rc);
				}
				double dScaleNew = dScale / dScaleThumbnail;
				ResizeImage(img, img, dScaleNew, dScaleNew >= 1 ? eScaleUpMethod : eScaleDownMethod);
			}
		}

		return img;
	}

	void C2dMatArray::ThumbnailMaker(std::stop_token token) {
		using namespace std::literals;
		//static std::atomic<long> iThreadNumber {};
		//long iCurrentThread = iThreadNumber.fetch_add(1);
		//TRACE("ThumbnailMaker Start(%d)\n", iCurrentThread);

		std::unique_lock lock(m_mtxThumbnail);
		while (m_cvThumbnail.wait_for(lock, 1000ms, [&]{return (bool)m_bImageReady;}), !token.stop_requested()) {	// NOT '&&' but ','
			std::unique_ptr<std::pair<xPoint2i, P_ITEM>> rItemL;
			int nThumbnail = 0;
			while (!token.stop_requested()) {
				std::unique_ptr<std::pair<xPoint2i, P_ITEM>> rItem;
				{
					std::lock_guard<std::mutex> lock(m_mtxThumbnailWork);
					if (m_dequeThumbnailWork.size()) {
						rItem = move(m_dequeThumbnailWork.front());
						m_dequeThumbnailWork.pop_front();
					}
					if (m_dequeThumbnailWork.empty())
						m_bImageReady = false;
				}
				if (!rItem)
					break;

				MakeThumbnail(*(rItem->second));

				rItemL = move(rItem);

				if (nThumbnail > 3) {
					if (rItemL && !token.stop_requested() && m_funcNotifier) {
						m_funcNotifier(rItemL->first, *(rItemL->second));
						rItemL.reset();
					}
				}
			}
			if (rItemL && !token.stop_requested() && m_funcNotifier) {
				m_funcNotifier(rItemL->first, *(rItemL->second));
			}
		};

		//TRACE("ThumbnailMaker End -- (%d)\n", iCurrentThread);
	}

	bool C2dMatArray::MakeThumbnail(T_ITEM& item) {
		{
			std::lock_guard<std::recursive_mutex> lock(item.mtxThumbnail);

			//item.bTumbnail = false;
			item.thumbnails.clear();
			for (const auto& size : m_sizesThumbnail) {
				cv::Mat imgThumbnail;
				cv::Size sizeThumbnail(imuldiv(item.img.cols, size), imuldiv(item.img.rows, size));
				double dScale = (double)size.first / size.second;
				int eResizingMethod = dScale < 1 ? m_eScaleDownMethod : m_eScaleUpMethod;

				Resize(item.img, imgThumbnail, sizeThumbnail, eResizingMethod);

				item.thumbnails.emplace_back(imgThumbnail);
			}
			//item.bTumbnail = true;
		}

		return false;
	}

	bool C2dMatArray::Resize(cv::Mat& imgSrc, cv::Mat& imgDest, const cv::Size& size, int eResizingMethod) {
		//if (m_bUseGPU && IsGPUEnabled()) {
		//	try {
		//		cv::cuda::GpuMat dst;
		//		cv::cuda::resize(cv::cuda::GpuMat(imgSrc), dst, size, .0, .0, eResizingMethod);
		//		dst.download(imgDest);
		//	} catch (...) {
		//		TRACE(__TFMSG(" - Error\n"));
		//	}
		//}

		try {
			resize(imgSrc, imgDest, size, 0.0, 0.0, eResizingMethod);
		} catch (cv::Exception&) {
			return false;
		}
		return true;
	}

}	// namespace gtl
