#pragma once

// CMatView

#include "gtl/coord.h"
#include "gtl/json_proxy.h"
#include "gtl/2dMatArray.h"
#include "gtl/mfc/_lib_gtl_mfc.h"
#include "gtl/mfc/mfc.h"

namespace gtl::win::inline mfc {
#pragma pack(push, 8)

    //! @brief mat view
    class GTL__MFC_CLASS CMatView : public CWnd {
		DECLARE_DYNAMIC(CMatView)

	public:
		enum eMOUSE_FUNCTION : int { MF_NONE, MF_DRAG_IMAGE, MF_SELECT_REGION, MF_ZOOM_IN, MF_ZOOM_OUT, MF_SCROLL_UP, MF_SCROLL_DOWN };
		enum eNOTIFY_CODE : uint {
			NC_SETFOCUS = NM_LAST-2000U,
			NC_KILLFOCUS,
			NC_SEL_REGION,

			// m_bNotifyAllMouseEvent = true 설정한 다음에... ptWindow, ptImage, ptReal 만 valid.
			NC_MOUSE_MOVE,
			NC_LBUTTON_DOWN,
			NC_LBUTTON_UP,
			NC_RBUTTON_DOWN,
			NC_RBUTTON_UP,
			NC_MBUTTON_DOWN,
			NC_MBUTTON_UP,
			NC_KEY_DOWN,
		};
		struct NOTIFY_DATA {
			NMHDR hdr;
			xRect2d rect;
			xRect2d rectReal;
			cv::Rect rcImage;

			CPoint ptWindow;		// 마우스 Window 좌표
			xPoint2d ptImage;		// 현재 마우스 포지션
			xPoint2d ptReal;		// 현재 마우스 포지션
		};
		enum eBLIT_METHOD { BM_TRANSPARENT, BM_ALPHABLEND, } ;

	public:
		// Setting
		class CSetting {
		public:
			bool bShowTool = true;								// Toolbar 화면 표시

			int eScaleUpMethod = cv::INTER_CUBIC;				// 이미지 확대시 Interpolation 방법 설정
			int eScaleDownMethod = cv::INTER_AREA;				// 이미지 축소시 Interpolation 방법 설정
			bool b2ClickSelection = true;						// 이미지 영역 선택시, click 회수 설정. true : 2click, false : one-click
			bool bClickToMagnifySelectedRect = true;			// 선택 영역 클릭시 확대.
			bool bPatternMatching = true;						// Pattern Matching 창을 사용할지 결정. true : Toolbar 에 아이콘 표시
			bool bToolTipOnCursor = true;						// 구현 안됨
			bool bClearSelectionOnMouseButton = false;			// 마우스 버튼 클릭시 현재 선택된 영역 지우기
			bool bMapWindow = true;								// 전체 이미지중에서 현재 화면에 보이는 영역이 어느만큼인지 보여줌.
			bool bStatusOnBottom = false;						// Status Info 를 윈도우 아래에 배치
			bool bKeyboardNavigation{false};					// Keyboard Scroll (VK_UP/VK_DONW/VK_LEFT/VK_RIGHT)
			double dZoomRate = 1.1;								// Zoom In/Out 비율 선택
			int iJPGQuality = 95;								// JPG 이미지 저장시 품실
			int eAutoScrollMethod = 1;							// 0 : pgdn, 1 : space bottom, 2 : space top
			int tSmoothScroll{ 0 };								// Smooth Scroll Duration in ms

			void SyncJson(bool bStore, gtl::bjson<>& section);
			void SyncRegistry(bool bStore, LPCTSTR pszSection, CWinApp& app);
		};
		CString m_strRegistrySection;	// Load/Store Settings from/to ...

		const CSetting& GetSettings() const { return m_setting; }
		void SetSettings(const CSetting& setting) {
			m_setting = setting;
			UpdateTool();
			ShowTool(m_setting.bShowTool);
		}

		bool m_bNotifyAllMouseEvent = false;
		CSetting& GetSettings() { return m_setting; }	// Call this function before creating...

	protected:
		CSetting m_setting;

	protected:
		enum { eIDStart = 1010, /*eIDZoomLable = eIDStart, */
			eIDZoom = eIDStart, eIDZoomUp, eIDZoomDown, eIDZoomReset, eIDSave, eIDPatternMatching, eIDSettings, eIDInfo, /*eIDViewPositionBox, */
			eIDEnd};

