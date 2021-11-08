#pragma once

// CMatView

#include "gtl/concepts.h"
#include "coord.h"
#include "misc.h"

#include "opencv2/opencv.hpp"

namespace gtl {
	#pragma pack(push, 8)


	//---------------------------------------------------------------------------------------------------------------------------------
	// IMatImage
	//
	class GTL__CLASS IMatImage {
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
	class GTL__CLASS C2dMatArray : public IMatImage {

	public:
		struct T_ITEM {
			cv::Mat img;
			mutable non_copyable_member<std::recursive_mutex> mtxThumbnail;
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

	#pragma pack(pop)
}
