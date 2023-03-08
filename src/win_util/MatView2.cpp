// MatView.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "MatView.h"
#include "MatHelper.h"
#include "misc/xUtil.h"
#include "misc/TMDialog.h"
#include "MatViewPatternMatchingDlg.h"
#include "MatViewSettingDlg.h"

#include <VersionHelpers.h>

enum {
	WM_UPDATE_DISPLAY_IMAGE = WM_APP+30,

};

#define dZoomRateWParam 10'000.

using namespace cv;

// CMatView dialog

static const bool g_bWindows7OrGreater = IsWindows7OrGreater();
extern HINSTANCE g_hInstance;

void CMatView::CSetting::SyncProfile(bool bStore, CProfileSection& section) {
	section.SyncItemValueBoolean(bStore, _T("ShowTool"), bShowTool);

	section.SyncItemValue(bStore, _T("ScaleUpMethod"), eScaleUpMethod);
	section.SyncItemValue(bStore, _T("ScaleDownMethod"), eScaleDownMethod);
	section.SyncItemValueBoolean(bStore, _T("2ClickSelection"), b2ClickSelection);
	section.SyncItemValueBoolean(bStore, _T("ClickToMagnifySelectedRect"), bClickToMagnifySelectedRect);
	section.SyncItemValueBoolean(bStore, _T("PatternMatching"), bPatternMatching);
	section.SyncItemValueBoolean(bStore, _T("TooltipOnCursor"), bToolTipOnCursor);
	section.SyncItemValueBoolean(bStore, _T("ClearSelectionOnMouseButton"), bClearSelectionOnMouseButton);
	section.SyncItemValueBoolean(bStore, _T("MapWindow"), bMapWindow);
	section.SyncItemValueBoolean(bStore, _T("StatusOnBottom"), bStatusOnBottom);
	section.SyncItemValue(bStore, _T("ZoomRate"), dZoomRate);
	section.SyncItemValue(bStore, _T("JPGQuality"), iJPGQuality);
	section.SyncItemValue(bStore, _T("KeyboardNavigation"), bKeyboardNavigation);
	section.SyncItemValue(bStore, _T("AutoScrollMethod"), eAutoScrollMethod);
	section.SyncItemValue(bStore, _T("SmoothScrollDuration"), tSmoothScroll);
}
void CMatView::CSetting::SyncRegistry(bool bStore, LPCTSTR pszSection, CWinApp& app) {
	if (bStore) {
		app.WriteProfileInt(pszSection, _T("ShowTool"), bShowTool);
		app.WriteProfileInt(pszSection, _T("ScaleUpMethod"), eScaleUpMethod);
		app.WriteProfileInt(pszSection, _T("ScaleDownMethod"), eScaleDownMethod);
		app.WriteProfileInt(pszSection, _T("2ClickSelection"), b2ClickSelection);
		app.WriteProfileInt(pszSection, _T("ClickToMagnifySelectedRect"), bClickToMagnifySelectedRect);
		app.WriteProfileInt(pszSection, _T("PatternMatching"), bPatternMatching);
		app.WriteProfileInt(pszSection, _T("TooltipOnCursor"), bToolTipOnCursor);
		app.WriteProfileInt(pszSection, _T("ClearSelectionOnMouseButton"), bClearSelectionOnMouseButton);
		app.WriteProfileInt(pszSection, _T("MapWindow"), bMapWindow);
		app.WriteProfileInt(pszSection, _T("StatusOnBottom"), bStatusOnBottom);
		app.WriteProfileString(pszSection, _T("ZoomRate"), Format(_T("%g"), dZoomRate));
		app.WriteProfileInt(pszSection, _T("JPGQuality"), iJPGQuality);
		app.WriteProfileInt(pszSection, _T("KeyboardNavigation"), bKeyboardNavigation);
		app.WriteProfileInt(pszSection, _T("AutoScrollMethod"), eAutoScrollMethod);
		app.WriteProfileInt(pszSection, _T("SmoothScrollDuration"), tSmoothScroll);
	} else {
		bShowTool						= app.GetProfileInt(pszSection, _T("ShowTool"), bShowTool);
		eScaleUpMethod					= app.GetProfileInt(pszSection, _T("ScaleUpMethod"), eScaleUpMethod);
		eScaleDownMethod				= app.GetProfileInt(pszSection, _T("ScaleDownMethod"), eScaleDownMethod);
		b2ClickSelection				= app.GetProfileInt(pszSection, _T("2ClickSelection"), b2ClickSelection);
		bClickToMagnifySelectedRect		= app.GetProfileInt(pszSection, _T("ClickToMagnifySelectedRect"), bClickToMagnifySelectedRect);
		bPatternMatching				= app.GetProfileInt(pszSection, _T("PatternMatching"), bPatternMatching);
		bToolTipOnCursor				= app.GetProfileInt(pszSection, _T("TooltipOnCursor"), bToolTipOnCursor);
		bClearSelectionOnMouseButton	= app.GetProfileInt(pszSection, _T("ClearSelectionOnMouseButton"), bClearSelectionOnMouseButton);
		bMapWindow						= app.GetProfileInt(pszSection, _T("MapWindow"), bMapWindow);
		bStatusOnBottom					= app.GetProfileInt(pszSection, _T("StatusOnBottom"), bStatusOnBottom);
		CString str 					= app.GetProfileString(pszSection, _T("ZoomRate"), Format(_T("%g"), dZoomRate));
		dZoomRate = _ttof(str);
		if (dZoomRate <= 1.0)
			dZoomRate = 1.1;
		iJPGQuality						= app.GetProfileInt(pszSection, _T("JPGQuality"), iJPGQuality);
		bKeyboardNavigation				= app.GetProfileInt(pszSection, _T("KeyboardNavigation"), bKeyboardNavigation);
		eAutoScrollMethod				= app.GetProfileInt(pszSection, _T("AutoScrollMethod"), eAutoScrollMethod);
		tSmoothScroll					= app.GetProfileInt(pszSection, _T("SmoothScrollDuration"), tSmoothScroll);
	}
}


HCURSOR CMatView::m_hCursorMag = nullptr;

IMPLEMENT_DYNAMIC(CMatView, CWnd)

CMatView::CMatView() : CWnd() {
	m_mouse.bDragMode = FALSE;

	SetMouseFunction();

	UseGPU(TRUE);

	m_pDlgPatternMatching = nullptr;

	if (!m_hCursorMag)
		m_hCursorMag = LoadCursor(g_hInstance, _T("IDC_MAGNIFYING"));

	m_imgOrg.SetNotifier([&](CPoint2i& pt, const C2dMatArray::T_ITEM& item) -> bool {
		int nRetry = 3;
		for (int i = 0; i < nRetry; i++) {
			if (PostMessage(WM_UPDATE_DISPLAY_IMAGE, (WPARAM)(intptr_t)(-1 * dZoomRateWParam), (LPARAM)true))
				return true;
		}
		return false;
	});
}

CMatView::~CMatView() {

	m_imgOrg.ResetNotifier();

	if (m_pDlgPatternMatching)
		delete m_pDlgPatternMatching;
}

//void CMatView::DoDataExchange(CDataExchange* pDX) {
//	CWnd::DoDataExchange(pDX);
//}

BEGIN_MESSAGE_MAP(CMatView, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//ON_WM_NCPAINT()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_DRAWITEM()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEHWHEEL()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_SETCURSOR()

	ON_CBN_SELCHANGE(eIDZoom, OnSelchangeZoom)
	ON_CBN_EDITCHANGE(eIDZoom, OnEditchangeZoom)
	ON_COMMAND(eIDZoomUp, OnZoomUp)
	ON_COMMAND(eIDZoomDown, OnZoomDown)
	ON_COMMAND(eIDZoomReset, OnZoomReset)
	ON_COMMAND(eIDPatternMatching, OnPatternMatching)
	ON_COMMAND(eIDSettings, OnSettings)
	//ON_CBN_SELCHANGE(eIDScaleDownMethod, OnSelchangeScaleDownMethod)
	//ON_CBN_SELCHANGE(eIDScaleUpMethod, OnSelchangeScaleUpMethod)
	ON_COMMAND(eIDSave, OnSave)

	ON_MESSAGE(WM_UPDATE_DISPLAY_IMAGE, OnUpdateDisplayImage)
END_MESSAGE_MAP()

// CMatView message handlers

//BOOL CMatView::OnInitDialog() {
//	CWnd::OnInitDialog();
//	return TRUE;
//}
//
static const int s_nSpace = 2;
int CMatView::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	//CHECK_MOCHA_LICENSE();
	if (!mocha_features::IsFeatureEnabled(mocha_features::KF_XMATH_UTIL)) return -1;

	lpCreateStruct->style |= WS_TABSTOP;

	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	__super::GetClientRect(m_rectTool);
	m_rectTool.bottom = m_rectTool.top + 24;

	CRect rect(m_rectTool);

	// Zoom Up/Down
	const int nSizeButton = 24;
	auto CreateButton = [&](CMFCButton& btn, int idc, int idb, LPCTSTR pszFace, CRect& rect, int nSpace) {
		rect.left = rect.right;
		rect.right = rect.left + nSizeButton;
		btn.Create(pszFace, WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON|BS_CENTER, rect, this, idc);
		btn.m_nFlatStyle = CMFCButton::BUTTONSTYLE_NOBORDERS;
		btn.SetImage(idb);
	};

	// "Zoom" Label
	rect.left += 4;
	rect.right = rect.left + 60;
	//m_staticZoom.Create(_T("Zoom : "), WS_CHILD|SS_CENTERIMAGE|SS_LEFT, rect, this, eIDZoomLable);

	// Zoom ComboBox
	//rect.left = rect.right + 0;
	rect.right = rect.left + 60;
	CRect rectCombo(rect);
	rectCombo.bottom += 300;
	m_comboZoom.Create(WS_CHILD|WS_VISIBLE|WS_TABSTOP|CBS_DROPDOWN, rectCombo, this, eIDZoom);
	m_comboZoom.AddString(_T("Fit"));
	m_comboZoom.AddString(_T("0.125"));
	m_comboZoom.AddString(_T("0.25"));
	m_comboZoom.AddString(_T("0.5"));
	m_comboZoom.AddString(_T("0.75"));
	m_comboZoom.AddString(_T("1.0"));
	m_comboZoom.AddString(_T("1.25"));
	m_comboZoom.AddString(_T("1.50"));
	m_comboZoom.AddString(_T("2.0"));
	m_comboZoom.AddString(_T("2.5"));
	m_comboZoom.AddString(_T("3"));
	m_comboZoom.AddString(_T("4"));
	m_comboZoom.AddString(_T("5.0"));
	m_comboZoom.AddString(_T("10.0"));
	m_comboZoom.SetCurSel(0);

	CreateButton(m_btnZoom[0], eIDZoomUp, IDB_ZOOM_IN, _T("+"), rect, 0);
	m_btnZoom[0].SetAutorepeatMode(300);
	CreateButton(m_btnZoom[1], eIDZoomDown, IDB_ZOOM_OUT, _T("-"), rect, 0);
	m_btnZoom[1].SetAutorepeatMode(300);
	CreateButton(m_btnZoom[2], eIDZoomReset, IDB_ZOOM_RESET, _T("/"), rect, 0);

	// Save
	CreateButton(m_btnSave, eIDSave, IDB_SAVE, _T("S"), rect, s_nSpace);

	// Pattern Matching
	CreateButton(m_btnTool, eIDPatternMatching, IDB_SEARCH, _T("TM"), rect, s_nSpace);

	// Setting
	CreateButton(m_btnSetting, eIDSettings, IDB_SETTINGS, _T("Settings"), rect, s_nSpace);

	// Info
	//rect.left = rect.right + s_nSpace;
	//rect.right = m_rectTool.right;
	m_staticInfo.Create(WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_BORDER|ES_AUTOHSCROLL|ES_NOHIDESEL|ES_READONLY, CRect(), this, eIDInfo);	// Rect 는 ShowTool(), OnSize() 함수에서 설정
	//m_ttInfo.Create(this, WS_VISIBLE|TTS_ALWAYSTIP|TTS_NOPREFIX/*|TTS_NOANIMATE*/|TTS_BALLOON);
	//m_ttInfo.AddTool(this, _T(""), rect, eIDInfo);
	//EnableToolTips(TRUE);

	if (AfxGetApp() && !m_strRegistrySection.IsEmpty())
		m_setting.SyncRegistry(false, m_strRegistrySection, *AfxGetApp());

	m_imgOrg.SetResizingMethod(m_setting.eScaleDownMethod, m_setting.eScaleUpMethod, false);
	UpdateTool();
	ShowTool(m_setting.bShowTool);

	SetTimer(T_CHECK_FOCUS, 100, nullptr);

	return 0;
}