	protected:
		// Toolbar
		CRect m_rectTool;
		CComboBox m_comboZoom;
		std::array<CMFCButton, 3> m_btnZoom;
		CMFCButton m_btnSave;
		CMFCButton m_btnTool;
		CMFCButton m_btnSetting;
		CEdit m_staticInfo;
		bool m_bDraggingProcessed = true;
		//CMatViewPatternMatchingDlg* m_pDlgPatternMatching;
		// View
		xCoordTrans2d m_ctI2S;	// Image to Screen
		std::unique_ptr<ICoordTrans> m_rCTI2M;
		std::array<bool, 3> m_bChannels;
		//cv::Mat m_imgOrg;
		C2dMatArray m_imgOrg;
		cv::Mat m_imgView;

		// Attributes
		struct T_ATTRIBUTE {
			CString strName;
			cv::Point ptPatch;				// Patch 위치
			CPoint ptTargetOffset;			// Dispatch 위치. Screen 좌표계
			cv::Mat img, imgView;
			eBLIT_METHOD bm;
			union {
				struct {
					COLORREF crTransparent;
				};
				struct {
					BLENDFUNCTION blend;
				};
			};
			bool operator == (LPCTSTR pszName) const { return strName.Compare(pszName) == 0; }
		};
		std::deque<std::unique_ptr<T_ATTRIBUTE>> m_imgsAttribute;

		//enum eMOUSE_MODE { MM_NONE, MM_ONECLICK, MM_TWOCLICK };
		struct {
			bool bDragMode = false;
			bool bSelectionMode = false;
			xPoint2d ptSelect0;
			xPoint2d ptSelect1;

			xPoint2d ptImgLast;
			xPoint2i ptScrollLast;
			xPoint2i ptLast;
			xPoint2i ptCurrent;
		} m_mouse;

		struct T_CROSS_MARK {
			CString strName;
			CString strLabel;
			bool bShow = false;
			xPoint2d pt;
			xSize2i size;
			int nPenStyle = PS_SOLID;
			int iThick = 1;
			COLORREF cr = RGB(255, 255, 255);
			int operator == (LPCTSTR pszName) const { return strName == pszName; }
		};
		struct T_RECT_REGION {
			CString strName;
			CString strLabel;
			bool bShow = false;
			xRect2d rect;
			int nPenStyle = PS_SOLID;
			int iThick = 1;
			COLORREF cr = RGB(255, 255, 255);
			int operator == (LPCTSTR pszName) const { return strName == pszName; }
		};
		struct {
			std::deque<std::unique_ptr<T_CROSS_MARK>> crosses;
			std::deque<std::unique_ptr<T_RECT_REGION>> rects;
			T_RECT_REGION rectCurrent;
		} m_display;
		struct {
			xPoint2d pt0, delta;
			std::chrono::steady_clock::time_point t0, t1;
		} m_smooth_scroll;
		inline static HCURSOR m_hCursorMag = nullptr;

	public:
		CMatView();   // standard constructor
		virtual ~CMatView();

		virtual BOOL PreTranslateMessage(MSG* pMsg);

