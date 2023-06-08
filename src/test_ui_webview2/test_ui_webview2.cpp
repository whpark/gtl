// test_ui_webview2.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "test_ui_webview2.h"
#include "gtl_ui_renderer_webview2.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
HWND                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

using namespace Microsoft::WRL;

std::shared_ptr<gtl::ui::renderer::CWebView2Ctrl> s_webview2;

void OnButtonEvent(gtl::ui::string_view_t id) {
	auto strMessage = fmt::format(_T("Button - {}"), gtl::ToStringW(id));
	::MessageBox(nullptr, strMessage.c_str(), nullptr, MB_OK);
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    using namespace Microsoft::WRL;

    gtl::SetCurrentPath_BinFolder();
	
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TESTUIWEBVIEW2, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    HWND hWnd = InitInstance (hInstance, nCmdShow);
    if (!hWnd)
        return 0;

    s_webview2 = std::make_shared<gtl::ui::renderer::CWebView2Ctrl>(hWnd);

	using namespace gtl::ui;
	using namespace gtl::ui::unit;

	//xWidgetData::InitUniqueID();
	position_t widthCol1 {120_px};

	xWidget& w = s_webview2->m_widget;
	w.m_properties.emplace_back(GText("user-select"), GText("none"));
	w.m_components.push_back(
		xWidget::GroupBoxVertical(GText("GroupBox <Vertical>"), {.m_properties{prop::Width(300_px)}},
			{
				xWidget::PanelHorizontal({},
				{
					xWidget::Label(GText("Title"s), widthCol1, {}, {}),
					xWidget::Button(GText("Button1"s), {}, {}, {.fn_OnClick = [&](rWidgetData rself, string_view_t evt, string_view_t text) ->bool{
						//if (auto r = w.FindComponent(GText("EDIT"))) { MessageBox(gtl::xStringW(m_webview->GetText_TEST(r->m_id))); }
						OnButtonEvent(ToString(rself->m_id)); return true; }}),
				}),
				xWidget::PanelHorizontal({}, {
					xWidget::Label(GText("Label2"s), widthCol1),
					xWidget::Button(GText("Button2"s), {}, {}, {.fn_OnClick = [](rWidgetData rself, string_view_t evt, string_view_t text) { OnButtonEvent(ToString(rself->m_id)); return true; }}),
					xWidget::Input(GText("text <value>"s), {}, {}, {}),
				}),
				xWidget::PanelHorizontal({}, {
					xWidget::Label(GText("Label3"s), widthCol1),
					xWidget::Select(GText("Select..."), {GText("string option 1"), GText("string option2"), GText("mmm3"), GText("mmm4"), GText("mmm5"), GText("mmmmmmm6"), GText("mmm")}, 4),
				}),
				xWidget::PanelHorizontal({}, {
					xWidget::Label(GText("Label4"s), widthCol1),
					xWidget::Select(GText("Select..."), {GText("text 1"), GText("string option2"), GText("mmm3"), GText("mmm4"), GText("mmm5"), GText("mmmmmmm6"), GText("fasdfasdf")}),
				}),
			})
	);
	//w.m_components.push_back(
	//	xWidget::List ()
	//);

	xWidgetData t{.m_tag=u8"fsdfsd"s};

	xWidget vertical_panel;
	vertical_panel.m_properties.push_back(prop::Width(200_px));
	vertical_panel.m_properties.push_back(prop::Height(30_px));
	xWidget hpanel;
	auto label = xWidget::Label(GText("Title"s));
	label->m_properties.push_back(prop::Width(300_px));
	label->m_properties.push_back(prop::Height(30_px));
	vertical_panel.AddComponent(label);
    w.AddComponent(std::move(vertical_panel));
	hpanel.AddComponent(xWidget::Label(GText("Title"s)));
	w.AddComponent(std::move(hpanel));

    s_webview2->InitializeWebView();

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TESTUIWEBVIEW2));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TESTUIWEBVIEW2));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TESTUIWEBVIEW2);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 720, 480, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd) {
      return nullptr;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return hWnd;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
		// Parse the menu selections:
		switch (int wmId = LOWORD(wParam); wmId)
		{
		case IDM_ABOUT:
            if (s_webview2 and s_webview2->m_webView) {
                auto& widget = s_webview2->m_widget;
                std::erase_if(s_webview2->m_widget.m_components, [](auto const& i) { return i->m_id == 5; });
                s_webview2->SetContext();
            }
			//DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
        break;
    case WM_SIZE :
		if (auto nType = wParam; (nType != SIZE_MINIMIZED) and s_webview2 and s_webview2->m_controller) {
			auto cx = LOWORD(lParam);
			auto cy = HIWORD(lParam);
			RECT rect{0, 0, cx, cy};
			s_webview2->m_controller->put_Bounds(rect);
		}
        break;
    case WM_MOVE :
		if (s_webview2 and s_webview2->m_controller) {
			s_webview2->m_controller->NotifyParentWindowPositionChanged();  // bug-bypassing
		}
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