void CMatView::OnDestroy() {
	m_imgOrg.ResetNotifier();;

	CWnd::OnDestroy();

}

void CMatView::OnTimer(UINT_PTR nIDEvent) {
	switch (nIDEvent) {
	case T_UPDATE_DISPLAY_IMAGE :
		if (IsWindowVisible()) {
			KillTimer(nIDEvent);
			UpdateDisplayImage();
			//CRect rectClient;
			//GetClientRect(rectClient);
			//if (!rectClient.IsRectEmpty())
			//	InvalidateRect(rectClient, false);
		}
		return;
		break;

	case T_CHECK_FOCUS :
		{
			CWnd* pWnd = GetFocus();
			if (pWnd) {
				bool bFocused = (pWnd == this) || IsChild(pWnd);
				if ( (bFocused && !m_bFocusedBefore) || (!bFocused && m_bFocusedBefore) ) {
					m_bFocusedBefore = bFocused;
					Invalidate(false);
				}
			}
		}
		return;
		break;

	case T_SMOOTH_SCROLL :
		{
			auto t = std::chrono::steady_clock::now();
			auto& ss = m_smooth_scroll;
			CPoint2d ptNew;
			if (t < ss.t1) {
				double len = (ss.t1-ss.t0).count();
				double pos = (t-ss.t0).count();
				//double t = (-cos(pos / len * std::numbers::pi) + 1)/2.;
				double t = sin(pos / len * std::numbers::pi/2.);
				ptNew = ss.pt0 + ss.delta * t;
			} else {
				KillTimer(nIDEvent);
				ptNew = ss.pt0 + ss.delta;
				ss.delta = CPoint2d{};
			}
			if (m_ctI2S.m_ptShift == ptNew)
				return;
			m_ctI2S.m_ptShift = ptNew;
			UpdateScrollBars();
			UpdateDisplayImage();
		}
		return;
	}

	CWnd::OnTimer(nIDEvent);
}

BOOL CMatView::PreTranslateMessage(MSG* pMsg) {
	if (!pMsg)
		return CWnd::PreTranslateMessage(pMsg);

	auto Scroll = [&](CPoint2d deltaImage) {
		if (deltaImage == CPoint2d{})
			return;
		if (m_setting.tSmoothScroll > 0) {
			auto& ss = m_smooth_scroll;
			if (ss.delta != CPoint2d{})
				return;
			auto interval = std::chrono::milliseconds(15);
			ss.t0 = std::chrono::steady_clock::now() - interval;

			// 시간
			double len = m_ctI2S.TransLength(deltaImage.GetLength());
			CRect rect;
			GetClientRect(rect);
			double w = std::max(rect.Width(), rect.Height());
			std::chrono::duration t = std::chrono::milliseconds((int64_t)(m_setting.tSmoothScroll * std::clamp(len / w, 0.2, 1.1)));
			if (t <= std::chrono::milliseconds(100)) {
				m_ctI2S.m_ptShift += deltaImage;
				UpdateScrollBars();
				UpdateDisplayImage();
				return;
			}
			ss.t1 = ss.t0 + t;
			ss.pt0 = m_ctI2S.m_ptShift;
			ss.delta = deltaImage;
			SetTimer(T_SMOOTH_SCROLL, interval.count(), nullptr);
		} else {
			m_ctI2S.m_ptShift += deltaImage;
			UpdateScrollBars();
			UpdateDisplayImage();
		}
	};

	if (pMsg->message == WM_KEYDOWN) {
		auto key = pMsg->wParam;
		switch (key) {
		case VK_ESCAPE :
			{
				if (GetCapture() == this)
					ReleaseCapture();

				bool bReturn = false;
				if (m_display.rectCurrent.bShow) {
					SetSelectedImageRect(CRect2d(), false);
					bReturn = true;
				}
				if (m_mouse.bSelectionMode) {
					m_mouse.bSelectionMode = false;
					bReturn = true;
				}
				Invalidate(true);

				if (bReturn)
					return TRUE;
			}
			break;

		case 'S' :
			{
				if (GetCapture() == this)
					ReleaseCapture();

				if ( (GetKeyState(VK_MENU) < 0) && (GetKeyState(VK_CONTROL) < 0) ) {
					OnSettings();
					return TRUE;
				}
			}
			break;

		case 'M' :
			{
				if (GetCapture() == this)
					ReleaseCapture();

				if ( (GetKeyState(VK_MENU) < 0) && (GetKeyState(VK_CONTROL) < 0) ) {
					ShowTool(!m_setting.bShowTool);
					return TRUE;
				}
			}
			break;

		case VK_LEFT :
		case VK_RIGHT :
		case VK_UP :
		case VK_DOWN :
		//case VK_HOME :
		//case VK_END :
		case VK_PRIOR :
		case VK_NEXT :
			if (m_setting.bKeyboardNavigation) {
				CRect rectClient;
				GetClientRect(rectClient);

				CPoint2d ptShift = m_ctI2S.TransI<CPoint2d>(100., 100.) - m_ctI2S.TransI<CPoint2d>(0., 0.);
				CPoint2d ptShiftPage = m_ctI2S.TransI<CPoint2d>(rectClient.BottomRight()) - m_ctI2S.TransI<CPoint2d>(rectClient.TopLeft());
				ptShiftPage *= 0.9;

				CPoint2d delta;
				if (key == VK_LEFT)			delta.x -= ptShift.x;
				else if (key == VK_RIGHT)	delta.x += ptShift.x;
				else if (key == VK_UP)		delta.y -= ptShift.y;
				else if (key == VK_DOWN)	delta.y += ptShift.y;
				else if (key == VK_PRIOR)	delta.y -= ptShiftPage.y;
				else if (key == VK_NEXT)	delta.y += ptShiftPage.y;

				Scroll(delta);

				return true;
			}
			break;

		case VK_SPACE :
			if (m_setting.bKeyboardNavigation) {
				CRect rectClient;
				GetClientRect(rectClient);

				CPoint2i ptCenter{ rectClient.CenterPoint() };
				//CPoint2i pt = ptCenter;

				int heightMin { (int) (m_ctI2S.TransI((double)rectClient.Height()) * 5.0 / 10) };
				int deltaDefaultScreen{ rectClient.Height() * 95 / 100 };
				double deltaImage { m_ctI2S.TransI((double)deltaDefaultScreen) };
				int heightMax { (int) (m_ctI2S.TransI((double)rectClient.Height()) * 9.5 / 10) };
				auto ptImgBottom = m_ctI2S.TransI<CPoint2i>(rectClient.left, rectClient.bottom);
				auto ptImgTop = m_ctI2S.TransI<CPoint2i>(rectClient.left, rectClient.top);
				auto y0 = std::min(ptImgBottom.y + heightMax, m_imgOrg.GetHeight() - 1);

				auto CheckIfBlank = [&](int y) -> bool {
					cv::Rect rc(0, y, m_imgOrg.GetWidth(), 1);
					auto img = m_imgOrg.GetROI(rc);
					if (img.empty())
						return true;
					//auto const vMax = 250;
					auto const vMin = 5;
					if (img.channels() == 3) {
						auto* ptr = img.ptr<cv::Vec3b>(0);
						auto min = ptr[0], max = ptr[0];
						int nDiff{};
						for (int x = 1; x < img.cols; x++) {
							auto const& v = ptr[x];
							for (int i = 0; i < 3; i++) {
								min[i] = std::min(v[i], min[i]);
								max[i] = std::max(v[i], max[i]);
								if (max[i] - min[i] > vMin)
									if (nDiff++ > 2)
										return false;
							}
						}
					}
					else if (img.channels() == 1) {
						auto* ptr = img.ptr<uint8_t>(0);
						auto min = ptr[0], max = ptr[0];
						int nDiff{};
						for (int x = 1; x < img.cols; x++) {
							auto const& v = ptr[x];
							min = std::min(v, min);
							max = std::max(v, max);
							if (max - min > vMin)
								if (nDiff++ > 2)
									return false;
						}
					}
					return true;
				};

				if (m_setting.eAutoScrollMethod == 0) {
				} else if (m_setting.eAutoScrollMethod == 1) {
					if (CheckIfBlank(ptImgBottom.y)) {
						auto yE = m_imgOrg.GetHeight();
						for (auto y = ptImgBottom.y; y < yE; y++) {
							if (!CheckIfBlank(y)) {
								deltaImage = y-ptImgTop.y;
								break;
							}
						}
					}
					else {
						deltaImage = heightMax;
						auto y2 = ptImgTop.y + heightMin*1/2;
						auto y = ptImgBottom.y;
						for (; y >= y2; y--) {
							if (CheckIfBlank(y)) {
								deltaImage = y - ptImgTop.y;
								break;
							}
						}
						if (y < y2) {	// not found
							auto yE = ptImgBottom.y + heightMax;
							for (; y < yE; y++) {
								if (CheckIfBlank(y)) {
									deltaImage = y - ptImgBottom.y;
									break;
								}
							}
						}
					}
				} else if (m_setting.eAutoScrollMethod == 2) {
					//if (true or CheckIfBlank(ptImgBottom.y)) {
						for (auto y = y0; y >= ptImgBottom.y + heightMin; y--) {
							if (CheckIfBlank(y)) {
								deltaImage = y - ptImgBottom.y;
								break;
							}
						}
					//}
					//else {
					//	for (auto y = ptImgBottom.y + heightMin; y < y0; y++) {
					//		if (CheckIfBlank(y)) {
					//			deltaImage = y - ptImgBottom.y + 1;
					//			break;
					//		}
					//	}
					//}
				} else if (m_setting.eAutoScrollMethod == 3) {
					auto y = ptImgTop.y;
					auto y1 = ptImgTop.y + heightMax;
					if (CheckIfBlank(y)) {
						while (y < y1 and CheckIfBlank(y++))
							;
					}
					auto yUp = ptImgTop.y + heightMin*3/5;
					y = std::max(y, yUp);

					while (y < y1 and !CheckIfBlank(y++))
						;

					while (y < y1 and CheckIfBlank(y++))
						;

					deltaImage = y - ptImgTop.y;
					//for (y++; y < y1; y++) {
					//	if (!CheckIfBlank(y)) {
					//		delta = m_ctI2S.Trans((double)(y - ptImgTop.y));
					//		break;
					//	}
					//}
					//delta--;
				}

				Scroll(CPoint2d(0., deltaImage));

				return true;
			}
			break;

		case VK_HOME :
		case VK_END :
			if (m_setting.bKeyboardNavigation) {
				//CRect rectClient;
				//GetClientRect(rectClient);

				//if (key == VK_HOME) {
				//	Scroll(CPoint2d(0, -m_ctI2S.m_ptShift.y));
				//	m_ctI2S.m_ptShift.y = 0;
				//}
				//else {
				//	m_ctI2S.m_ptShift.y = m_imgOrg.GetHeight();
				//}

				//UpdateScrollBars();
				//UpdateDisplayImage();

				Scroll(CPoint2d(0, (key == VK_HOME) ? -m_ctI2S.m_ptShift.y : m_imgOrg.GetHeight()-m_ctI2S.m_ptShift.y));
				return true;
			}
			break;

		}
	}
	//if (pMsg->message == WM_KEYUP) {
	//	if ( (GetKeyState(VK_SHIFT) >= 0) /*(pMsg->wParam == VK_SHIFT)*/ && m_mouse.bSelectionMode) {
	//		EndSelectionMode();
	//	}
	//}

	return CWnd::PreTranslateMessage(pMsg);
}