		// operations
	public:
		bool InitView(double dZoom = -1);
		bool SetImage(cv::Mat const& img, double dZoom = -1, bool bCopyImage = false);
		bool SetImageCenter(xPoint2d const& pt = xPoint2d(-1, -1), double dZoom = -1, bool bDelayedUpdate = false);
		double GetZoom() { double dZoom = -1; CheckAndGetZoom(dZoom); return dZoom; }
		void SetZoom(double dZoom);
		void SetMouseFunction(eMOUSE_FUNCTION eMFLButton = MF_DRAG_IMAGE, eMOUSE_FUNCTION eMFMButton = MF_DRAG_IMAGE, eMOUSE_FUNCTION eMFRButton = MF_SELECT_REGION,
							  eMOUSE_FUNCTION eMFWheelUp = MF_SCROLL_UP, eMOUSE_FUNCTION eMFWheelDown = MF_SCROLL_DOWN, eMOUSE_FUNCTION eMFWheelUpCtrl = MF_ZOOM_IN, eMOUSE_FUNCTION eMFWheelDownCtrl = MF_ZOOM_OUT)
		{
			m_eMFLButton = eMFLButton; m_eMFMButton = eMFMButton; m_eMFRButton = eMFRButton; m_eMFWheelUp = eMFWheelUp; m_eMFWheelDown = eMFWheelDown; m_eMFWheelUpCtrl = eMFWheelUpCtrl; m_eMFWheelDownCtrl = eMFWheelDownCtrl;
		}
		void GetMouseFunction(eMOUSE_FUNCTION& eMFLButton, eMOUSE_FUNCTION& eMFMButton, eMOUSE_FUNCTION& eMFRButton,
							  eMOUSE_FUNCTION& eMFWheelUp, eMOUSE_FUNCTION& eMFWheelDown, eMOUSE_FUNCTION& eMFWheelUpCtrl, eMOUSE_FUNCTION& eMFWheelDownCtrl) const {
			eMFLButton = m_eMFLButton; eMFMButton = m_eMFMButton; eMFRButton = m_eMFRButton;
			eMFWheelUp = m_eMFWheelUp; eMFWheelDown = m_eMFWheelDown;
			eMFWheelUpCtrl = m_eMFWheelUpCtrl; eMFWheelDownCtrl = m_eMFWheelDownCtrl;
		}
		void ShowTool(bool bShow);
		bool IsToolVisible() const { return m_setting.bShowTool; }
		void UpdateTool();

		bool GetImage(cv::Mat& img) { if (m_imgOrg.empty()) return false; cv::Rect rc(0, 0, m_imgOrg.GetWidth(), m_imgOrg.GetHeight()); m_imgOrg.GetROI(rc).copyTo(img); return true; }
		bool GetImageView(cv::Mat& img) const { if (m_imgView.empty()) return false; m_imgView.copyTo(img); return true; }
		cv::Mat GetImage() { cv::Mat img; GetImage(img); return std::move(img); }
		bool GetCurrentImageRect(xRect2d& rect) const;
		bool SetCurrentImageRect(xRect2d const& rect);

		bool StartSelectionMode(xPoint2d const& ptImage);
		bool EndSelectionMode();
		bool GetSelectedImageRect(xRect2d& rect) const;
		bool SetSelectedImageRect(xRect2d const& rect, bool bSelect = true);

		//const cv::Mat& GetImage() const { return m_imgOrg; }
		C2dMatArray const& GetMatImage() { return m_imgOrg; }
		const cv::Mat& GetImageView() const { return m_imgView; }

		template < typename tcontainer >
		auto Find(tcontainer& container, LPCTSTR pszName) {
			return std::find_if(container.begin(), container.end(), [pszName](auto const& a){return a->strName == pszName;});
		}
		auto FindAttr(LPCTSTR pszName) {
			return std::find_if(m_imgsAttribute.begin(), m_imgsAttribute.end(), [pszName](auto const& a){return a->strName == pszName;});
		}
		T_ATTRIBUTE& GetAttr(LPCTSTR pszName) {
			auto pos = std::find_if(m_imgsAttribute.begin(), m_imgsAttribute.end(), [pszName](auto const& a){return a->strName == pszName;});
			if (pos == m_imgsAttribute.end()) {
				m_imgsAttribute.push_back(std::make_unique<T_ATTRIBUTE>());
				pos = m_imgsAttribute.end()-1;
			}
			return *(*pos);
		}
		bool AddAttributeLayer(LPCTSTR pszName, const cv::Mat& img, COLORREF crTransparent);
		bool AddAttributeLayer(LPCTSTR pszName, const cv::Mat& img, BLENDFUNCTION blend);
		bool AddAttributeLayer(LPCTSTR pszName, const cv::Point& ptLT, const cv::Mat& img, COLORREF crTransparent);
		bool AddAttributeLayer(LPCTSTR pszName, const cv::Point& ptLT, const cv::Mat& img, BLENDFUNCTION blend);
		bool SetAttributeLayer(LPCTSTR pszName, const cv::Mat& img);
		bool DeleteAttributeLayer(LPCTSTR pszName = NULL);

