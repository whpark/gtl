//module;
//
//#include "gtl/gtl.h"
//#include "gtl.ui.h"
//
//// add headers that you want to pre-compile here
//#include "targetver.h"
//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
//// Windows Header Files
//#include <windows.h>
//// C RunTime Header Files
//#include <tchar.h>
//
//#include <Windowsx.h>
//#include <assert.h>
//#include <unknwn.h>
//
//#include <wrl.h>
//#include <wil/com.h>
//
//#include "webview2.h"
//
//#define BOOST_JSON_STANDALONE
//#include "boost/json.hpp"
//
//
//export module gtl.ui:webview2ctrl;
//
//using namespace std::literals;
//using namespace gtl::literals;


#pragma once

#include "gtl/gtl.h"
#include "ui_predefine.h"
#include "ui_prop.h"
#include "ui_widget.h"

// add headers that you want to pre-compile here
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <tchar.h>

#include <Windowsx.h>
#include <assert.h>
#include <unknwn.h>

#include <wrl.h>
#include <wil/com.h>

#include "webview2.h"


namespace gtl::ui::renderer {

	//=============================================================================================================================
	//! @brief 
	class CWebView2Ctrl {
	public:
		HWND m_hWndOwner{};
		wil::com_ptr<ICoreWebView2> m_webView;
		wil::com_ptr<ICoreWebView2Controller> m_controller;

		xWidget m_widget;

	protected:
		EventRegistrationToken m_webMessageReceivedToken = {};

	public:
		CWebView2Ctrl(HWND hWndOwner) : m_hWndOwner{ hWndOwner } {
		}

	public:
		bool SetContext() {
			string_t strContent;

			static std::filesystem::path path(L"gtl.ui.html");
			if (std::streamsize len = std::filesystem::file_size(path); len > 0) {
				std::ifstream f(path, std::ios_base::binary);
				strContent.assign(len, 0);
				if (!f.read((char*)strContent.data(), (std::streamsize)len)) {
					strContent.clear();
				}
			} else
				return false;

			static string_t const strInsertionText = GText("<!-- gtl.ui - here ❤ -->");
			if (auto index = strContent.find(strInsertionText, 0); index != strContent.npos) {
				string_t strTail{strContent.data() + index + strInsertionText.size(), strContent.size() - index - strInsertionText.size()};

				auto strBody = m_widget.MakeHTML();

				strContent.resize(index);
				strContent.reserve(index+strBody.size()+strTail.size());
				strContent += strBody;
				strContent += strTail;
			}

			if (strContent.empty())
				return false;

			auto str = gtl::ToStringW(strContent);
			m_webView->NavigateToString(str.c_str());
			m_webView->add_WebMessageReceived(
				Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
					[this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) {
						wil::unique_cotaskmem_string messageRaw;
						if (args->TryGetWebMessageAsString(&messageRaw) == S_OK) {
							std::wstring message = messageRaw.get();

							OnHTMLMessage(sender, message);

							return S_OK;
						}
						return S_FALSE;
					}).Get(), &m_webMessageReceivedToken);

			return true;
		}
		auto OnCreateContext(HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
			if (controller != nullptr) {
				m_controller = controller;
				m_controller->get_CoreWebView2(&m_webView);
			}

			// Add a few settings for the webview
			// The demo step is redundant since the values are the default settings
			ICoreWebView2Settings* Settings;
			m_webView->get_Settings(&Settings);
			Settings->put_IsScriptEnabled(true);
			Settings->put_AreDefaultScriptDialogsEnabled(true);
			Settings->put_IsWebMessageEnabled(true);
			Settings->put_AreDefaultContextMenusEnabled(false);
			Settings->put_IsZoomControlEnabled(false);
		#ifdef _DEBUG
			Settings->put_AreDevToolsEnabled(true);
			m_webView->OpenDevToolsWindow();
		#endif

			// Resize WebView to fit the bounds of the parent window
			RECT bounds;
			GetClientRect(m_hWndOwner, &bounds);
			m_controller->put_Bounds(bounds);

			// Schedule an async task to navigate to Bing
			//m_webView->Navigate(L"./wui.html");
			SetContext();

			// Step 4 - Navigation events

			// Step 5 - Scripting

			// Step 6 - Communication between host and web content

			return S_OK;
		}
		void InitializeWebView() {

			auto r = CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
				Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
					[&](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {

						// Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
						env->CreateCoreWebView2Controller(m_hWndOwner, Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
							[&](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT { return this->OnCreateContext(result, controller); }
							).Get());
						return S_OK;
					}).Get());
		}

		void OnHTMLMessage(ICoreWebView2* sender, std::wstring message) {
			static auto const strButtonClick = L"Button_Click:"s;
			static auto const strEditChange = L"Edit_Change:"s;
			if (message.starts_with(strButtonClick)) {
				string_t id = ToString<char_type, wchar_t>(message.substr(strButtonClick.size()));
				if (auto rWidget = m_widget.FindComponent(id); rWidget and rWidget->fn_OnClick) {
					rWidget->fn_OnClick(rWidget, {}, {});
				}
				//std::wstring reply = L"message";
				//sender->PostWebMessageAsString(reply.c_str());
			}
			else if (message.starts_with(strEditChange)) {
				string_t m = ToString<char_type, wchar_t>(message.substr(strEditChange.size()));
				if (auto i = m.find(':'); i != m.npos) {
					string_t id = m.substr(0, i);
					string_t str = m.substr(i+1);
					if (auto rWidget = m_widget.FindComponent(id); rWidget and rWidget->fn_OnTextChanged) {
						if (rWidget->fn_OnTextChanged(rWidget, {}, str)) {
						} else {
						}
					} else {
						AfxMessageBox(gtl::xStringW(m));
					}
				}
			}
			else if (message.compare(L"GetWindowBounds") == 0) {
				RECT bounds{};
				m_controller->get_Bounds(&bounds);
				std::wstring reply =
					L"{\"WindowBounds\":\"Left:" + std::to_wstring(bounds.left)
					+ L"\\nTop:" + std::to_wstring(bounds.top)
					+ L"\\nRight:" + std::to_wstring(bounds.right)
					+ L"\\nBottom:" + std::to_wstring(bounds.bottom)
					+ L"\"}";
				sender->PostWebMessageAsJson(reply.c_str());
			}
		}

		gtl::xStringU8 GetText_TEST(string_t id) {
			gtl::xStringU8 strResult;
			CEvent e(true, true);
			e.ResetEvent();
			m_webView->ExecuteScript(fmt::format(LR"xx(document.getElementById('{}').value)xx", gtl::xStringW(id)).c_str(),
									 Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
										 [&strResult, &e](HRESULT error, PCWSTR result) -> HRESULT
										 {
											 if (error != S_OK) {
												 //ShowFailure(error, L"ExecuteScript failed");
											 } else {
												 strResult = std::wstring_view(result);
											 }
											 e.SetEvent();
											 return S_OK;
										 }).Get()
									 );
			while (::WaitForSingleObjectEx(e.m_hObject, 100, true) != WAIT_OBJECT_0) {
				theApp.PumpMessage();
				Sleep(0);
			}
			return strResult;
		}

		//void CloseWebView();
		//void CloseWebViewCtrl();
		//std::wstring GetLocalUri(std::wstring relativePath);
		//std::wstring GetLocalPath(std::wstring relativePath, bool keep_exe_path);

		//wil::com_ptr<ICoreWebView2Environment> m_webViewEnvironment;
		//wil::com_ptr<ICoreWebView2CompositionController> m_compositionController;
	};


}