void TextOutOL(CDC& dc, int x, int y, LPCTSTR pszText, COLORREF crText, COLORREF crOutline) {
	dc.SetTextColor(crOutline);
	dc.TextOut(x-1, y, pszText);
	dc.TextOut(x+1, y, pszText);
	dc.TextOut(x, y-1, pszText);
	dc.TextOut(x, y+1, pszText);
	dc.SetTextColor(crText);
	dc.TextOut(x, y, pszText);
}

//void CMatView::OnNcPaint() {
//	__super::OnNcPaint();
//	TRACE("here\n");
//}
//
void CMatView::OnPaint() {
	m_bDraggingProcessed = true;

	CPaintDC dcMain(this);

	CRect rectClient0;
	GetClientRect(rectClient0);
	if (rectClient0.IsRectEmpty())
		return;

	CRect rectClient(rectClient0);
	if (!g_bWindows7OrGreater) {
		__super::GetClientRect(rectClient);
	}

	CMemDC memDC(dcMain, rectClient);

	CDC& dc = memDC.GetDC();

	CPen pen(PS_SOLID, 1, RGB(0, 0, 0));
	CPen* pOldPen = dc.SelectObject(&pen);
	CBrush brush(RGB(255, 255, 255));
	CBrush* pOldBrush = dc.SelectObject(&brush);

	dc.SelectStockObject(WHITE_BRUSH);
	dc.SelectStockObject(NULL_PEN);
	dc.Rectangle(rectClient);

	if (!m_imgView.empty()) {
		MatToDC(m_imgView, m_imgView.size(), dc, rectClient);
	}
	for (int i = 0; i < m_imgsAttribute.N(); i++) {
		const T_ATTRIBUTE& attr = m_imgsAttribute[i];
		if (attr.imgView.empty())
			continue;
		CRect rectTarget(rectClient);
		rectTarget.left		= attr.ptTargetOffset.x;
		rectTarget.top		= attr.ptTargetOffset.y;
		rectTarget.right	= rectTarget.left + attr.imgView.cols;
		rectTarget.bottom	= rectTarget.top + attr.imgView.rows;

		switch (attr.bm) {
		case BM_TRANSPARENT :
			MatToDCTransparent(attr.imgView, attr.imgView.size(), dc, rectTarget, attr.crTransparent);
			break;
		case BM_ALPHABLEND :
			MatToDCAlphaBlend(attr.imgView, attr.imgView.size(), dc, rectTarget, attr.blend);
			break;
		}
	}

	// cross mark
	for (int i = 0; i < m_display.crosses.N(); i++) {
		const T_CROSS_MARK& cross = m_display.crosses[i];
		if (!cross.bShow)
			continue;
		CPoint pt;
		if (!m_imgOrg.empty()) {
			pt = m_ctI2S(cross.pt);
		}
		if (rectClient.PtInRect(pt)) {
			CPen pen(cross.nPenStyle, cross.iThick, cross.cr);
			dc.SelectObject(&pen);
			dc.MoveTo(pt.x - cross.size.cx, pt.y);
			dc.LineTo(pt.x + cross.size.cx, pt.y);
			dc.MoveTo(pt.x, pt.y - cross.size.cy);
			dc.LineTo(pt.x, pt.y + cross.size.cy);
			if (!cross.strLabel.IsEmpty()) {
				dc.SetBkMode(TRANSPARENT);
				TextOutOL(dc, pt.x, pt.y, cross.strLabel, cross.cr & RGB(0xff, 0xff, 0xff), RGB(0, 0, 0));
			}
		}
	}

	// rect region
	for (int i = 0; i < m_display.rects.N(); i++) {
		const T_RECT_REGION& rect = m_display.rects[i];
		if (!rect.bShow)
			continue;
		CRect2d rc;
		if (m_imgOrg.empty()) {
			rc = rect.rect;
		} else {
			rc.pt0 = m_ctI2S(rect.rect.pt0);
			rc.pt1 = m_ctI2S(rect.rect.pt1);
		}
		if ( ! (CRect2d(rectClient) & rc).IsRectEmpty() ) {
			CPen pen(rect.nPenStyle, rect.iThick, rect.cr & RGB(0xff, 0xff, 0xff));
			dc.SelectObject(&pen);
			dc.SelectStockObject(NULL_BRUSH);
			//dc.Rectangle(CRect(rc));
			dc.MoveTo(_round(rc.pt0.x), _round(rc.pt0.y));
			dc.LineTo(_round(rc.pt1.x), _round(rc.pt0.y));
			dc.LineTo(_round(rc.pt1.x), _round(rc.pt1.y));
			dc.LineTo(_round(rc.pt0.x), _round(rc.pt1.y));
			dc.LineTo(_round(rc.pt0.x), _round(rc.pt0.y));
			if (!rect.strLabel.IsEmpty()) {
				dc.SetBkMode(TRANSPARENT);
				dc.SetTextColor(rect.cr & RGB(0xff, 0xff, 0xff));
				dc.DrawText(rect.strLabel, (CRect)rc, DT_CENTER|DT_VCENTER);
			}
		}
	}

	// Selected Rect
	const auto& r = m_display.rectCurrent;
	if (!m_imgOrg.empty() && r.bShow) {
		CRect2d rc;

		rc.pt0 = m_ctI2S(r.rect.pt0);
		rc.pt1 = m_ctI2S(r.rect.pt1);
		if ( ! (CRect2d(rectClient) & rc).IsRectEmpty() ) {
			CPen pen(r.nPenStyle, r.iThick, r.cr);
			dc.SelectObject(&pen);
			dc.SelectStockObject(NULL_BRUSH);
			//dc.Rectangle(CRect(rc));
			dc.MoveTo(_round(rc.pt0.x), _round(rc.pt0.y));
			dc.LineTo(_round(rc.pt1.x), _round(rc.pt0.y));
			dc.LineTo(_round(rc.pt1.x), _round(rc.pt1.y));
			dc.LineTo(_round(rc.pt0.x), _round(rc.pt1.y));
			dc.LineTo(_round(rc.pt0.x), _round(rc.pt0.y));

			// Cross
			int rop = dc.SetROP2(R2_XORPEN);

			dc.SelectStockObject(WHITE_PEN);
			const int nSize = 30;
			CPoint2d pt = rc.CenterPoint();
			dc.MoveTo(_round(pt.x - nSize/2), _round(pt.y));
			dc.LineTo(_round(pt.x + nSize/2), _round(pt.y));
			dc.MoveTo(_round(pt.x), _round(pt.y - nSize/2));
			dc.LineTo(_round(pt.x), _round(pt.y + nSize/2));

			dc.SetROP2(rop);
		}

	}

	if (!m_imgOrg.empty() && m_mouse.bSelectionMode) {
		CRect2d rc;
		rc.pt0 = m_ctI2S(m_mouse.ptSelect0);
		rc.pt1 = m_ctI2S(m_mouse.ptSelect1);
		if ( ! (CRect2d(rectClient) & rc).IsRectEmpty() ) {
			CPen pen(PS_DOT, 1, RGB(255, 255, 0));
			dc.SelectObject(&pen);
			dc.SelectStockObject(NULL_BRUSH);
			//dc.Rectangle(CRect(rc));
			dc.MoveTo(_round(rc.pt0.x), _round(rc.pt0.y));
			dc.LineTo(_round(rc.pt1.x), _round(rc.pt0.y));
			dc.LineTo(_round(rc.pt1.x), _round(rc.pt1.y));
			dc.LineTo(_round(rc.pt0.x), _round(rc.pt1.y));
			dc.LineTo(_round(rc.pt0.x), _round(rc.pt0.y));

			// Cross
			int rop = dc.SetROP2(R2_XORPEN);

			dc.SelectStockObject(WHITE_PEN);
			const int nSize = 30;
			CPoint2d pt = rc.CenterPoint();
			dc.MoveTo(_round(pt.x - nSize/2), _round(pt.y));
			dc.LineTo(_round(pt.x + nSize/2), _round(pt.y));
			dc.MoveTo(_round(pt.x), _round(pt.y - nSize/2));
			dc.LineTo(_round(pt.x), _round(pt.y + nSize/2));

			dc.SetROP2(rop);
		}
	}

	// Mini Map
	if (!m_imgOrg.empty() && m_setting.bMapWindow) {
		CRect rect;
		CSize2i size(m_imgOrg.size());

		int cx = m_imgOrg.GetWidth();
		int cy = m_imgOrg.GetHeight();

		CCoordTrans ctI(m_ctI2S.GetInverse());
		CPoint ptImgTopLeft = ctI(rectClient.TopLeft());
		ptImgTopLeft.x = std::clamp<int>(ptImgTopLeft.x, 0, cx);
		ptImgTopLeft.y = std::clamp<int>(ptImgTopLeft.y, 0, cy);

		CPoint ptImgBottomRight = ctI(rectClient.BottomRight());
		ptImgBottomRight.x = std::clamp<int>(ptImgBottomRight.x, 0, cx);
		ptImgBottomRight.y = std::clamp<int>(ptImgBottomRight.y, 0, cy);

		// 면적이 1/10 이 되도록....
		double dArea = rectClient.Width() * rectClient.Height() / 100;
		double dRate = sqrt(dArea / (cx * cy));
		double dRateMaxX = rectClient.Width() * 0.80 / cx;
		double dRateMaxY = rectClient.Height() * 0.80 / cy;

		dRate = std::min(dRate, std::min(dRateMaxX, dRateMaxY));

		int rcx = std::max(2, int(dRate * cx));
		int rcy = std::max(2, int(dRate * cy));

		CCoordTrans ct(dRate, {1, 0, 0, 1}, {cx/2., cy/2.}, {rcx/2., rcy/2.} );

		CRect rectFullSize(ct.Trans(CPoint2d(0, 0)), ct.Trans(CPoint2d(cx, cy)));
		CRect rectDisplayed(ct.Trans(ptImgTopLeft), ct.Trans(ptImgBottomRight));
		int nMinLength = 2;
		if (rectDisplayed.Width() < nMinLength)
			rectDisplayed.right = rectDisplayed.left + nMinLength;
		if (rectDisplayed.Height() < nMinLength)
			rectDisplayed.bottom = rectDisplayed.top + nMinLength;

		if ( (rectFullSize & rectDisplayed) != rectFullSize ) {
			CImage img;
			img.Create(rcx, rcy, 24);
			CDC dcBox;
			dcBox.Attach(img.GetDC());
			static CBrush brushFullSize(RGB(255, 255, 255));
			dcBox.SelectStockObject(BLACK_PEN);
			//dcBox.SelectStockObject(NULL_BRUSH);
			dcBox.SelectObject(&brushFullSize);
			//dcBox.Rectangle(rectFullSize);
			Mat imgMap;
			resize(m_imgOrg.GetMergedImage(), imgMap, Size(rcx, rcy), 0, 0, cv::INTER_NEAREST);
			MatToDC(imgMap, Size(rcx,rcy), dcBox, CRect(0, 0, rcx, rcy));
			//static CBrush brushDisplayed(RGB(255, 128, 255));
			static CPen penBox(PS_SOLID, 5, RGB(128, 0, 128));
			dcBox.SelectObject(&penBox);
			dcBox.SelectStockObject(NULL_BRUSH);
			dcBox.Rectangle(rectDisplayed);
			//dcBox.SelectObject(&brushDisplayed);
			//dcBox.FillRect(rectDisplayed, &brushDisplayed);
			dcBox.Detach();
			img.ReleaseDC();
			img.AlphaBlend(dc, rectClient.right-rcx-5, rectClient.bottom-rcy-5, rcx, rcy, 0, 0, rcx, rcy, 100);
		}
	}

	// Focus 표시
	CWnd* pWndFocus = GetFocus();
	bool bFocused = pWndFocus && ( (pWndFocus == this) || IsChild(pWndFocus) );
	if (bFocused) {
		static CPen penFocused(PS_DOT, 3, RGB(128, 0, 128));
		dc.SelectObject(&penFocused);
	} else {
		dc.SelectStockObject(BLACK_PEN);
	}
	dc.SelectStockObject(NULL_BRUSH);
	dc.Rectangle(rectClient);

	dc.SelectObject(pOldPen);
	dc.SelectObject(pOldBrush);

}