		bool ShowCrossMark(LPCTSTR pszName = NULL, bool bShow = true);
		bool AddCrossMark(LPCTSTR pszName, xPoint2d const& pt, LPCTSTR pszLabel = nullptr, int iSizeX = 5, int iSizeY = 5, int nPenStyle = PS_SOLID, int iThick = 3, COLORREF cr = RGB(255, 0, 0));
		bool DeleteCrossMark(LPCTSTR pszName = NULL);

		bool ShowRectRegion(LPCTSTR pszName = NULL, bool bShow = true);
		bool AddRectRegion(LPCTSTR pszName, const xRect2d& rect, LPCTSTR pszLabel = NULL, int nPenStyle = PS_DOT, int iThick = 3, COLORREF crPen = RGB(255, 255, 255));
		bool DeleteRectRegion(LPCTSTR pszName = NULL);

	protected:
		bool CheckAndGetZoom(double& dZoom);

		eMOUSE_FUNCTION m_eMFLButton{}, m_eMFMButton{}, m_eMFRButton{}, m_eMFWheelUp{}, m_eMFWheelDown{}, m_eMFWheelUpCtrl{}, m_eMFWheelDownCtrl{};
		bool DoMouseFunction(eMOUSE_FUNCTION eMF, bool bDown, UINT nFlags, CPoint point, short zDelta = 0);
		bool SelectRegion(xRect2d& rect);

		LRESULT NotifyEvent(eNOTIFY_CODE evtCode, const xRect2d& rect = xRect2d(), bool bImageRect = false);
		LRESULT NotifyMouseEvent(eNOTIFY_CODE evtCode, CPoint pt);

	public:
		void GetClientRect(LPRECT lpRect) const;
		xCoordTrans2d GetCT() const { return m_ctI2S; }
		void SetCT_I2M(std::unique_ptr<ICoordTrans> rCTI2M) { m_rCTI2M = std::move(rCTI2M); }
		void SetCT_I2M(const xCoordTrans2d& ctI2M) { m_rCTI2M.reset(new xCoordTrans2d(ctI2M)); }
		ICoordTrans* GetCT_I2M() { return m_rCTI2M.get(); }
		bool UpdateMousePosition(CPoint const& pt);
		bool UpdateTrackPosition(LPCRECT lpRect);
		bool UpdateToolTip(LPCTSTR psz);
		int GetScaleDownMethod() const { return m_imgOrg.GetScaleDownMethod(); }
		int GetScaleUpMethod() const { return m_imgOrg.GetScaleUpMethod(); }
		void SetResizingMethod(int eScaleDownMethod = cv::INTER_LINEAR, int eScaleUpMethod = cv::INTER_LINEAR, bool bThumbnailInBkgnd = false);

	protected:
		bool UpdateDisplayImage(double dZoom = -1, bool bDelayedUpdate = false);
		bool PrepareDisplayImage(cv::Mat const& imgOrg, cv::Mat& imgView, const cv::Point& ptPatch, CPoint& ptOffset, const CRect& rectClient, double dZoom = -1);
		bool PrepareDisplayImage(C2dMatArray const& imgSet, cv::Mat& imgView, CRect const& rectClient, double dZoom);
		bool UpdateScrollBars(double dZoom = -1);
		int OnScroll(int eBar, UINT nSBCode, UINT nPos);

	protected:
		DECLARE_MESSAGE_MAP()
	public:
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnDestroy();
		xPoint2d m_ptImageCenter;
		bool m_bFocusedBefore = false;
		enum eTIMER { T_UPDATE_DISPLAY_IMAGE = 1045, T_CHECK_FOCUS, T_SMOOTH_SCROLL };
		afx_msg void OnTimer(UINT_PTR nIDEvent);
		afx_msg void OnSize(UINT nType, int cx, int cy);

		//afx_msg void OnNcPaint();
		afx_msg void OnPaint();

		afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);

		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
		afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnSelchangeZoom();
		afx_msg void OnEditchangeZoom();
		afx_msg void OnZoomUp();
		afx_msg void OnZoomDown();
		afx_msg void OnZoomReset();
		afx_msg void OnSave();
		afx_msg void OnPatternMatching();
		//afx_msg void OnSettings();
		//afx_msg void OnSelchangeScaleDownMethod();
		//afx_msg void OnSelchangeScaleUpMethod();
		afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
		afx_msg LRESULT OnUpdateDisplayImage(WPARAM wParam, LPARAM lParam);

    };


#pragma pack(pop)
}
