
// test_win.h : main header file for the test_win application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CtestwinApp:
// See test_win.cpp for the implementation of this class
//

class CtestwinApp : public CWinAppEx
{
public:
	CtestwinApp() noexcept;


// Overrides
public:
	virtual BOOL InitInstance() override;
	virtual int ExitInstance() override;

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState() override;
	virtual void LoadCustomState() override;
	virtual void SaveCustomState() override;

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CtestwinApp theApp;