void CMatView::OnSize(UINT nType, int cx, int cy) {
	CWnd::OnSize(nType, cx, cy);

	if (nType == SIZE_MINIMIZED) // || (cx < 100) || (cy < 100) )
		return;

	//if (m_rcUpdateScroll.IsUnique()) {
		UpdateScrollBars();
		UpdateDisplayImage();
	//}

	CRect rectClient;
	GetClientRect(rectClient);
	if (m_staticInfo) {
		CRect rect;
		m_staticInfo.GetWindowRect(rect);
		ScreenToClient(rect);
		if (m_setting.bStatusOnBottom) {
			CRect rectC;
			__super::GetClientRect(rectC);
			rect.top = rectClient.bottom;
			rect.bottom = rectC.bottom;
			rect.left = rectC.left;
			rect.right = rectClient.right;
		} else {
			rect.right = rectClient.right;
		}
		m_staticInfo.MoveWindow(rect);
		m_rectTool.right = rectClient.right;
	}
}

void CMatView::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CWnd::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CMatView::OnMouseMove(UINT nFlags, CPoint point) {
	if (m_mouse.bDragMode && !m_imgOrg.empty() && (GetCapture() == this)) {
		CPoint2d pt0(m_mouse.ptLast);
		CPoint2d pt1(point);
		CPoint2d ptShift((pt0-pt1)/m_ctI2S.GetScale()*2 + m_mouse.ptImgLast);
		SetImageCenter(ptShift, -1, !m_bDraggingProcessed);
	}

	UpdateMousePosition(point);

	if (m_bNotifyAllMouseEvent) {
		NotifyMouseEvent(NC_MOUSE_MOVE, point);
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CMatView::OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt) {
	CWnd::OnMouseHWheel(nFlags, zDelta, pt);
}

BOOL CMatView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) {
	if (m_imgOrg.empty())
		return TRUE;

	if (GetKeyState(VK_CONTROL) < 0) {
		DoMouseFunction((zDelta >= 0) ? m_eMFWheelUpCtrl : m_eMFWheelDownCtrl, TRUE, nFlags, pt, (short)abs(zDelta));
	}
	else {
		DoMouseFunction((zDelta >= 0) ? m_eMFWheelUp : m_eMFWheelDown, TRUE, nFlags, pt, (short)abs(zDelta));
	}

	//return CWnd::OnMouseWheel(nFlags, zDelta, pt);
	return TRUE;
}

BOOL CMatView::DoMouseFunction(eMOUSE_FUNCTION eMF, BOOL bDown, UINT /*nFlags*/, CPoint point, short zDelta) {
	//ScreenToClient(&point);

	if (bDown) {
		if (m_setting.bClearSelectionOnMouseButton && m_display.rectCurrent.bShow) {
			SetSelectedImageRect(CRect2d(), false);
			Invalidate(false);
		}
	} else {
		if (m_mouse.bDragMode) {
			m_mouse.bDragMode = FALSE;
			if (GetCapture() == this)
				ReleaseCapture();
			return TRUE;
		}
	}

	if (m_imgOrg.empty())
		return true;

	switch (eMF) {
	case MF_NONE :
		break;

	case MF_DRAG_IMAGE :
		if (bDown) {
			if (!GetCapture())
				SetCapture();

			m_mouse.ptScrollLast.x = GetScrollPos(SB_HORZ);
			m_mouse.ptScrollLast.y = GetScrollPos(SB_VERT);
			m_mouse.ptLast = point;
			m_ctI2S.GetShift(m_mouse.ptImgLast);
			m_mouse.bDragMode = TRUE;

		} else {
			m_mouse.bDragMode = FALSE;
			if ( (GetCapture() == this) && !m_mouse.bSelectionMode )
				ReleaseCapture();
			return TRUE;
		}
		break;

	case MF_SELECT_REGION :
		if (bDown) {
			if (GetCapture() || m_mouse.bSelectionMode) {
				EndSelectionMode();
			} else {
				bool b2ClickSelection = m_setting.b2ClickSelection;
				bool bShift = (GetKeyState(VK_CONTROL) < 0);
				if (b2ClickSelection ^ bShift) {
					// 2-Click-Selection
					StartSelectionMode(m_ctI2S.TransI<CPoint2d>(point));
				} else {
					// 1-Click-Selection
					CRect rectClient;
					GetClientRect(rectClient);
					if (rectClient.PtInRect(point)) {
						CRect2d rect;
						rect.pt0 = point;
						if (SelectRegion(rect))
							NotifyEvent(NC_SEL_REGION, rect);
					}
				}
			}
		} else {
		}
		break;

	case MF_ZOOM_IN :
	case MF_ZOOM_OUT :
		{
			ScreenToClient(&point);

			if (eMF == MF_ZOOM_OUT)
				zDelta = -zDelta;

			double dZoom = GetZoom();
			//double dRate = 1.0 + (zDelta > 0 ? m_setting.dZoomRate-1 : 1-m_setting.dZoomRate)*2;
			dZoom *= (zDelta > 0 ? m_setting.dZoomRate : 1/m_setting.dZoomRate);
			CheckAndGetZoom(dZoom);

			CRect rectClient;
			GetClientRect(rectClient);
			CPoint2d ptShift;
			ptShift = m_ctI2S.TransI<CPoint2d>(point.x, point.y);
			CCoordTrans ct(m_ctI2S);
			ct.SetShift(ptShift);
			ct.SetOffset(point);
			ct.SetScale(dZoom);
			CPoint2d ptCenter(rectClient.CenterPoint());
			m_ctI2S.SetShift(ct.TransI(ptCenter));

			SetDlgItemDouble(this, eIDZoom, dZoom, _T("%g"));

			UpdateScrollBars();
			UpdateDisplayImage();
		}
		break;

	case MF_SCROLL_UP :
	case MF_SCROLL_DOWN :
		{
			ScreenToClient(&point);

			if (eMF == MF_SCROLL_DOWN)
				zDelta = -zDelta;

			//double dZoom = GetZoom();
			////double dRate = 1.0 + (zDelta > 0 ? m_setting.dZoomRate-1 : 1-m_setting.dZoomRate)*2;
			//dZoom *= (zDelta > 0 ? m_setting.dZoomRate : 1/m_setting.dZoomRate);
			//CheckAndGetZoom(dZoom);

			CRect rectClient;
			GetClientRect(rectClient);
			CPoint2d ptShift;
			ptShift = m_ctI2S.TransI<CPoint2d>(point.x, point.y-zDelta);
			CCoordTrans ct(m_ctI2S);
			ct.SetShift(ptShift);
			ct.SetOffset(point);
			CPoint2d ptCenter(rectClient.CenterPoint());
			m_ctI2S.SetShift(ct.TransI(ptCenter));

			//SetDlgItemDouble(this, eIDZoom, dZoom, _T("%g"));

			UpdateScrollBars();
			UpdateDisplayImage();
		}
		break;
	}

	return TRUE;
}

// Rect Tracker with Center Cross Mark
class CRectTrackerCenterMarkCalback : public CRectTrackerCenterMark {
public:
	CRect m_rectExclude;
	COLORREF m_crCrossMark;
	CMatView* m_pView = nullptr;
protected:
	CPoint m_ptCenterLast;
public:
	CRectTrackerCenterMarkCalback(CMatView* pView, COLORREF crCrossMark = RGB(255, 255, 255)) : CRectTrackerCenterMark(crCrossMark), m_pView(pView) {  }
	CRectTrackerCenterMarkCalback(CMatView* pView, LPCRECT lpSrcRect, UINT nStyle, COLORREF crCrossMark = RGB(255, 255, 255)) : CRectTrackerCenterMark(lpSrcRect, nStyle), m_crCrossMark(crCrossMark), m_pView(pView) {}

	virtual void DrawTrackerRect(LPCRECT lpRect, CWnd* pWndClipTo, CDC* pDC, CWnd* pWnd) {
		__super::DrawTrackerRect(lpRect, pWndClipTo, pDC, pWnd);
		if (m_pView && m_pView->m_hWnd)
			m_pView->UpdateTrackPosition(lpRect);
	}
	BOOL Track(CWnd* pWnd, CPoint point, BOOL bAllowInvert = FALSE, CWnd* pWndClipTo = NULL)	{ m_ptCenterLast.x = -1000; return __super::Track(pWnd, point, bAllowInvert, pWndClipTo); }
	BOOL TrackRubberBand(CWnd* pWnd, CPoint point, BOOL bAllowInvert = TRUE)					{ m_ptCenterLast.x = -1000; return __super::TrackRubberBand(pWnd, point, bAllowInvert); }
};

BOOL CMatView::UpdateMousePosition(const CPoint& point) {
	CPoint2d ptImage;
	ptImage = m_ctI2S.TransI(CPoint2d(point));

	auto PrintSelection = [&] (CString& str, const CPoint2d& ptStart, const CPoint2d& ptEnd) {
		CRect2d rc;
		rc.pt0 = m_ctI2S(ptStart);
		rc.pt1 = m_ctI2S(ptEnd);
		CPoint2d ptI0 = ptStart;
		CPoint2d ptI1 = ptEnd;
		CPoint2d ptIC = (ptI0+ptI1)/2;
		str += Format(_T("IC(%.1f, %.1f) IS<%.1f, %.1f>"), ptIC.x, ptIC.y,ptI1.x-ptI0.x, ptI1.y-ptI0.y);
		if (m_rCTI2M) {
			CPoint2d pt0 = m_rCTI2M->Trans(ptI0);
			CPoint2d pt1 = m_rCTI2M->Trans(ptI1);
			CPoint2d ptC = (pt0+pt1)/2;
			str += Format(_T(", MC(%.3f, %.3f) MS<%.3f, %.3f>"), ptC.x, ptC.y, pt1.x-pt0.x, pt1.y-pt0.y);
		}
	};

	CString str;
	if ( (ptImage.x >= 0) && (ptImage.x <= m_imgOrg.GetWidth()) && (ptImage.y >= 0) && (ptImage.y <= m_imgOrg.GetHeight()) && (m_imgOrg.depth() == CV_8U) ) {
		CPoint2i pt(int(ptImage.x), int(ptImage.y));	// drop the floating points
		if (pt.x < 0) pt.x = 0;
		if (pt.y < 0) pt.y = 0;
		if (pt.x >= m_imgOrg.GetWidth())	pt.x = m_imgOrg.GetWidth() -1;
		if (pt.y >= m_imgOrg.GetHeight())	pt.y = m_imgOrg.GetHeight()-1;

		// Color
		if (m_imgOrg.channels() == 1) {
			str += Format(_T("cr[%02x]"), (int)m_imgOrg.at<BYTE>(pt));
		} else if (m_imgOrg.channels() == 3) {
			Vec3b cr = m_imgOrg.at<Vec3b>(pt);
			str += Format(_T("cr[%02x%02x%02x]"), (int)cr[2], (int)cr[1], (int)cr[0]);
		}

		// Mouse Position
		if (!str.IsEmpty())
			str += _T(", ");
		str += Format(_T("I(%.1f, %.1f)"), ptImage.x, ptImage.y);
		if (m_rCTI2M) {
			CPoint2d ptM(m_rCTI2M->Trans(ptImage));
			if (!str.IsEmpty())
				str += _T(", ");
			str += Format(_T("M(%.3f, %.3f)"), ptM.x, ptM.y);
		}

	} else {
		str += _T("cr[], I()");
		if (m_rCTI2M) {
			str += _T("M()");
		}
	}

	if (m_mouse.bSelectionMode) {
		if (!str.IsEmpty())
			str += _T(", ");
		PrintSelection(str, m_mouse.ptSelect0, m_mouse.ptSelect1);
	}

	if (m_display.rectCurrent.bShow /*&& m_display.rectCurrent.rect.PtInRect(ptImage)*/) {
		if (!str.IsEmpty())
			str += _T(", ");
		PrintSelection(str, m_display.rectCurrent.rect.pt0, m_display.rectCurrent.rect.pt1);
	}

	CString strC;
	m_staticInfo.GetWindowText(strC);
	if (str != strC) {
		m_staticInfo.SetWindowText(str);
		UpdateToolTip(str);
	}

	if (m_mouse.bSelectionMode) {
		m_mouse.ptSelect1 = m_ctI2S.TransI<CPoint2d>(point);
		Invalidate(false);
	}

	return 0;
}

BOOL CMatView::UpdateTrackPosition(LPCRECT lpRect) {
	if (!m_staticInfo)
		return FALSE;
	CRect rect(lpRect);
	CString str;
	CPoint2d ptI0 = m_ctI2S.TransI<CPoint2d>(rect.left, rect.top);
	CPoint2d ptI1 = m_ctI2S.TransI<CPoint2d>(rect.right, rect.bottom);
	CPoint2d ptIC = (ptI0+ptI1)/2;
	str.Format(_T("IC(%.1f, %.1f) IS<%.1f, %.1f>"), ptIC.x, ptIC.y,ptI1.x-ptI0.x, ptI1.y-ptI0.y);
	if (m_rCTI2M) {
		CPoint2d pt0 = m_rCTI2M->Trans(ptI0);
		CPoint2d pt1 = m_rCTI2M->Trans(ptI1);
		CPoint2d ptC = (pt0+pt1)/2;
		str += Format(_T(", MC(%.3f, %.3f) MS<%.3f, %.3f>"), ptC.x, ptC.y, pt1.x-pt0.x, pt1.y-pt0.y);
	}
	m_staticInfo.SetWindowText(str);
	return TRUE;
}

BOOL CMatView::UpdateToolTip(LPCTSTR psz) {
	//if (!m_ttInfo)
	//	return false;
	//CToolInfo ti;
	//ZeroVar(ti);
	//if (!m_ttInfo.GetToolInfo(ti, this, eIDInfo))
	//	return false;

	//CRect rect;
	//m_staticInfo.GetWindowRect(rect);
	//ScreenToClient(rect);
	//ti.rect = rect;
	//m_ttInfo.SetToolInfo(&ti);

	//m_ttInfo.UpdateTipText(psz, this, eIDInfo);

	//m_ttInfo.Activate(true);

	return true;
}

void CMatView::SetResizingMethod(int eScaleDownMethod, int eScaleUpMethod, bool bThumbnailInBkgnd) {
	//if (m_comboScaleUpMethod.GetCurSel() != eScaleUpMethod)
	//	m_comboScaleUpMethod.SetCurSel(eScaleUpMethod);
	//if (m_comboScaleDownMethod.GetCurSel() != eScaleDownMethod)
	//	m_comboScaleDownMethod.SetCurSel(eScaleDownMethod);

	if (eScaleDownMethod >= 0)
		m_setting.eScaleDownMethod = eScaleDownMethod;
	if (eScaleUpMethod >= 0)
		m_setting.eScaleUpMethod = eScaleUpMethod;
	m_imgOrg.SetResizingMethod(eScaleDownMethod, eScaleUpMethod, bThumbnailInBkgnd);

	UpdateDisplayImage();
}

BOOL CMatView::SelectRegion(CRect2d& rect) {
	if (m_imgOrg.empty())
		return false;

	CRectTrackerCenterMarkCalback tracker(this, RGB(255, 255, 255));
	tracker.m_sizeMin.SetSize(8, 8);
	tracker.m_rectExclude = m_rectTool;
	if (!tracker.TrackRubberBand(this, rect.TopLeft(), TRUE))
		return false;
	CRect rectSelected;
	tracker.GetTrueRect(rectSelected);
	rect.pt0 = CPoint2d(rectSelected.left, rectSelected.top);
	rect.pt1 = CPoint2d(rectSelected.right, rectSelected.bottom);

	if ( (abs(rectSelected.Width()) <= tracker.m_sizeMin.cx) && (abs(rectSelected.Height()) <= tracker.m_sizeMin.cy) )
		return false;

	CRect2d rectSelectedImage;
	rectSelectedImage.pt0 = m_ctI2S.TransI<CPoint2d>(rectSelected.left, rectSelected.top);
	rectSelectedImage.pt1 = m_ctI2S.TransI<CPoint2d>(rectSelected.right, rectSelected.bottom);

	SetSelectedImageRect(rectSelectedImage, true);
	//auto& r = m_display.rectCurrent;
	//r.bShow = true;
	//r.rect.pt0 = m_ctI2S.TransI<CPoint2d>(rectSelected.left, rectSelected.top);
	//r.rect.pt1 = m_ctI2S.TransI<CPoint2d>(rectSelected.right, rectSelected.bottom);
	//r.cr = RGB(255, 255, 255);
	//r.iThick = 1;
	//r.nPenStyle = PS_DASH;
	//r.strLabel.Empty();
	//r.strName.Empty();
	//Invalidate(FALSE);
	return true;
}

LRESULT CMatView::NotifyEvent(eNOTIFY_CODE evtCode, const CRect2d& rect, bool bImageRect) {
	NOTIFY_DATA nm;
	ZeroVar(nm);
	nm.hdr.code = evtCode;
	nm.hdr.hwndFrom = m_hWnd;
	nm.hdr.idFrom = GetDlgCtrlID();

	CRect2d rectImage;
	if (bImageRect) {
		rectImage = rect;
	} else {
		rectImage.pt0 = m_ctI2S.TransI<CPoint2d>(rect.left,  rect.top);
		rectImage.pt1 = m_ctI2S.TransI<CPoint2d>(rect.right, rect.bottom);
	}
	nm.rect = rectImage;

	if (m_rCTI2M) {
		nm.rectReal.pt0 = m_rCTI2M->Trans(rectImage.pt0);
		nm.rectReal.pt1 = m_rCTI2M->Trans(rectImage.pt1);
	}

	Rect rcImage(rectImage);
	AdjustROI(rcImage, nm.rcImage, m_imgOrg.size());

	if ( (evtCode == NC_SEL_REGION)
		&& m_pDlgPatternMatching && m_pDlgPatternMatching->m_hWnd && m_pDlgPatternMatching->IsWindowVisible()
		&& m_pDlgPatternMatching->IsDlgButtonChecked(IDC_CB_SELECT_IMAGE)
		)
	{
		return m_pDlgPatternMatching->SendMessage(WM_PATTERN_MATCHING_DLG_SEL_MARK, nm.hdr.idFrom, (LPARAM)&nm);
	}

	return GetParent()->SendMessage(WM_NOTIFY, nm.hdr.idFrom, (LPARAM)&nm);
}

LRESULT CMatView::NotifyMouseEvent(eNOTIFY_CODE evtCode, CPoint pt) {
	NOTIFY_DATA nm;
	ZeroVar(nm);
	nm.hdr.code = evtCode;
	nm.hdr.hwndFrom = m_hWnd;
	nm.hdr.idFrom = GetDlgCtrlID();

	nm.ptWindow = pt;
	nm.ptImage = m_ctI2S.TransI<CPoint2d>(pt);
	if (m_rCTI2M) {
		nm.ptReal = m_rCTI2M->Trans(nm.ptImage);
	}

	return GetParent()->SendMessage(WM_NOTIFY, nm.hdr.idFrom, (LPARAM)&nm);
}

void CMatView::OnLButtonDown(UINT nFlags, CPoint point) {
	if (m_bNotifyAllMouseEvent) {
		NotifyMouseEvent(NC_LBUTTON_DOWN, point);
	}

	if (m_setting.bClickToMagnifySelectedRect && m_display.rectCurrent.bShow) {
		auto& r = m_display.rectCurrent;
		CRect2i rect;
		rect.pt0 = m_ctI2S.Trans(r.rect.pt0);
		rect.pt1 = m_ctI2S.Trans(r.rect.pt1);

		if (!rect.IsRectEmpty() && rect.PtInRect(point)) {
			SetCurrentImageRect(r.rect);
			return;
		}

		SetSelectedImageRect(CRect2d(), false);
	}

	if (GetFocus() != this)
		SetFocus();
	DoMouseFunction((GetKeyState(VK_SHIFT) < 0) ? m_eMFRButton : m_eMFLButton, TRUE, nFlags, point);
	CWnd::OnLButtonDown(nFlags, point);

	NotifyEvent((eNOTIFY_CODE)NM_CLICK, CRect2d(point, point));

}

void CMatView::OnLButtonUp(UINT nFlags, CPoint point) {
	if (m_bNotifyAllMouseEvent) {
		NotifyMouseEvent(NC_LBUTTON_UP, point);
	}

	DoMouseFunction((GetKeyState(VK_SHIFT) < 0) ? m_eMFRButton : m_eMFLButton, FALSE, nFlags, point);
	CWnd::OnLButtonUp(nFlags, point);
}

void CMatView::OnRButtonDown(UINT nFlags, CPoint point) {
	if (m_bNotifyAllMouseEvent) {
		NotifyMouseEvent(NC_RBUTTON_DOWN, point);
	}

	if (GetFocus() != this)
		SetFocus();
	DoMouseFunction((GetKeyState(VK_SHIFT) < 0) ? m_eMFLButton : m_eMFRButton, TRUE, nFlags, point);
	CWnd::OnRButtonDown(nFlags, point);

	NotifyEvent((eNOTIFY_CODE)NM_RCLICK, CRect2d(point, point));
}

void CMatView::OnRButtonUp(UINT nFlags, CPoint point) {
	if (m_bNotifyAllMouseEvent) {
		NotifyMouseEvent(NC_RBUTTON_UP, point);
	}

	DoMouseFunction((GetKeyState(VK_SHIFT) < 0) ? m_eMFLButton : m_eMFRButton, FALSE, nFlags, point);
	CWnd::OnRButtonUp(nFlags, point);
}

void CMatView::OnMButtonDown(UINT nFlags, CPoint point) {
	if (m_bNotifyAllMouseEvent) {
		NotifyMouseEvent(NC_MBUTTON_DOWN, point);
	}

	if (GetFocus() != this)
		SetFocus();
	DoMouseFunction(m_eMFMButton, TRUE, nFlags, point);
	CWnd::OnMButtonDown(nFlags, point);
}

void CMatView::OnMButtonUp(UINT nFlags, CPoint point) {
	if (m_bNotifyAllMouseEvent) {
		NotifyMouseEvent(NC_MBUTTON_UP, point);
	}

	DoMouseFunction(m_eMFMButton, FALSE, nFlags, point);
	CWnd::OnMButtonUp(nFlags, point);
}

int CMatView::OnScroll(int eBar, UINT nSBCode, UINT /*nPos*/) {
	if (!m_hWnd)
		return FALSE;

	SCROLLINFO si;
	GetScrollInfo(eBar, &si, SIF_ALL);

	if (si.nMin == si.nMax)
		return -1;

	if (si.nPage == 0)
		return -1;

	int nNewPos = -1;
	switch (nSBCode) {
	case SB_BOTTOM :
		break;
	case SB_ENDSCROLL :
		break;
	case SB_LINEDOWN :
		if ((si.nPos + 1) <= si.nMax)
			nNewPos = si.nPos+1;
		break;
	case SB_LINEUP :
		if ((si.nPos - 1) >= si.nMin)
			nNewPos = si.nPos-1;
		break;
	case SB_PAGEDOWN :
		if ((si.nPos + (int)si.nPage) < si.nMax) 
			nNewPos = si.nPos + si.nPage;
		else
			nNewPos = si.nMax;
		break;
	case SB_PAGEUP :
		if ((si.nPos - (int)si.nPage) >= si.nMin)
			nNewPos = si.nPos - si.nPage;
		else
			nNewPos = si.nMin;
		break;
	case SB_THUMBPOSITION :
	case SB_THUMBTRACK :
		nNewPos = si.nTrackPos;
		break;
	case SB_TOP :
		break;
	}

	if (nNewPos == -1)
		return nNewPos;

	if (nNewPos < si.nMin)
		nNewPos = si.nMin;
	if (nNewPos > si.nMax-(int)si.nPage)
		nNewPos = si.nMax-(int)si.nPage;

	CPoint2d ptShift;
	m_ctI2S.GetShift(ptShift);
	if (eBar == SB_HORZ) {
		ptShift.x = (nNewPos + si.nPage/2) / m_ctI2S.GetScale();
	} else if (eBar == SB_VERT) {
		ptShift.y = (nNewPos + si.nPage/2) / m_ctI2S.GetScale();
	}
	m_ctI2S.SetShift(ptShift);

	SetScrollPos(eBar, nNewPos);

	return nNewPos;
}

void CMatView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
	OnScroll(SB_HORZ, nSBCode, nPos);
	UpdateDisplayImage();
}

void CMatView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
	OnScroll(SB_VERT, nSBCode, nPos);
	UpdateDisplayImage();
}

BOOL CMatView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) {
	if (m_setting.bClickToMagnifySelectedRect && m_display.rectCurrent.bShow && m_hCursorMag) {
		CRect2i rect;
		rect.pt0 = m_ctI2S(m_display.rectCurrent.rect.pt0);
		rect.pt1 = m_ctI2S(m_display.rectCurrent.rect.pt1);
		CPoint pt = GetCurrentMessage()->pt;
		ScreenToClient(&pt);
		if (rect.PtInRect(pt)) {
			SetCursor(m_hCursorMag);
			return TRUE;
		}
	}	
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

BOOL CMatView::CheckAndGetZoom(double& dZoom) {
	if (dZoom < 0)
		dZoom = GetDlgItemDouble(this, eIDZoom);
	BOOL bResult = (dZoom != 0.0);

	if (m_imgOrg.empty()) {
		dZoom = 1.0;
		return FALSE;
	}

	CRect rectClient;
	GetClientRect(rectClient);
	if (rectClient.IsRectEmpty()) {
		dZoom = 1.0;
		return FALSE;
	}

	double dMinZoom = 1;
	const double dFitZoom = _min((double)rectClient.Width()/m_imgOrg.GetWidth(), (double)rectClient.Height()/m_imgOrg.GetHeight());
	double dMaxZoom = 10.0;

	//if (dZoom < dFitZoom)
	//	bResult = FALSE;

	if (dMinZoom > dFitZoom)
		dMinZoom = dFitZoom;

	//if (dMaxZoom < dFitZoom*10)
	//	dMaxZoom = dFitZoom*10;

	if (dZoom == 0.0)
		dZoom = dFitZoom;

	if (dZoom < dMinZoom)
		dZoom = dMinZoom;

	if (dZoom > dMaxZoom)
		dZoom = dMaxZoom;

	//if (!bResult)
	//	m_comboZoom.SelectString(-1, _T("Fit To Image"));

	return bResult;
}

void CMatView::OnSelchangeZoom() {
	CRect2d rect;
	GetCurrentImageRect(rect);

	int iSel = m_comboZoom.GetCurSel();
	if (iSel < 0)
		return;
	{
		//CRefCounter rcUpdateScroll(m_rcUpdateScroll);
		CString str;
		m_comboZoom.GetLBText(iSel, str);
		double dZoom = _ttof(str);
		//Size s = m_imgOrg.size();
		//m_ctI2S.SetShift(s.width/2, s.height/2);
		m_ctI2S.SetShift(rect.CenterPoint());
		UpdateScrollBars(dZoom);
		UpdateDisplayImage(dZoom);

		//SetImageCenter(rect.CenterPoint());
	}
}
void CMatView::OnEditchangeZoom() {
	//Size s = m_imgOrg.size();
	//m_ctI2S.SetShift(s.width/2, s.height/2);

	CRect2d rect;
	GetCurrentImageRect(rect);
	m_ctI2S.SetShift(rect.CenterPoint());

	UpdateScrollBars();
	UpdateDisplayImage();

}
void CMatView::OnZoomUp() {
	double dZoom = GetZoom();
	dZoom *= m_setting.dZoomRate;
	SetZoom(dZoom);
}
void CMatView::OnZoomDown() {
	double dZoom = GetZoom();
	dZoom /= m_setting.dZoomRate;
	SetZoom(dZoom);
}
void CMatView::OnZoomReset() {
	InitView(0);
}

void CMatView::OnSave() {
	if (m_imgOrg.empty())
		return;

	CWaitCursor wc;
	CFileDialog dlg(FALSE, _T(".png"), nullptr, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_NOCHANGEDIR, FILTER_IMAGE_FILES, this);
	wc.Restore();
	if (dlg.DoModal() != IDOK)
		return;

	try {
		Mat img;
		if (m_display.rectCurrent.bShow) {
			cv::Rect rc;
			AdjustROI(m_display.rectCurrent.rect, rc, m_imgOrg.size());
			img = m_imgOrg.GetROI(rc);
		} else {
			img = m_imgOrg.GetMergedImage();
		}
		std::vector<int> params;
		params.push_back(cv::IMWRITE_JPEG_QUALITY);
		params.push_back(m_setting.iJPGQuality);
		imwrite((LPCSTR)CStringA(dlg.GetPathName()), img, params);
	} catch (...) {
		MessageBox(_T("Failed"));
	}
}

void CMatView::OnPatternMatching() {
	if (m_imgOrg.empty())
		return;

	if (!m_pDlgPatternMatching) {
		m_pDlgPatternMatching = new CMatViewPatternMatchingDlg(this);
		m_pDlgPatternMatching->Create(this);
	}

	m_pDlgPatternMatching->ShowWindow(SW_SHOW);
	m_pDlgPatternMatching->SetFocus();
}

void CMatView::OnSettings() {
	CMatViewSettingDlg dlg(this);
	dlg.m_setting = m_setting;
	if (dlg.DoModal() != IDOK)
		return;

	m_setting = dlg.m_setting;
	SetResizingMethod(m_setting.eScaleDownMethod, m_setting.eScaleUpMethod);

	if (AfxGetApp() && !m_strRegistrySection.IsEmpty())
		m_setting.SyncRegistry(true, m_strRegistrySection, *AfxGetApp());

	UpdateTool();
	ShowTool(m_setting.bShowTool);
}

//-----------------------------------------------------------------------------
//

void CMatView::GetClientRect(LPRECT lpRect) const {
	__super::GetClientRect(lpRect);
	if (m_setting.bShowTool) {
		lpRect->top = m_rectTool.bottom;
		if (m_setting.bStatusOnBottom)
			lpRect->bottom -= m_rectTool.Height();
	}
}

//-----------------------------------------------------------------------------
//
BOOL CMatView::PrepareDisplayImage(const cv::Mat& imgOrg, cv::Mat& imgView, const cv::Point& ptPatch, CPoint& ptOffset, const CRect& rectClient, double dZoom) {
	if (!m_hWnd || imgOrg.empty() || rectClient.IsRectEmpty())
		return FALSE;

	//CWaitCursor wc;

	CRect2d rectDispatch;
	rectDispatch.pt0 = m_ctI2S(ptPatch);
	rectDispatch.pt1 = m_ctI2S(CPoint2d(ptPatch)+CSize2d(imgOrg.size()));
	CRect2d rectTarget(rectClient);
	rectTarget &= rectDispatch;
	//rectTarget.NormalizeRect();

	if ( rectTarget.IsRectEmpty() ) {
		imgView.release();
		ptOffset = {0, 0};
		return FALSE;
	}

	// dZoom 값 무시. m_ctI2S 사용하면 되는데....
	dZoom = m_ctI2S.m_dScale;
	if (m_ctI2S.m_dScale == 1.0) {
		imgView = imgOrg;
	} else {
		int eInterpolation = cv::INTER_NEAREST;// dZoom >= 1.0 ? m_imgOrg.GetScaleUpMethod() : m_imgOrg.GetScaleDownMethod();
		ResizeImage(imgOrg, imgView, dZoom, eInterpolation);
	}
	//Rect rc(0, 0, imgView.cols, imgView.rows);

	//if (rectDispatch.pt0.x < rectClient.left) {
	//	rc.x = rectClient.left-rectDispatch.pt0.x;
	//	rectDispatch.pt0.x = rectClient.left;
	//	rc.width -= rc.x;
	//}
	//if (rectDispatch.pt0.y < rectClient.top) {
	//	rc.y = rectClient.top-rectDispatch.pt0.y;
	//	rectDispatch.pt0.y = rectClient.top;
	//	rc.height -= rc.y;
	//}
	//if (rc.width > rectTarget.right-rc.x)
	//	rc.width = rectTarget.right-rc.x;
	//if (rc.height > rectTarget.bottom - rc.y)
	//	rc.height = rectTarget.bottom - rc.y;
	//if (CheckROI(rc, imgView.size())) {
	//	imgView = imgView(rc);
	//} else {
	//	imgView.release();
	//	return false;
	//}
	// round down
	ptOffset.x = (int)rectDispatch.pt0.x;
	ptOffset.y = (int)rectDispatch.pt0.y;

	//if (!CheckAndGetZoom(dZoom)) {
	//	if (dZoom == 1.0)
	//		imgView = imgOrg;
	//	else
	//		ResizeImage(imgOrg, imgView, dZoom, dZoom >= 1.0 ? m_imgOrg.GetScaleUpMethod() : m_imgOrg.GetScaleDownMethod());
	//} else if (dZoom > 0.0) {
	//	CCoordTrans ctI2S(m_ctI2S);
	//	ctI2S.SetScale(dZoom);
	//	CRect2d rectDOrg;
	//	rectDOrg.pt0 = ctI2S.TransI(CPoint2d(rectClient.TopLeft()));
	//	rectDOrg.pt1 = ctI2S.TransI(CPoint2d(rectClient.BottomRight()));

	//	cv::Rect rectOrg(_round(rectDOrg.left-0.5), _round(rectDOrg.top-0.5), _round(rectDOrg.Width()+1.0), _round(rectDOrg.Height()+1.0));

	//	cv::Rect r;
	//	AdjustROI(rectOrg, r, imgOrg.size());

	//	Mat img = imgOrg(r);
	//	if (dZoom == 1.0)
	//		img.copyTo(imgView);
	//	else {
	//		ResizeImage(img, imgView, dZoom, dZoom >= 1.0 ? m_imgOrg.GetScaleUpMethod() : m_imgOrg.GetScaleDownMethod());
	//		// 확대할 경우, 이미지 위치 보정
	//		if ( (imgView.cols > rectClient.Width()) || (imgView.rows > rectClient.Height()) ) {
	//			CPoint2d pt0(rectClient.TopLeft());// = m_ctI2S.Trans(rectdOrg.TopLeft());
	//			CPoint2d pt1 = m_ctI2S.Trans(CPoint2d(rectOrg.x, rectOrg.y));
	//			int x = _max(0, _round(pt0.x - pt1.x));
	//			int y = _max(0, _round(pt0.y - pt1.y));
	//			int cx = _min(imgView.cols - x, rectClient.Width());
	//			int cy = _min(imgView.rows - y, rectClient.Height());
	//			imgView = imgView(Rect(x, y, cx, cy));
	//		}
	//	}
	//}

	if (!imgView.empty() && (imgView.channels() == 1)) {
		cvtColor(imgView, imgView, CV_GRAY2BGR);
	}

	return TRUE;
}

BOOL CMatView::PrepareDisplayImage(C2dMatArray& imgSet, cv::Mat& imgView, const CRect& rectClient, double dZoom) {
	if (!m_hWnd || imgSet.empty() || rectClient.IsRectEmpty())
		return FALSE;

	//CWaitCursor wc;

	DWORD dwTick = GetTickCount();

	CheckAndGetZoom(dZoom);
	if (dZoom > 0.0) {
		CCoordTrans ctI2S(m_ctI2S);
		ctI2S.SetScale(dZoom);
		CRect2d rectDOrg;
		rectDOrg.pt0 = ctI2S.TransI(CPoint2d(rectClient.TopLeft()));
		rectDOrg.pt1 = ctI2S.TransI(CPoint2d(rectClient.BottomRight()));

		cv::Rect rectOrg(_round(rectDOrg.left-0.5), _round(rectDOrg.top-0.5), _round(rectDOrg.Width()+1.0), _round(rectDOrg.Height()+1.0));

		cv::Rect r;
		AdjustROI(rectOrg, r, imgSet.size());

		imgView = imgSet.GetResizedImage(r, dZoom);

		if (dZoom >= 1.0) {
			// 확대할 경우, 이미지 위치 보정
			if ( (imgView.cols > rectClient.Width()) || (imgView.rows > rectClient.Height()) ) {
				CPoint2d pt0(rectClient.TopLeft());// = m_ctI2S.Trans(rectdOrg.TopLeft());
				CPoint2d pt1 = m_ctI2S.Trans(CPoint2d(rectOrg.x, rectOrg.y));
				int x = _max(0, _round(pt0.x - pt1.x));
				int y = _max(0, _round(pt0.y - pt1.y));
				int cx = _min(imgView.cols - x, rectClient.Width());
				int cy = _min(imgView.rows - y, rectClient.Height());
				imgView = imgView(Rect(x, y, cx, cy));
			}
		}
	}

	if (!imgView.empty() && (imgView.channels() == 1)) {
		cvtColor(imgView, imgView, CV_GRAY2BGR);
	}

	//DWORD dwTick1 = GetTickCount();
	//TRACE("UpdateDisplay : %d msec\n", dwTick1-dwTick);

	return TRUE;
}

BOOL CMatView::UpdateDisplayImage(double dZoom, bool bDelayedUpdate) {
	if (!m_hWnd)
		return FALSE;
	if (m_imgOrg.empty())
		return FALSE;

	CRect rectClient;
	GetClientRect(rectClient);
	if (rectClient.IsRectEmpty())
		return FALSE;

	if (!m_bDraggingProcessed && bDelayedUpdate) {
		SetTimer(T_UPDATE_DISPLAY_IMAGE, 100, nullptr);
		return TRUE;
	}	
	//CWaitCursor wc;

	PrepareDisplayImage(m_imgOrg, m_imgView, rectClient, dZoom);

	for (int i = 0; i < m_imgsAttribute.N(); i++) {
		m_imgsAttribute[i].imgView.release();
		PrepareDisplayImage(m_imgsAttribute[i].img, m_imgsAttribute[i].imgView, m_imgsAttribute[i].ptPatch, m_imgsAttribute[i].ptTargetOffset, rectClient, dZoom);
	}

	m_bDraggingProcessed = false;
	InvalidateRect(rectClient, FALSE);

	{
		MSG msg;
		while (PeekMessage(&msg, m_hWnd, WM_UPDATE_DISPLAY_IMAGE, WM_UPDATE_DISPLAY_IMAGE, PM_REMOVE))
			;
		KillTimer(T_UPDATE_DISPLAY_IMAGE);
	}

	return TRUE;
}

BOOL CMatView::UpdateScrollBars(double dZoom) {
	if (!m_hWnd)
		return FALSE;
	if (m_imgOrg.empty())
		return FALSE;

//	CRefCounter rcUpdateScroll(m_rcUpdateScroll);

	{
		SCROLLINFO siH, siV;
		ZeroVar(siH);
		siH.cbSize = sizeof(siH);
		ZeroVar(siV);
		siV.cbSize = sizeof(siV);
		//GetScrollInfo(SB_HORZ, &siH);
		//GetScrollInfo(SB_VERT, &siV);

		CSize2i size;
		CRect2i rectClient;
		GetClientRect((CRect&)rectClient);

		if (!CheckAndGetZoom(dZoom)) {
			size = (CSize2i&)rectClient;
		} else {
			size.cx = (int)(m_imgOrg.GetWidth() * dZoom);
			size.cy = (int)(m_imgOrg.GetHeight() * dZoom);
		}

		// Backup Center Position
		CPoint2i ptScroll;
		CPoint2d ptShift;
		m_ctI2S.GetShift(ptShift);
		m_ctI2S.SetScale(dZoom);
		CRect rect(0, 0, m_imgOrg.GetWidth(), m_imgOrg.GetHeight());
		rect.DeflateRect(_round_int(rectClient.Width() / m_ctI2S.GetScale() / 2.), _round_int(rectClient.Height() / m_ctI2S.GetScale() / 2.));

		// Point Validation
		if (rect.left <= rect.right) {
			if (ptShift.x < rect.left)
				ptShift.x = rect.left;
			if (ptShift.x > rect.right)
				ptShift.x = rect.right;
		} else {
			ptShift.x = rect.CenterPoint().x;
		}
		if (rect.top <= rect.bottom) {
			if (ptShift.y < rect.top)
				ptShift.y = rect.top;
			if (ptShift.y > rect.bottom)
				ptShift.y = rect.bottom;
		} else {
			ptShift.y = rect.CenterPoint().y;
		}
		m_ctI2S.SetShift(ptShift);
		m_ctI2S.SetOffset(rectClient.CenterPoint());

		if (GetStyle() & WS_HSCROLL) {
			siH.cbSize = sizeof(siH);
			siH.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
			siH.nMin = 0;
			if (size.cx <= rectClient.Width()) {
				siH.nPage = siH.nMin;
				siH.nMax = siH.nMin;	// Disables
				siH.nPos = siH.nMin;
			} else {
				siH.nPage = rectClient.Width();
				siH.nMax = size.cx;
				siH.nPos = _round_int(ptShift.x * m_ctI2S.GetScale() - siH.nPage/2);
			}
			SetScrollInfo(SB_HORZ, &siH);
		}

		if (GetStyle() & WS_VSCROLL) {
			siV.cbSize = sizeof(siV);
			siV.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
			siV.nMin = 0;
			if (size.cy <= rectClient.Height()) {
				siV.nPage = siV.nMin;
				siV.nMax = siV.nMin;	// Disables
				siV.nPos = siV.nMin;
			} else {
				siV.nPage = rectClient.Height();
				siV.nMax = size.cy;
				siV.nPos = _round_int(ptShift.y * m_ctI2S.GetScale() - siV.nPage/2);
			}
			SetScrollInfo(SB_VERT, &siV);
		}
	}

	return true;
}

//BOOL CMatView::UpdateScrollBarPosition() {
//	return TRUE;
//}

BOOL CMatView::InitView(double dZoom) {
	m_ctI2S.Set();
	if (dZoom >= 0)
		m_ctI2S.SetShift(m_imgOrg.GetWidth()/2, m_imgOrg.GetHeight()/2);

	if (dZoom == 0.0) {
		m_comboZoom.SetCurSel(0);
	} else if (dZoom > 0.0) {
		CString str;
		str.Format(_T("%g"), dZoom);
		if (m_comboZoom.SelectString(-1, str) < 0)
			m_comboZoom.SetWindowText(str);
	}

	UpdateScrollBars(dZoom);
	UpdateDisplayImage(dZoom);

	return true;
}

BOOL CMatView::SetImage(const Mat& img, double dZoom, BOOL bCopyImage) {
	m_imgOrg.Destroy();
	m_imgView.release();

	//DeleteAttributeLayer();
	//DeleteCrossMark();
	//DeleteRectRegion();
	m_mouse.bSelectionMode = false;
	m_mouse.bDragMode = false;

	if (img.empty()) {
		InitView(dZoom);
		Invalidate(FALSE);
	} else {
		if (bCopyImage) {
			if (!m_imgOrg.Create(CSize2i(1, 1)))
				return false;
			m_imgOrg.SetPartialImage({0, 0}, img, bCopyImage);
		} else {
			if (!m_imgOrg.Create((Mat&)img, img.size(), CSize2i(1, 1)) ) //, { { 1, 2}, {1, 4}, }, 2);
				return false;
		}

		m_ctI2S.SetShift(m_imgOrg.GetWidth()/2, m_imgOrg.GetHeight()/2);

		InitView(dZoom);
		UpdateDisplayImage();
	}

	return true;
}

BOOL CMatView::SetImageCenter(const CPoint2d& pt, double dZoom, bool bDelayedUpdate) {
	if (!m_hWnd || m_imgOrg.empty())
		return false;

	CheckAndGetZoom(dZoom);

	CRect rectClient;
	GetClientRect(rectClient);
	CPoint2d ptShift(pt);
	CCoordTrans ct(m_ctI2S);
	ct.SetShift(ptShift);
	ct.SetOffset(rectClient.CenterPoint());
	ct.SetScale(dZoom);
	CPoint2d ptCenter(rectClient.CenterPoint());
	m_ctI2S.SetShift(ct.TransI(ptCenter));

	SetDlgItemDouble(this, eIDZoom, dZoom, _T("%g"));

	UpdateScrollBars();
	UpdateDisplayImage(-1, bDelayedUpdate);

	return true;
}

void CMatView::SetZoom(double dZoom) {
	if (m_imgOrg.empty())
		return;

	CheckAndGetZoom(dZoom);

	SetDlgItemDouble(this, eIDZoom, dZoom, _T("%g"));
	UpdateScrollBars();
	UpdateDisplayImage();
}

void CMatView::ShowTool(BOOL bShow) {
	m_setting.bShowTool = bShow;

	if (!m_hWnd)
		return;

	UpdateScrollBars();
	UpdateDisplayImage();

	int nCmdShow = m_setting.bShowTool ? SW_SHOW : SW_HIDE;
	for (int id = eIDStart; id < eIDEnd; id++) {
		CWnd* pWnd = GetDlgItem(id);
		if (pWnd) {
			switch (id) {
			case eIDPatternMatching :
				pWnd->ShowWindow(m_setting.bShowTool && m_setting.bPatternMatching ? SW_SHOW : SW_HIDE);
				break;
			case eIDInfo :
				if (m_setting.bShowTool) {
					CRect rect;
					if (m_setting.bStatusOnBottom) {
						{	// 흔적이 자꾸 남아서..... 이거 여기서 하면 안되는데...
							CRect rect;
							pWnd->GetWindowRect(rect);
							ScreenToClient(rect);
							CBrush brush((COLORREF)GetSysColor(CTLCOLOR_DLG));
							CClientDC dc(this);
							dc.SelectObject(brush);
							dc.SelectStockObject(NULL_PEN);
							dc.Rectangle(rect);
						}

						CRect rectClient;
						GetClientRect(rectClient);
						rect.left = 0;
						rect.top = rectClient.bottom;
						rect.bottom = rect.top + m_rectTool.Height();
						rect.right = rectClient.right;
					} else {
						CRect rectPrev;
						pWnd->GetWindowRect(rect);
						ScreenToClient(rect);
						CWnd* pWndPrev = GetNextDlgTabItem(pWnd, true);
						if (pWndPrev) {
							pWndPrev->GetWindowRect(rectPrev);
							ScreenToClient(rectPrev);
							rect.MoveToXY(rectPrev.right, rectPrev.top);
							rect.bottom = rectPrev.bottom;
							rect.right = m_rectTool.right;
						}
					}
					pWnd->MoveWindow(rect);
				}
				pWnd->ShowWindow(nCmdShow);
				break;
			default :
				pWnd->ShowWindow(nCmdShow);
				break;
			}
		}
	}

	Invalidate(true);
}

void CMatView::UpdateTool() {
	if (!m_hWnd)
		return;

	CRect rect, rectPrev;
	for (int idc = eIDStart; idc < eIDEnd; idc++) {
		CWnd* pCtrl = GetDlgItem(idc);
		if (!pCtrl)
			continue;
		pCtrl->GetWindowRect(rect);
		ScreenToClient(rect);
		switch (idc) {
		case eIDPatternMatching :
			{
				BOOL bVisible = pCtrl->IsWindowVisible();
				if (m_setting.bPatternMatching) {
					pCtrl->ShowWindow(SW_SHOW);
				} else {
					if (bVisible)
						pCtrl->ShowWindow(SW_HIDE);
					continue;
				}
			}
			break;
		}

		if (idc == eIDStart) {
		} else {
			CRect rectO(rect);
			if (rect.left != rectPrev.right) {
				rect.MoveToX(rectPrev.right);
			}
			if (idc == eIDEnd-1) {
				CRect rectClient;
				GetClientRect(rectClient);
				rect.right = rectClient.right;
			}
			if ( (rect != rectO) && !rect.IsRectEmpty() )
				pCtrl->MoveWindow(rect);
		}

		rectPrev = rect;
	}
}

BOOL CMatView::GetCurrentImageRect(CRect2d& rect) const {
	if (m_imgOrg.empty())
		return FALSE;

	CRect rectClient;
	GetClientRect(rectClient);
	rect.pt0 = m_ctI2S.TransI(CPoint2d(rectClient.TopLeft()));
	rect.pt1 = m_ctI2S.TransI(CPoint2d(rectClient.BottomRight()));

	if (rect.left < 0) rect.left = 0;
	if (rect.top < 0) rect.top = 0;
	if (rect.right >= m_imgOrg.GetWidth()) rect.right = m_imgOrg.GetWidth();
	if (rect.bottom >= m_imgOrg.GetHeight()) rect.bottom = m_imgOrg.GetHeight();

	return TRUE;
}

BOOL CMatView::SetCurrentImageRect(const CRect2d& rect) {
	if (m_imgOrg.empty())
		return FALSE;

	CRect rectClient;
	GetClientRect(rectClient);
	if (rectClient.IsRectEmpty())
		return false;

	double dZoomX = rectClient.Width() / rect.Width();
	double dZoomY = rectClient.Height() / rect.Height();

	double dZoom = _min(dZoomX, dZoomY);
	if (dZoom > 10)
		dZoom = 10.;

	return SetImageCenter(rect.CenterPoint(), dZoom);
}

//-----------------------------------------------------------------------------

BOOL CMatView::StartSelectionMode(const CPoint2d& ptImage) {
	SetCapture();

	m_mouse.bSelectionMode = true;
	m_mouse.ptSelect1 = m_mouse.ptSelect0 = ptImage;

	return true;
}

BOOL CMatView::EndSelectionMode() {
	if (!m_mouse.bSelectionMode)
		return false;

	if (GetCapture() == this)
		ReleaseCapture();

	m_mouse.bSelectionMode = false;
	CRect2d rect(m_mouse.ptSelect0, m_mouse.ptSelect1);
	rect.NormalizeRect();
	SetSelectedImageRect(rect, true);
	NotifyEvent(NC_SEL_REGION, rect, true);

	return TRUE;
}

BOOL CMatView::GetSelectedImageRect(CRect2d& rect) const {
	if (m_imgOrg.empty())
		return false;

	rect = m_display.rectCurrent.rect;

	return CheckROI(rect, m_imgOrg.size());
}

BOOL CMatView::SetSelectedImageRect(const CRect2d& rect, bool bSelect) {
	auto& r = m_display.rectCurrent;
	r.bShow = bSelect;
	if (bSelect) {
		r.rect = rect;
		r.cr = RGB(128, 255, 128);
		r.iThick = 1;
		r.nPenStyle = PS_DASH;
		r.strLabel.Empty();
		r.strName.Empty();
	} else {
		m_display.rectCurrent.bShow = false;
		m_display.rectCurrent.rect.SetRectEmpty();
		//m_rectSelected.SetRectEmpty();
	}

	Invalidate(FALSE);

	return false;
}

//-----------------------------------------------------------------------------

BOOL CMatView::AddAttributeLayer(LPCTSTR pszName, const cv::Mat& img, COLORREF crTransparent) {
	int iIndex = m_imgsAttribute.FindByValue(pszName);
	if (iIndex < 0) {
		iIndex = m_imgsAttribute.Push(new T_ATTRIBUTE);
	}
	m_imgsAttribute[iIndex].ptPatch = {0, 0};
	m_imgsAttribute[iIndex].img = img;
	m_imgsAttribute[iIndex].bm = BM_TRANSPARENT;
	m_imgsAttribute[iIndex].crTransparent = crTransparent;
	UpdateDisplayImage();
	return TRUE;
}
BOOL CMatView::AddAttributeLayer(LPCTSTR pszName, const cv::Mat& img, BLENDFUNCTION blend) {
	int iIndex = m_imgsAttribute.FindByValue(pszName);
	if (iIndex < 0) {
		iIndex = m_imgsAttribute.Push(new T_ATTRIBUTE);
	}
	m_imgsAttribute[iIndex].ptPatch = {0, 0};
	m_imgsAttribute[iIndex].img = img;
	m_imgsAttribute[iIndex].bm = BM_ALPHABLEND;
	m_imgsAttribute[iIndex].blend = blend;
	UpdateDisplayImage();
	return TRUE;
}
BOOL CMatView::AddAttributeLayer(LPCTSTR pszName, const cv::Point& ptLT, const cv::Mat& img, COLORREF crTransparent) {
	int iIndex = m_imgsAttribute.FindByValue(pszName);
	if (iIndex < 0) {
		iIndex = m_imgsAttribute.Push(new T_ATTRIBUTE);
	}
	m_imgsAttribute[iIndex].ptPatch = ptLT;
	m_imgsAttribute[iIndex].img = img;
	m_imgsAttribute[iIndex].bm = BM_TRANSPARENT;
	m_imgsAttribute[iIndex].crTransparent = crTransparent;
	UpdateDisplayImage();
	return TRUE;
}
BOOL CMatView::AddAttributeLayer(LPCTSTR pszName, const cv::Point& ptLT, const cv::Mat& img, BLENDFUNCTION blend) {
	int iIndex = m_imgsAttribute.FindByValue(pszName);
	if (iIndex < 0) {
		iIndex = m_imgsAttribute.Push(new T_ATTRIBUTE);
	}
	m_imgsAttribute[iIndex].ptPatch = ptLT;
	m_imgsAttribute[iIndex].img = img;
	m_imgsAttribute[iIndex].bm = BM_ALPHABLEND;
	m_imgsAttribute[iIndex].blend = blend;
	UpdateDisplayImage();
	return TRUE;
}
BOOL CMatView::SetAttributeLayer(LPCTSTR pszName, const cv::Mat& img) {
	int iIndex = m_imgsAttribute.FindByValue(pszName);
	if (iIndex < 0) {
		return FALSE;
	}
	m_imgsAttribute[iIndex].img = img;
	UpdateDisplayImage();
	return TRUE;
}
BOOL CMatView::DeleteAttributeLayer(LPCTSTR pszName) {
	if (pszName) {
		while (m_imgsAttribute.DeleteByValue(pszName)) ;
	} else {
		m_imgsAttribute.DeleteAll();
	}
	UpdateDisplayImage();
	return TRUE;
}


BOOL CMatView::ShowCrossMark(LPCTSTR pszName, BOOL bShow) {
	if (pszName) {
		int iIndex = -1;
		do {
			iIndex = m_display.crosses.FindByValue(pszName, iIndex);
			if (iIndex >= 0)
				m_display.crosses[iIndex].bShow = bShow;
			else
				break;
		} while (TRUE);
	} else {
		for (int i = 0; i < m_display.crosses.N(); i++) {
			m_display.crosses[i].bShow = bShow;
		}
	}

	if (m_hWnd)
		Invalidate(FALSE);
	return TRUE;
}
int CMatView::AddCrossMark(LPCTSTR pszName, const CPoint2d& pt, LPCTSTR pszLabel, int iSizeX, int iSizeY, int nPenStyle, int iThick, COLORREF cr) {
	int iIndex = -1;
	if (pszName)
		iIndex = m_display.crosses.FindByValue(pszName);
	if (iIndex < 0)
		iIndex = m_display.crosses.Push(new T_CROSS_MARK);
	if (iIndex < 0)
		return iIndex;
	T_CROSS_MARK& cross = m_display.crosses[iIndex];
	cross.strName = pszName;
	cross.strLabel = pszLabel;
	cross.bShow = TRUE;
	cross.pt = pt;
	cross.size.cx = iSizeX >= 0 ? iSizeX : 0;
	cross.size.cy = iSizeY >= 0 ? iSizeY : 0;
	cross.iThick = iThick;
	cross.cr = cr;

	if (m_hWnd)
		Invalidate(FALSE);
	return TRUE;
}
BOOL CMatView::DeleteCrossMark(LPCTSTR pszName) {
	if (pszName) {
		while(m_display.crosses.DeleteByValue(pszName)) ;
	} else {
		m_display.crosses.DeleteAll();
	}

	if (m_hWnd)
		Invalidate(FALSE);
	return TRUE;
}

BOOL CMatView::ShowRectRegion(LPCTSTR pszName, BOOL bShow) {
	if (pszName) {
		int iIndex = -1;
		do {
			iIndex = m_display.rects.FindByValue(pszName, iIndex);
			if (iIndex >= 0)
				m_display.rects[iIndex].bShow = bShow;
			else
				break;
		} while (TRUE);
	} else {
		for (int i = 0; i < m_display.rects.N(); i++) {
			m_display.rects[i].bShow = bShow;
		}
	}

	if (m_hWnd)
		Invalidate(FALSE);
	return TRUE;
}
int CMatView::AddRectRegion(LPCTSTR pszName, const CRect2d& rect, LPCTSTR pszLabel, int nPenStyle, int iThick, COLORREF crPen) {
	int iIndex = -1;
	if (pszName)
		iIndex = m_display.rects.FindByValue(pszName);
	if (iIndex < 0)
		iIndex = m_display.rects.Push(new T_RECT_REGION);
	if (iIndex < 0)
		return iIndex;
	T_RECT_REGION& region = m_display.rects[iIndex];
	region.strName = pszName;
	region.strLabel = pszLabel;
	region.bShow = TRUE;
	region.rect = rect;
	region.nPenStyle = nPenStyle;
	region.iThick = iThick;
	region.cr = crPen;

	if (m_hWnd)
		Invalidate(FALSE);
	return iIndex;
}
BOOL CMatView::DeleteRectRegion(LPCTSTR pszName) {
	if (pszName) {
		while (m_display.rects.DeleteByValue(pszName)) ;
	} else {
		m_display.rects.DeleteAll();
	}

	if (m_hWnd)
		Invalidate(FALSE);
	return TRUE;
}

LRESULT CMatView::OnUpdateDisplayImage(WPARAM wParam, LPARAM lParam) {

	return UpdateDisplayImage( ((intptr_t)wParam) / dZoomRateWParam, lParam);
}

//-----------------------------------------------------------------------------


//=============================================================================
//
